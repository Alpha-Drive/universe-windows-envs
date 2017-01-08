
#define _USE_MATH_DEFINES
#include <cmath>

#include <Windows.h>

#include <AutoItX3_DLL.h>
#include <string>
#include <vector>
#include <GTAVKeyboardController.h>
#include <thread>
#include <boost/thread/csbl/memory/unique_ptr.hpp>
#include <Common.h>
#include <Utils.h>
#include <tchar.h>

// TODO: Move this to the client and do keyboard control over VNC.
// TODO: Do as much control as possible through the joystick in GTAV, as it's less likely to send key strokes to things outside the game.
std::wstring s2ws(const std::string& s)
{
	std::wstring stemp = std::wstring(s.begin(), s.end());
	LPCWSTR sw = stemp.c_str();
	return sw;
}

void send_key(const std::string & key, const int down_up_wait_time = kUpDownWaitTime, const int between_key_wait_time = kBetweenKeyWaitTime)
{
	AU3_WinActivate(L"Grand Theft Auto V", L"");
	AU3_Sleep(between_key_wait_time);
	std::string cmd_up = kKeyStart + key + kKeyUp;
	std::string cmd_down = kKeyStart + key + kKeyDown;
	AU3_Send(s2ws(cmd_down).c_str()); 	// i.e. AU3_Send(L"{RIGHT down}");
	AU3_Sleep(down_up_wait_time);
	AU3_Send(s2ws(cmd_up).c_str());	    // i.e. AU3_Send(L"{RIGHT up}");
}

void send_keys(const std::vector<std::string> & keys, const int down_up_wait_time = kUpDownWaitTime, const int between_key_wait_time = kBetweenKeyWaitTime)
{
	for(std::string key : keys)
	{
		P_WARN("Sending " << key << " keystroke" << std::endl);

		send_key(key, down_up_wait_time, between_key_wait_time);
	}	
}


void wait_for_win_to_exist()
{
	bool found = false;
	while ( ! found )
	{
		found = AU3_WinExists(L"Grand Theft Auto V", L"");
		if(found)
		{
			HWND hwnd = FindWindow(NULL, L"Grand Theft Auto V");
			TCHAR class_name[MAX_PATH];
			GetClassName(hwnd, class_name, _countof(class_name));

			TCHAR expected_class_name[MAX_PATH] = L"grcWindow";
			TCHAR folder_class_name[MAX_PATH] = L"CabinetWClass";

			if(_tcscmp(class_name, folder_class_name) == 0)
			{
				P_WARN("Warning: closing \"Grand Theft Auto V\" explorer window as it will clash for control with game window" << std::endl);
				PostMessage(hwnd, WM_CLOSE, 0, 0);
				found = false;
			}
			else if(_tcscmp(class_name, expected_class_name) != 0)
			{
				found = false;
			}
		}

		if ( ! found)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
			P_INFO("Waiting for GTAV Window to exist" << std::endl);
		}
	}
}

void try_loading_story_mode()
{
	wait_for_win_to_exist();

	AU3_WinActivate(L"Grand Theft Auto V", L"");

	P_WARN("\n\nSending keys to load story mode. Leave the game open to skip this step in the future. \n\n");

	// We're not sure when the loading splash comes up so button mash
	// TODO: Switch to Universe vexpect or https://github.com/MyBotRun/Libraries/tree/master/ImageSearchDLL
	send_keys({ "RIGHT", "RIGHT", "RIGHT", "RIGHT", "RIGHT", "ENTER", "ENTER" });
}

void wait_for_script_hook_to_load(SharedAgentMemory * shared)
{
	bool has_loaded = false;
	P_INFO("Waiting for GTAVScriptHookProxy to load (make sure you're in a car, the game is not paused, and that you are running as Administrator)" << std::endl);
	while ( ! has_loaded )
	{
		int game_seconds = shared->time.second;
		std::this_thread::sleep_for(std::chrono::milliseconds(311)); // Should be about 30ms per game second, so wait ~10x that
		has_loaded = shared->time.second != game_seconds;
	}
	P_INFO("GTAVScriptHookProxy has loaded" << std::endl);
}

void load_saved_game()
{
	// Open main menu
	send_key("ESC");

	P_WARN("\n\nDO NOT TOUCH MOUSE OR KEYBOARD. We are sending keys to GTAV to reload game\n\n");

	// Main menu takes a while to come up
	AU3_Sleep(3000);

	// Go to game -> load game and select the most recently saved game
	send_keys({"RIGHT", "RIGHT", "RIGHT", "RIGHT", "ENTER", "DOWN", "DOWN", "ENTER", "ENTER", "ENTER"});
}