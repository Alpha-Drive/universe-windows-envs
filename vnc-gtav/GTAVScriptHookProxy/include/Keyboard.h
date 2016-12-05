/*
		THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
					http://dev-c.com
				(C) Alexander Blade 2015
*/

#pragma once

#include <windows.h>

// parameters are the same as with aru's ScriptHook for IV
void onKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);

bool isKeyDown(DWORD key);
bool isKeyJustUp(DWORD key, bool exclusive = true);
void resetKeyState(DWORD key);