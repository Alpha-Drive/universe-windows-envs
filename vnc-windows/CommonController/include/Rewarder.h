#ifndef REWARDER_H_
#define REWARDER_H_
#include "AgentConn.h"

#include "Common.h"
#include <ctime>
#include <chrono>

const int kSecsToWaitBetweenRewardLogs = 2;

class Rewarder
{
public:
	Rewarder(int websocket_port, std::string env_id, std::shared_ptr<AgentConn> agent_conn, int rewards_per_second, boost::log::sources::severity_logger_mt<ls::severity_level> lg);
	~Rewarder();
	void sendRewardAndIncrementStepCounter(const double reward, const bool done, Json::Value& info);
	bool ready_for_reward();
	int get_episode_id();
	int get_frames_per_second();
	void sendIsDone();
	void sendHeartbeat();
	int episode_step_counter;
	int episode_counter;
	double episode_return;
	int frames_per_second_;
	void reset();
	void hard_reset();
private:
	void sendReward_(const double reward, const bool done, Json::Value& info);
	// TODO: Convert shared pointer to unique pointer by transferring ownership and related
	// functions to Env.cpp - possibly eliminate this class altogether.
	std::shared_ptr<AgentConn> agent_conn_;
	
	std::chrono::time_point<std::chrono::system_clock> last_reward_pushed_at_;
	std::chrono::time_point<std::chrono::system_clock> last_reward_log_at_;
	std::string env_id_;
	boost::log::sources::severity_logger_mt<ls::severity_level> lg_;
};

#endif // !REWARDER_H_