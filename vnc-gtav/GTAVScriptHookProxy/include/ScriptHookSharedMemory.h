#pragma once

#include <SharedAgentMemory.h>

namespace OpenAIGym
{
	class ScriptHookSharedMemory
	{
	public:
		static SharedAgentMemory * shared();

		static void allocate_shared_agent_memory();
		static SharedAgentMemory* get_shared_agent_data();
		static void destroy_shared_agent_memory();

	private:
		ScriptHookSharedMemory() = delete;
		~ScriptHookSharedMemory() = delete;
		ScriptHookSharedMemory(const ScriptHookSharedMemory &) = delete;
	};
	
}
