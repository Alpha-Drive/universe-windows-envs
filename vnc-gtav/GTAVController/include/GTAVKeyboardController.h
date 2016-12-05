#ifndef GTAV_KEYBOARD_CONTROLLER_H_
#define GTAV_KEYBOARD_CONTROLLER_H_

#include <string>
#include <SharedAgentMemory.h>


const int kBetweenKeyWaitTime = 500;
const int kUpDownWaitTime = 200;
const std::string kKeyStart = "{";
const std::string kKeyDown = " down}";
const std::string kKeyUp = " up}";

void load_saved_game();
void try_loading_story_mode();
void wait_for_script_hook_to_load(SharedAgentMemory * shared);


#endif // !GTAV_KEYBOARD_CONTROLLER_H_