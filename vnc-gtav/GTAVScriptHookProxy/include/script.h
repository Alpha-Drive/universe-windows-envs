#pragma once

#include "ScenarioManager.h"
#include "ScriptHookSharedMemory.h"
#include "SharedAgentMemory.h"

//#define USE_NVIDIA_SCENARIOS

namespace OpenAIGym
{
	class Script
	{
	public:
		static void refresh(Player& player_id, Ped& player_ped, int& vehicle, SharedAgentMemory* shared);
		static void deep_drive();
		static void set_shared_mem_initial_values(SharedAgentMemory* shared);
		static void wait_for_shared_mem_to_initialize(SharedAgentMemory* shared);
		static void main();

		static void cleanup();
	private:

		static void initializeLogger();
		static void perform_temp_vehicle_action(Ped player_ped, int vehicle, SharedAgentMemory* shared);
		static void detect_if_in_vehicle(int vehicle);
		static void handle_artificial_demonstration_switching(SharedAgentMemory* shared, Ped player_ped, int vehicle, bool is_game_driving);
		static void add_debug_status_text(std::string text);
		static void get_acceleration(Vector3 speed, Vector3& acceleration, Vector3& jariness);
		static void set_reward_and_info_shared_mem(SharedAgentMemory* shared, int player, int vehicle);
		static void display_loading_paths_message();
		static void write_shared();
		static void set_camera(const int vehicle, SharedAgentMemory* shared, const int curr_vehicle);
		static Nvidia::ScenarioManager s_scenario_manager_;

		Script() = delete;
		~Script() = delete;
		Script(const Script &) = delete;
	};
	
}
