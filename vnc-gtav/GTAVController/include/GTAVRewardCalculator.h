#pragma once

#ifndef GTAV_REWARD_CALCULATOR_H_
#define GTAV_REWARD_CALCULATOR_H_

#include "Common.h"
#include <GTAVControllerSharedMemory.h>
#include <chrono>
#include <json/json.h>
#include <atomic>

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

const double kEndDistance = 10;
const double kMaxSecondsNoProgress = 20;
const double kMaxOffroadSeconds = 1;

// TODO: Use one reward / penaty period and DRY up the throttling code
const double kSlowPenaltyPeriod = 1;
const double kSpeedingPenaltyPeriod = 1;
const double kDiscomfortPenaltyPeriod = 1;


class GTAVRewardCalculator
{
public:
	GTAVRewardCalculator(std::string envId, boost::log::sources::severity_logger_mt<ls::severity_level> lg);
	~GTAVRewardCalculator();
	void set_destination(SharedAgentMemory* shared);
	void reset(SharedAgentMemory* shared);
	void get_reward(SharedAgentMemory* shared, double& reward, bool& done, Json::Value& info);
	bool get_is_done();
	void set_is_done(bool is_done);
private:
	double get_distance_reward(SharedAgentMemory* shared, bool& done, bool& reached_destination, bool& stalled);
	void get_speed_penalties(SharedAgentMemory* shared, double& slow_penalty, double& speed_infraction_penalty);
	void get_offroad_penalty(SharedAgentMemory* shared, double& off_road_penalty);
	void get_discomfort_penalty(SharedAgentMemory* shared, double& discomfort_penalty);
	void calc_sane_driving(SharedAgentMemory* shared, double& reward, bool& done, Json::Value& info);
	GTAVRewardMode reward_mode_;
	double last_reward_;
	bool winding_road_direction_is_down_ = true;
	std::atomic<bool> is_done_;
	boost::log::sources::severity_logger_mt<ls::severity_level> lg_;
	bool embarked_ = false;
	double closest_distance_ = 0;
	double current_distance_;
	double last_forward_jariness_ = 0;
	double last_lateral_jariness_ = 0;
	double last_vertical_jariness_ = 0;

	// TODO: DRY up these throttling fields
	std::chrono::time_point<std::chrono::system_clock> last_closest_distance_time_;
	std::chrono::time_point<std::chrono::system_clock> last_on_road_time_;
	std::chrono::time_point<std::chrono::system_clock> last_slow_penalty_time_;
	std::chrono::time_point<std::chrono::system_clock> last_speeding_time_;
	std::chrono::time_point<std::chrono::system_clock> last_discomfort_penalty_time_;
	bool delivered_first_slow_penalty_ = false;
	bool delivered_first_speeding_penatly_ = false;
	bool delivered_first_discomfort_penalty_ = false;
};

#endif // !GTAV_REWARD_CALCULATOR_H_