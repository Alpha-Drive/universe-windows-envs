#include <windows.h>
#include "ScriptHookSharedMemory.h"
#include <future>
#include <sstream>

SharedAgentMemory * shared_agent_memory;

HANDLE agent_file_map;
LPBYTE lp_agent_shared_memory = NULL;

using namespace OpenAIGym;

void ScriptHookSharedMemory::allocate_shared_agent_memory()
{
	shared_agent_memory = get_shared_agent_data();

	if(shared_agent_memory != nullptr)
	{
		return;
	}

	SharedAgentMemory **agent_data = &shared_agent_memory;
    int totalSize = sizeof(SharedAgentMemory);

	std::stringstream strName;
    strName << AGENT_SHARED_MEMORY;
    agent_file_map = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
		PAGE_READWRITE, 0, totalSize, strName.str().c_str());

    if(!agent_file_map)
    {
	    return;
    }

    lp_agent_shared_memory = static_cast<LPBYTE>(
		MapViewOfFile(agent_file_map, FILE_MAP_ALL_ACCESS, 0, 0, totalSize));

    if(!lp_agent_shared_memory)
    {
        CloseHandle(agent_file_map);
        agent_file_map = NULL;
		return;
    }
    *agent_data = reinterpret_cast<SharedAgentMemory*>(lp_agent_shared_memory); // Casting is important, do not remove.
}

SharedAgentMemory * ScriptHookSharedMemory::get_shared_agent_data()
{
	HANDLE hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, AGENT_SHARED_MEMORY);
	if (hFileMap == NULL)
	{
		return nullptr;
	}

	// Cast to our shared struct
	SharedAgentMemory* infoIn = static_cast<SharedAgentMemory*>(MapViewOfFile(
		hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SharedAgentMemory)));

	if (!infoIn)
	{
		CloseHandle(hFileMap);
		return nullptr;
	}
	return infoIn;
}

void ScriptHookSharedMemory::destroy_shared_agent_memory()
{
    if(lp_agent_shared_memory && agent_file_map)
    {
        UnmapViewOfFile(lp_agent_shared_memory);
        CloseHandle(agent_file_map);

        agent_file_map = NULL;
        lp_agent_shared_memory = NULL;
    }
	delete shared_agent_memory;
}

SharedAgentMemory * ScriptHookSharedMemory::shared()
{
	return shared_agent_memory;
}