#pragma once

#include "input.h"

namespace Hooks {
	subhook::Hook GetAsyncKeyState_hook;

	unsigned char random_asm[] = {
		0xb8, 0x04, 0x00, 0x00, 0x00, 0xc3
	}; // mov eax, 4; ret

	__stdcall SHORT MyGetAsyncKeyState(int vKey) {
		return Input::IsPressed(vKey) ? 0x8000 : 0;
	}

	void Install() {
		// Hook GetAsyncKeyState
		auto user32 = LoadLibrary("User32.dll");
		auto addr = GetProcAddress(user32, "GetAsyncKeyState");
		GetAsyncKeyState_hook.Install((void *)addr, (void *)MyGetAsyncKeyState);

		// Hook RNG
		memcpy((DWORD*)0x00403054, random_asm, 6);
	};
}
