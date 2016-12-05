#ifndef GTAV_ENV_H_
#define GTAV_ENV_H_

#include "Rewarder.h"
#include "Env.h"
#include "SharedAgentMemory.h"
#include "GTAVRewardCalculator.h"

class GTAVEnv : public Env
{
public:
	void set_win_active_to_long_ago();
	GTAVEnv(std::string env_id, std::string instance_id, int websocket_port, std::shared_ptr<AgentConn> agent_conn, bool skip_loading_saved_game, int rewards_per_second);
	~GTAVEnv();
	void noop();
	void loop() override;
	void connect() override;
	void step() override;
	bool is_done() override;
	void change_settings(const Json::Value& settings) override;
	
	// N.B. Using override in conjunction with boost signals causes a "pure virtual function call" error - thus no override below
	void reset_game();
	void after_reset();
	void when_no_clients();

private:
	GTAVRewardCalculator reward_calculator_;
	std::unique_ptr<SharedAgentMemory> shared_;
	bool skip_loading_saved_game_;
	std::chrono::time_point<std::chrono::system_clock> last_win_activate_at_;
	void check_win_active_every_n_secs(int seconds);
};

#endif // !GTAV_ENV_H_