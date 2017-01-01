#include "GTAVRewardCalculator.h"
#include <cassert>
#include "../../GTAVScriptHookProxy/vendor/ScriptHookV/include/natives.h"


GTAVRewardCalculator::GTAVRewardCalculator(std::string envId)
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
}

GTAVRewardCalculator::~GTAVRewardCalculator()
{}

double GTAVRewardCalculator::calc_sane_driving(SharedAgentMemory * shared)
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

	double distance = shared->distance_from_destination;

	double distance_reward;
	distance_reward = last_distance_ - distance;
	if(distance < 10)
	{
		// Turn around
		winding_road_direction_is_down_ = ! winding_road_direction_is_down_;
	}
	last_distance_ = distance;

	double collision_reward = 0;
	if (last_collision_time_ != shared->last_collision_time)
	{
		// Accidents per mile is 1 / 165,000 (1 / 7.229e9 meters) in U.S. - be ten times better (since we get +1 reward per meter)
		collision_reward = -pow(7.229, 9) * 10;
		last_collision_time_ = shared->last_collision_time;
	}
	double on_road_reward = 0;
	if (shared->on_road == false)
	{
		// Guessing this may need to happen once in a while (for multiple seconds) if we are blocked
		on_road_reward = -pow(1.6, 6) / (5 * 8); // every 100 miles (in meters) over 5 seconds at 8FPS
	}

	double speed_limit = 20;
	double speed_infraction_reward = 0;
	if (shared->speed > speed_limit)
	{
		// Squared to outweigh distance advantage of speeding (proportional to kinetic energy - yay!)
		speed_infraction_reward = -pow(shared->speed - speed_limit, 2);
	}

	return distance_reward + collision_reward + on_road_reward + speed_infraction_reward;
}

void GTAVRewardCalculator::reset(SharedAgentMemory * shared)
{
	last_reward_ = 0;
	shared->dest_x = WINDING_BOTTOM_X;
	shared->dest_y = WINDING_BOTTOM_Y;
	shared->dest_z = WINDING_BOTTOM_Z;
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
