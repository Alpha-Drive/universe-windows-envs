#pragma once

#ifndef GTAV_REWARD_CALCULATOR_H_
#define GTAV_REWARD_CALCULATOR_H_

#include "Common.h"
#include <GTAVControllerSharedMemory.h>

enum GTAVRewardMode
{
	SANE_DRIVING,
	SPEED,
	WINDING_ROAD,
};

#define WINDING_TOP_X -2648.517334;
#define WINDING_TOP_Y 1498.584106;
#define WINDING_TOP_Z 118.599236;
#define WINDING_BOTTOM_X -2090.888184;
#define WINDING_BOTTOM_Y 2298.642578;
#define WINDING_BOTTOM_Z 37.532829;

class GTAVRewardCalculator
{
public:
	GTAVRewardCalculator(std::string envId);
	~GTAVRewardCalculator();
	void reset(SharedAgentMemory* shared);
	double get_reward(SharedAgentMemory* shared);
private:
	double calc_sane_driving(SharedAgentMemory* shared);
	GTAVRewardMode reward_mode_;
	double last_reward_;
	bool winding_road_direction_is_down_ = true;
	double last_distance_;
	int last_collision_time_;
};

#endif // !GTAV_REWARD_CALCULATOR_H_