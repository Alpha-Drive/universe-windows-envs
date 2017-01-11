#include "GTAVRewardCalculator.h"
#include <cassert>
#include "../../GTAVScriptHookProxy/vendor/ScriptHookV/include/natives.h"
#include <thread>
#include <json/json.h>


GTAVRewardCalculator::GTAVRewardCalculator(std::string envId, boost::log::sources::severity_logger_mt<ls::severity_level> lg): is_done_(false)
{
	if (envId == "gtav.SaneDriving-v0")
	{
		reward_mode_ = SANE_DRIVING;
	}
	else if (envId == "gtav.Speed-v0")
	{
		reward_mode_ = SPEED;
	}
	else {
		P_ERR("Unknown envId: " << envId << std::endl);
	}
	
	last_reward_ = 0.0;
	lg = lg_;
}

// Return the rewards that we have aggregated since the last call to getReward()
void GTAVRewardCalculator::get_reward(SharedAgentMemory* shared, double& reward, bool& done, Json::Value& info)
{
	done = false;
	bool already_done = is_done_;
	switch (reward_mode_)
	{
	case SANE_DRIVING:
		calc_sane_driving(shared, reward, done, info);
		break;
	case SPEED:
		reward = shared->speed;
		break;
	default:
		P_ERR("Reward mode unrecognised!" << std::endl);
		break;
	}

	if (done && ! already_done)
	{
		BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Episode done";
	}

	if (done)
	{
		reset(shared);
	}

	is_done_ = done;
}

GTAVRewardCalculator::~GTAVRewardCalculator()
{}

void GTAVRewardCalculator::set_destination(SharedAgentMemory* shared)
{
	if(winding_road_direction_is_down_)
	{
		shared->dest_x = WINDING_BOTTOM_X;
		shared->dest_y = WINDING_BOTTOM_Y;
		shared->dest_z = WINDING_BOTTOM_Z;
	} 
	else
	{
		shared->dest_x = WINDING_TOP_X;
		shared->dest_y = WINDING_TOP_Y;
		shared->dest_z = WINDING_TOP_Z;
	}
}

double GTAVRewardCalculator::get_distance_reward(SharedAgentMemory* shared, bool& done, bool& reached_destination, bool& stalled)
{
	// Returns meters of progress made towards destination
	reached_destination = false;
	stalled = false;

	set_destination(shared);
	current_distance_ = shared->distance_from_destination;
	int distance_check = static_cast<int>(current_distance_);
	if (distance_check != current_distance_)
	{
		// Valid distances are whole numbers.
		return 0;
	}

	if ( ! embarked_ && current_distance_ >= kEndDistance)
	{
		embarked_ = true;
		closest_distance_ = current_distance_;
		last_closest_distance_time_ = std::chrono::steady_clock::now();
	}

	if ( ! embarked_)
	{
		return 0;
	}

	double distance_reward = 0;
	if (current_distance_ > kEndDistance)
	{
		// Pathfind returned actual distance (i.e. not -1 when not found)
		if (closest_distance_ > current_distance_)
		{
			// We've made progress
			distance_reward = closest_distance_ - current_distance_;
			closest_distance_ = current_distance_;
			last_closest_distance_time_ = std::chrono::steady_clock::now();
		}
		else
		{
			// Sometimes pathfind can return a longer route even though we've made progress along the correct path.
			// Wait for some time to make sure we really are not making progress.
			std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
			std::chrono::duration<double> elapsed_seconds = current_time - last_closest_distance_time_;
			if (elapsed_seconds.count() > kMaxSecondsNoProgress)
			{
				if ( ! is_done_)
				{
					BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "No progress in last " << kMaxSecondsNoProgress << " seconds. Stalled, ending episode.";
				}
				stalled = true;
				done = true;
			}
		}
	}
	else if (current_distance_ >= 0 && current_distance_ < kEndDistance)
	{
		// We may have reached our destination
		double eucl_distance = sqrt( 
			pow(shared->dest_x - double(shared->x_coord), 2) + 
			pow(shared->dest_y - double(shared->y_coord), 2) + 
			pow(shared->dest_z - double(shared->z_coord), 2));

		if (eucl_distance < kEndDistance) {
			// Euclidean distance agrees! We're close.
			reached_destination = true;
#ifdef ENDLESS_EPISODE
			// Turn around
			winding_road_direction_is_down_ = !winding_road_direction_is_down_;
			set_destination(shared);
			while(distance < kEndDistance)
			{
				// Wait for new distance to be calculated by ScriptHook path finder
				distance = shared->distance_from_destination;
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
#else
			if ( ! is_done_)
			{
				BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Reached destination, finishing episode: distance from destination " << current_distance_;
			}
			done = true;
#endif
		}
	}

	BOOST_LOG_SEV(lg_, boost::log::trivial::debug) << "distance reward " << distance_reward;

	if (std::isnan(distance_reward))
	{
		BOOST_LOG_SEV(lg_, boost::log::trivial::error) << "Distance reward is NaN" << distance_reward;
		return 0;
	}

	if (distance_reward != 0)
	{
		BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Distance reward " << distance_reward;
	}

	return distance_reward;
}

void GTAVRewardCalculator::get_speed_penalties(SharedAgentMemory* shared, double& slow_penalty, double& speeding_penalty)
{
	double speed_limit = 10; // meters per second // TODO: Pull this dynamically from an IPL file
	const double speed = shared->speed;
	if (speed > speed_limit + 3)
	{
		std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = current_time - last_speeding_time_;
		if ( ! delivered_first_speeding_penatly_ || elapsed_seconds.count() > kSpeedingPenaltyPeriod)
		{
			last_speeding_time_ = std::chrono::steady_clock::now();
			// Squared to outweigh distance advantage of speeding (proportional to kinetic energy - yay!)
			speeding_penalty = pow(speed - speed_limit, 2);
			delivered_first_speeding_penatly_ = true;
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Speeding penalty " << speeding_penalty;
		}
	}
	else if (speed < speed_limit - 3)
	{
		// Too slow - this only makes sense on this particular stretch of road where there are no stop signs or backups.
		std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = current_time - last_slow_penalty_time_;
		if ( ! delivered_first_slow_penalty_ || elapsed_seconds.count() > kSlowPenaltyPeriod)
		{
			last_slow_penalty_time_ = std::chrono::steady_clock::now();
			slow_penalty = 1;
			delivered_first_slow_penalty_ = true;
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Driving too slow penalty " << slow_penalty;
		}
	}
}

void GTAVRewardCalculator::get_offroad_penalty(SharedAgentMemory* shared, double& off_road_penalty)
{
	if (shared->on_road == true)
	{
		last_on_road_time_ = std::chrono::steady_clock::now();
	}
	else
	{
		std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = current_time - last_on_road_time_;
		if (elapsed_seconds.count() > kMaxOffroadSeconds)
		{
			off_road_penalty = 10; // rewards are in scale of meters
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Driving offroad penalty " << off_road_penalty;

			// reset time
			last_on_road_time_ = std::chrono::steady_clock::now();
		}
	}
}

void GTAVRewardCalculator::get_discomfort_penalty(SharedAgentMemory* shared, double& discomfort_penalty)
{
	std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed_seconds = current_time - last_discomfort_penalty_time_;
	if ( ! delivered_first_discomfort_penalty_ || elapsed_seconds.count() > kDiscomfortPenaltyPeriod)
	{
		last_discomfort_penalty_time_ = std::chrono::steady_clock::now();
		discomfort_penalty = 
			shared->forward_jariness  - last_forward_jariness_ +
			shared->lateral_jariness  - last_lateral_jariness_ + 
			shared->vertical_jariness - last_vertical_jariness_;

		last_forward_jariness_ = shared->forward_jariness;
		last_lateral_jariness_ = shared->lateral_jariness;
		last_vertical_jariness_ = shared->vertical_jariness;

		if (discomfort_penalty > 0)
		{
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Discomfort penalty " << discomfort_penalty;
		}
		else if (discomfort_penalty < 0)
		{
			discomfort_penalty = 0;
		}
		delivered_first_discomfort_penalty_ = true;
	}
}

void GTAVRewardCalculator::calc_sane_driving(SharedAgentMemory* shared, double& reward, bool& done, Json::Value& info)
{
	// Returns meters of progress made towards destination minus speeding, slowness, jariness, offroad, and driving against traffic penalties.

	bool reached_destination;
	bool stalled;
	double distance_reward = get_distance_reward(shared, done, reached_destination, stalled);

	bool had_collision = false;
	bool drove_against_traffic = false;

	BOOST_LOG_SEV(lg_, boost::log::trivial::debug) << "last collision time" << shared->last_collision_time;

	if (shared->last_collision_time > 0)
	{
		// Collision = Game over
		if ( ! is_done_)
		{
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Collision! GAME OVER";
		}
		done = true;
	}

	if (shared->time_since_drove_against_traffic > 0)
	{
		// Drove against traffic = Game over
		if ( ! is_done_)
		{
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Drove against traffic! GAME OVER";
		}
		done = true;
	}

	double off_road_penalty = 0;
	get_offroad_penalty(shared, off_road_penalty);

	double slow_penalty = 0;
	double speeding_penalty = 0;
	get_speed_penalties(shared, slow_penalty, speeding_penalty);

	double discomfort_penalty = 0;
	get_discomfort_penalty(shared, discomfort_penalty);

	info["reward.distance_reward"] = distance_reward;
	info["reward.off_road_penalty"] = off_road_penalty;
	info["reward.speeding_penalty"] = speeding_penalty;
	info["reward.slow_penalty"] = slow_penalty;
	info["reward.discomfort_penalty"] = discomfort_penalty;
	info["reward.reached_destination"] = reached_destination;
	info["reward.stalled"] = stalled;

	reward = distance_reward - off_road_penalty - speeding_penalty - slow_penalty - discomfort_penalty;

	if (std::isnan(reward))
	{
		BOOST_LOG_SEV(lg_, boost::log::trivial::error) << "Reward is NaN" << reward;
		reward = 0;
	}
}

void GTAVRewardCalculator::reset(SharedAgentMemory * shared)
{
	last_reward_ = 0;
	embarked_ = false;
	closest_distance_ = 0;
	shared->dest_x = WINDING_BOTTOM_X;
	shared->dest_y = WINDING_BOTTOM_Y;
	shared->dest_z = WINDING_BOTTOM_Z;
	shared->should_reset_agent = true;
	last_forward_jariness_ = 0;
	last_lateral_jariness_ = 0;
	last_vertical_jariness_ = 0;
	is_done_ = false;
}

bool GTAVRewardCalculator::get_is_done()
{
	return is_done_;
}

void GTAVRewardCalculator::set_is_done(bool is_done)
{
	is_done_ = is_done;
}
