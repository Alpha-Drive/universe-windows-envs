#define NOMINMAX
#define _USE_MATH_DEFINES
#include <future>
#include <SharedAgentMemory.h>
#include <windows.h>
#include <GTAVKeyboardController.h>
#include <GTAVControllerSharedMemory.h>
#include <iostream>
#include <Common.h>

#include <AgentConn.h>
#include <GTAVEnv.h>

int main(int argc, const char* argv[])
{
	init_logger();

	boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> lg;

	BOOST_LOG_SEV(lg, boost::log::trivial::info) << "Starting GTAV controller...";

	if (argc < 6)
	{
		P_ERR("Format is: GTAVController.exe <env_id> <instance_id> <websocket_port> <skip_loading_saved_game> <rewards_per_second> <stop_for_attach>" << std::endl);
		P_ERR("e.g. GTAVController.exe \"GTAVController-v0\" \"env_d753500d080a948\" 15900" << std::endl);
		return 1;
	}

	P_WARN("*****************************************" << std::endl);
	P_INFO("**            GTAVController           **" << std::endl);
	P_INFO("**  Compiled on " << __DATE__ << ", " << __TIME__ << "  **" << std::endl);
	P_WARN("*****************************************" << std::endl);
	P_INFO("Please make sure to leave Grand Theft Auto V open and on top of any " <<
		"other window!" << std::endl);


	std::string env_id = argv[1];
	std::string env_instance_id = argv[2];
	std::string websocket_port_string = argv[3];
	std::string rewards_per_second_string = argv[4];
	std::string skip_loading_saved_game_string = argv[5];
	int websocket_port = atoi(websocket_port_string.c_str());
	int rewards_per_second = atoi(rewards_per_second_string.c_str());
	bool skip_loading_saved_game = skip_loading_saved_game_string == "true";


	std::string pause_for_attach_str = argv[6];
	P_INFO("Pause for attach " + pause_for_attach_str);
	bool stop_for_attach = pause_for_attach_str == "true";
	if(stop_for_attach)
	{
		P_DEBUG("\n\n\n>>>>>>>>>>>>>>>> Press ENTER after you have chosen 'Attach to Process...' in the Visual Studio DEBUG menu. <<<<<<<<<<<<<<<<\n\n\n");
		std::cin.ignore();
	}

	BOOST_LOG_SEV(lg, boost::log::trivial::debug) << "parsed arguments";

	std::shared_ptr<AgentConn> agent_conn(new AgentConn(websocket_port));

	BOOST_LOG_SEV(lg, boost::log::trivial::debug) << "setup agent_conn";

	GTAVEnv env(env_id, env_instance_id, websocket_port, agent_conn, skip_loading_saved_game, rewards_per_second);

	BOOST_LOG_SEV(lg, boost::log::trivial::debug) << "instantiated environment";

	env.loop();

#ifdef PROJ_DEBUG
  P_DEBUG("Press ENTER to continue..." << std::endl);
  std::cin.ignore();
#endif

	return 0;
}
