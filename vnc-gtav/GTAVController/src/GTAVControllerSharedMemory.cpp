#include <SharedAgentMemory.h>
#include <windows.h>
#include <chrono>
#include <thread>
#include <iostream>
#include "Common.h"

SharedAgentMemory shared_agent_memory;

SharedAgentMemory * get_shared_agent_data()
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

SharedAgentMemory * wait_for_shared_agent_memory(int milliseconds)
{
	SharedAgentMemory* _shared = nullptr;
	P_INFO("Trying to access shared agent data... If GTAVScriptHookProxy.asi was not in the GTAV folder when the game started, copy it there and restart the game." << std::endl);
	while (_shared == nullptr && milliseconds > 0) {
		_shared = get_shared_agent_data();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		milliseconds--;
	}
	return _shared;
}

SharedAgentMemory * shared();