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

	std::shared_ptr<AgentConn> agent_conn(new AgentConn(9002));

	BOOST_LOG_SEV(lg, boost::log::trivial::debug) << "setup agent_conn";

#ifdef PROJ_DEBUG
  P_DEBUG("Press ENTER to continue..." << std::endl);
  std::cin.ignore();
#endif

	return 0;
}
