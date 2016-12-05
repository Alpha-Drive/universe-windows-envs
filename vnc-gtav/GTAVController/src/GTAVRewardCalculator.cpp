#include "GTAVRewardCalculator.h"
#include <cassert>


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
	double end_x = 0;
	double end_y = 0;
	double end_z = 0;

	// TODO: Return distance between alternating ends of road, whether we've stayed on the road, and whether we've hit something.
	if(winding_road_direction_is_down_)
	{
		end_x = WINDING_BOTTOM_X;
		end_y = WINDING_BOTTOM_Y;
		end_z = WINDING_BOTTOM_Z;
	} 
	else
	{
		end_x = WINDING_TOP_X;
		end_y = WINDING_TOP_Y;
		end_z = WINDING_TOP_Z;
	}

	double curr_x = shared->x_coord;
	double curr_y = shared->y_coord;
	double curr_z = shared->z_coord;

	double distance = sqrt( 
			pow(end_x - double(curr_x), 2) + 
			pow(end_y - double(curr_y), 2) + 
			pow(end_z - double(curr_z), 2));

	shared->distance_from_destination = distance;
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

	double speed_limit = 30;
	double speed_infraction_reward = 0;
	if (shared->speed > 20)
	{
		// Squared to outweigh distance advantage of speeding (proportional to kinetic energy - yay!)
		speed_infraction_reward = -pow(shared->speed - speed_limit, 2);
	}

	return distance_reward + collision_reward + on_road_reward + speed_infraction_reward;
}

void GTAVRewardCalculator::reset(SharedAgentMemory * shared)
{
	last_reward_ = 0;
	double end_x = WINDING_BOTTOM_X;
	double end_y = WINDING_BOTTOM_Y;
	double end_z = WINDING_BOTTOM_Z;

	last_distance_ = sqrt( 
		pow(end_x - double(shared->x_coord), 2) + 
		pow(end_y - double(shared->y_coord), 2) + 
		pow(end_z - double(shared->z_coord), 2));
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
