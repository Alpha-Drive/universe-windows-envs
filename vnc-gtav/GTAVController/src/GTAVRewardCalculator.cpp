#include "GTAVRewardCalculator.h"
#include <cassert>
#include "../../GTAVScriptHookProxy/vendor/ScriptHookV/include/natives.h"
#include <thread>


GTAVRewardCalculator::GTAVRewardCalculator(std::string envId, boost::log::sources::severity_logger_mt<ls::severity_level> lg)
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

double GTAVRewardCalculator::get_distance_reward(SharedAgentMemory* shared)
{
	// Returns meters of progress made towards destination.

	set_destination(shared);
	double distance = shared->distance_from_destination;
	if ( ! embarked_ && distance >= kEndDistance)
	{
		embarked_ = true;
		closest_distance_ = distance;
		last_closest_distance_time_ = std::chrono::steady_clock::now();
	}

	double distance_reward = 0;
	if (distance >= 0)
	{
		// Pathfind returned actual distance (i.e. not -1 when not found)
		if (closest_distance_ > distance)
		{
			// We've made progress
			distance_reward = closest_distance_ - distance;
			closest_distance_ = distance;
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
				BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "No progress in last " << kMaxSecondsNoProgress << " seconds. Ending episode.";
				is_done_ = true;
			}
		}
	}

	if (embarked_ && distance >= 0 && distance < kEndDistance)
	{
		// We may have reached our destination
		double eucl_distance = sqrt( 
			pow(shared->dest_x - double(shared->x_coord), 2) + 
			pow(shared->dest_y - double(shared->y_coord), 2) + 
			pow(shared->dest_z - double(shared->z_coord), 2));

		if (eucl_distance < kEndDistance) {
			// Euclidean distance agrees! We're close.
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
			is_done_ = true;
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Reached destination, finishing episode: distance from destination " << distance;
#endif
		}
	}
	return distance_reward;
}

void GTAVRewardCalculator::get_speed_penalties(SharedAgentMemory* shared, double& slow_penalty, double& speed_infraction_penalty)
{
	double speed_limit = 25; // meters per second // TODO: Pull this dynamically from an IPL file
	double speed = shared->speed;
	double expected_avg_speed = speed_limit * 0.95;
	if (speed > speed_limit)
	{
		std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = current_time - last_speed_infraction_time_;
		if ( ! delivered_first_speed_infraction_penatly_ || elapsed_seconds.count() > kSpeedInfractionPenaltyPeriod)
		{
			last_speed_infraction_time_ = std::chrono::steady_clock::now();
			// Squared to outweigh distance advantage of speeding (proportional to kinetic energy - yay!)
			speed_infraction_penalty = pow(shared->speed - speed_limit, 2);
			delivered_first_speed_infraction_penatly_ = true;
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Speed infraction penalty";
		}

	}
	else if (speed < expected_avg_speed)
	{
		// Too slow
		std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = current_time - last_slow_penalty_time_;
		if ( ! delivered_first_slow_penalty_ || elapsed_seconds.count() > kSlowPenaltyPeriod)
		{
			last_slow_penalty_time_ = std::chrono::steady_clock::now();
			slow_penalty = 1;
			delivered_first_slow_penalty_ = true;
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Driving too slow penalty";
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
			off_road_penalty = 1000; // rewards are in scale of meters
			BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Driving offroad penalty";

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
			shared->forward_jariness - last_forward_jariness_ +
			shared->lateral_jariness - last_lateral_jariness_ + 
			shared->vertical_jariness - last_vertical_jariness_;
		delivered_first_slow_penalty_ = true;
	}
}

double GTAVRewardCalculator::calc_sane_driving(SharedAgentMemory * shared)
{
	// All rewards on scale of meters towards destination

	double distance_reward = get_distance_reward(shared);

	if (shared->last_collision_time > 0)
	{
		// Collision = Game over
		BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Collision! GAME OVER";
		is_done_ = true;
	}

	if (shared->time_since_drove_against_traffic > 0)
	{
		// Drove against traffic = Game over
		BOOST_LOG_SEV(lg_, boost::log::trivial::info) << "Drove against traffic! GAME OVER";
		is_done_ = true;
	}

	double off_road_penalty = 0;
	get_offroad_penalty(shared, off_road_penalty);

	double slow_penalty = 0;
	double speed_infraction_penalty = 0;
	get_speed_penalties(shared, slow_penalty, speed_infraction_penalty);

	double discomfort_penalty = 0;
	get_discomfort_penalty(shared, discomfort_penalty);

	return distance_reward - off_road_penalty - speed_infraction_penalty - slow_penalty - discomfort_penalty;
}

void GTAVRewardCalculator::reset(SharedAgentMemory * shared)
{
	last_reward_ = 0;
	embarked_ = false;
	shared->dest_x = WINDING_BOTTOM_X;
	shared->dest_y = WINDING_BOTTOM_Y;
	shared->dest_z = WINDING_BOTTOM_Z;
	shared->should_reset_agent = true;
}

// Return the rewards that we have aggregated since the last call to getReward()
double GTAVRewardCalculator::get_reward(SharedAgentMemory * shared)
{
	double current_reward = 0;
	switch (reward_mode_)
	{
	case SANE_DRIVING:
		current_reward = calc_sane_driving(shared);
		break;
	case SPEED:
		current_reward = shared->speed;
		break;
	default:
		P_ERR("Reward mode unrecognised!" << std::endl);
		break;
	}
	return current_reward;
}

bool GTAVRewardCalculator::get_is_done()
{
	return is_done_;
}

void GTAVRewardCalculator::set_is_done(bool is_done)
{
	is_done_ = is_done;
}
