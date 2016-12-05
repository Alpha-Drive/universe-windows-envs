#include <json/json.h>
#include "Rewarder.h"
#include "Common.h"

Rewarder::Rewarder(int port, std::string env_id, std::shared_ptr<AgentConn> agent_conn, int frames_per_second)
{
	episode_step_counter = 0;
	episode_counter = 0;
	env_id_ = env_id;
	last_reward_pushed_at_ = std::chrono::system_clock::now();
	agent_conn_ = agent_conn;
	frames_per_second_ = frames_per_second;
}

Rewarder::~Rewarder()
{}


void Rewarder::hard_reset() {
	// We really should be creating a new object for this, which means we need to create AgentConn before the Env, then allow for bidirectional communication. 
	// Perhaps AgentConn owning the Env is the best way to do this, then reversing the direction boost signals currently flow. 
	// Signals add a lot of boilerplate, complexity, dev time, and runtime flakiness though, so would be nice to do without them.
	episode_step_counter = 0;
	episode_counter = 0;
	last_reward_pushed_at_ = std::chrono::system_clock::now();
}

void Rewarder::reset() {
	episode_step_counter = 0;
	episode_counter += 1;
}

void Rewarder::sendReward_(const double reward, const bool done, Json::Value info)
{
	P_INFO("Sending reward" << std::endl);
	int episode_id = get_episode_id();
	agent_conn_->send_reward(reward, episode_id, done, info);
}

void Rewarder::sendRewardAndIncrementStepCounter(double reward, Json::Value info)
{
	// This method is throttled to rewardRateInHertz
//	P_DEBUG("episode_step_counter: " << episode_step_counter << std::endl);
	episode_step_counter++;

	// Don't send rewards for the first couple steps
	if (episode_step_counter > 5)
	{
		sendReward_(reward, false, info);
	}
}

bool Rewarder::ready_for_reward() {
	// If it's the first few moves of the episode, do not send a reward


	// If we have sent a reward too recently, do not send a reward
	std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = current_time - last_reward_pushed_at_;

	double minimum_seconds_to_wait = 1.0 / static_cast<double>(frames_per_second_);

#ifdef PROJ_DEBUG
//	P_INFO("Time elapsed for reward check: " << elapsedSeconds.count() << " ms" << std::endl);
#endif
	bool ready = elapsed_seconds.count() > minimum_seconds_to_wait;

	if (ready)
	{
		last_reward_pushed_at_ = current_time;
	}

	return ready;
}

int Rewarder::get_episode_id()
{
	return episode_counter;
}

int Rewarder::get_frames_per_second()
{
	return frames_per_second_;
}
