#include "GTAVEnv.h"
#include <SharedAgentMemory.h>
#include <GTAVControllerSharedMemory.h>
#include <GTAVKeyboardController.h>
#include <boost/signals2.hpp>
#include "GTAVRewardCalculator.h"
#include <AutoItX3_DLL.h>
#include "stdafx.h"
#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;

GTAVEnv::GTAVEnv(std::string env_id, std::string instance_id, int websocket_port, std::shared_ptr<AgentConn> agent_conn, bool skip_loading_saved_game, int rewards_per_second) : 
	Env(env_id, instance_id, websocket_port, agent_conn, rewards_per_second),
	reward_calculator_(env_id)
{
	skip_loading_saved_game_ = skip_loading_saved_game;
	set_win_active_to_long_ago();
}

GTAVEnv::~GTAVEnv()
{
}

void GTAVEnv::noop()
{
	joystick_.set_x_axis(0); // steer zero
	joystick_.set_z_axis(0); // throttle zero
}

void GTAVEnv::loop()
{
	connect(); // Blocks until GTAV starts
	BOOST_LOG_SEV(lg_, boost::log::trivial::debug) << "connected to gtav";
	noop(); // In case joystick defaults have us moving already
	int steps = 0;
	while (true) // EPISODE LOOP
	{
		while (true) // STEP LOOP
		{			
			check_win_active_every_n_secs(10);
			step();
			steps++;
		} // END STEP LOOP

	} // END EPISODE LOOP
}

void GTAVEnv::connect()
{
	shared_.reset(wait_for_shared_agent_memory(10));
	if(shared_)
	{
		P_INFO("Game already open, reusing." << std::endl);
	}
	while( ! shared_ )
	{
		shared_.reset(wait_for_shared_agent_memory(10));
		P_INFO("Attempting to load story mode and get accesss to shared agent data" << std::endl);
		try_loading_story_mode();
	}

	P_INFO("Loading saved game" << std::endl);
	reset_game();
}

void GTAVEnv::step()
{
	// Push rewards at our throttled rate
	if (rewarder.ready_for_reward()) {
		Json::Value info;
		info["speed"]                       = shared_->speed;
		info["spin"]                        = shared_->spin;
		info["x_coord"]                     = shared_->x_coord;
		info["y_coord"]                     = shared_->y_coord;
		info["z_coord"]                     = shared_->z_coord;
		info["heading"]                     = shared_->heading;
		info["is_game_driving"]             = shared_->is_game_driving;
		info["script_hook_loadtime"]        = shared_->script_hook_loadtime;
		info["on_road"]                     = shared_->on_road;
		info["center_of_lane_reward"]       = shared_->center_of_lane_reward; // TODO: Add distance from right / left lanes
		info["game_time.second"]            = shared_->time.second;
		info["game_time.minute"]            = shared_->time.minute;
		info["game_time.hour"]              = shared_->time.hour;
		info["game_time.day_of_month"]      = shared_->time.day_of_month;
		info["game_time.month"]             = shared_->time.month;
		info["game_time.year"]              = shared_->time.year;
		info["game_time.ms_per_game_min"]   = shared_->time.ms_per_game_min;
		info["last_collision_time"]         = shared_->last_collision_time;
		info["last_material_collided_with"] = std::to_string(shared_->last_material_collided_with); // unsigned long to json hack

		// Forward vector for measuring accerlation / comfort
		info["forward_vector_x"]            = shared_->forward_vector.x;
		info["forward_vector_y"]            = shared_->forward_vector.y;
		info["forward_vector_z"]            = shared_->forward_vector.z;

		info["distance_from_destination"]   = shared_->distance_from_destination;

		rewarder.sendRewardAndIncrementStepCounter(reward_calculator_.get_reward(shared_.get()), info);
	}
}

bool GTAVEnv::is_done()
{
	return false;
}

void GTAVEnv::reset_game()
{
	wait_for_script_hook_to_load(shared_.get());
	if(skip_loading_saved_game_)
	{
		BOOST_LOG_SEV(lg_, ls::warning) << "skip_loading_saved_game is set, skipping reload - you should turn this off if you are not debugging, simulating reset delay...";

		// Zero reset time causes a race condition in universe client where `completed_episode_id` differs from `_current_episode_id` too quickly
		std::this_thread::sleep_for(std::chrono::seconds(1));

		BOOST_LOG_SEV(lg_, ls::warning) << "fake reset done";
	}
	else
	{
		load_saved_game();
	}
	reward_calculator_.reset(shared_.get());
}

void GTAVEnv::after_reset()
{
	noop();
}

void GTAVEnv::when_no_clients()
{
	BOOST_LOG_SEV(lg_, ls::info) << "no clients, sending noop";
	noop();
}

void GTAVEnv::check_win_active_every_n_secs(int seconds)
{
	std::chrono::time_point<std::chrono::system_clock> current_time = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = current_time - last_win_activate_at_;
	if(elapsed_seconds.count() > seconds)
	{
		AU3_WinActivate(L"Grand Theft Auto V", L"");
		last_win_activate_at_ = current_time;
	}
}

void GTAVEnv::set_win_active_to_long_ago()
{
	std::string ts("2000-01-01 23:59:59.000");
	ptime t(time_from_string(ts));
	tm pt_tm = to_tm( t );
	last_win_activate_at_ = std::chrono::system_clock::from_time_t(std::mktime(&pt_tm));
}

void GTAVEnv::change_settings(const Json::Value& settings)
{
	// Note that settings should be carefully chosen so as to not allow reward hacking.
	// Only things we would expect an agent to control like the position / rotation of its head e.g. via the camera
	// should be added. e.g. no cheat codes >)
	if (settings[1].asString() == "use_custom_camera")
	{
		(*shared_).use_custom_camera = settings[2].asBool();
		BOOST_LOG_SEV(lg_, ls::info) << "agent set custom camera to true";
	}
	if (settings[1].asString() == "desired_cam_x_offset")
	{
		(*shared_).desired_cam_x_offset = settings[2].asDouble();
		(*shared_).use_custom_camera = true;
	}
	if (settings[1].asString() == "desired_cam_y_offset")
	{
		(*shared_).desired_cam_y_offset = settings[2].asDouble();
		(*shared_).use_custom_camera = true;
	}
	if (settings[1].asString() == "desired_cam_z_offset")
	{
		(*shared_).desired_cam_z_offset = settings[2].asDouble();
		(*shared_).use_custom_camera = true;
	}
}