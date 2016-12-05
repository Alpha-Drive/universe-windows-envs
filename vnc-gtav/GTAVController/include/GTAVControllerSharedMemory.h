#ifndef GTAV_CONTROLLER_SHARED_MEMORY_H_
#define GTAV_CONTROLLER_SHARED_MEMORY_H_

#include <SharedAgentMemory.h>

SharedAgentMemory * get_shared_agent_data();
SharedAgentMemory * wait_for_shared_agent_memory(int milliseconds);

#endif // !GTAV_CONTROLLER_SHARED_MEMORY_H_

