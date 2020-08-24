#pragma once

#include "input.h"
#include <type_traits>

namespace Hooks {
	subhook::Hook GetAsyncKeyState_hook;
	subhook::Hook CreateFile_hook;
	subhook::Hook GetLocalTime_hook;
	std::string savefile;

	// chosen by a fair dice roll.
	unsigned char random_asm[] = {
		0xb8, 0x04, 0x00, 0x00, 0x00, 0xc3
	}; // mov eax, 4; ret

	using createFile_t = std::add_pointer<decltype(CreateFileA)>::type;

	__stdcall SHORT MyGetAsyncKeyState(int vKey) {
		return Input::IsPressed(vKey) ? 0x8000 : 0;
	}

	__stdcall HANDLE MyCreateFileA(
		LPCSTR lpFileName,
		DWORD dwDesiredAccess,
		DWORD a,
		LPSECURITY_ATTRIBUTES b,
		DWORD dwCreationDisposition,
		DWORD c,
		HANDLE d
	) {
		DWORD creatDisp = dwCreationDisposition;
		//printf("CreateFile %s %d %d\n", lpFileName, dwDesiredAccess, dwCreationDisposition);
		if (ends_with(lpFileName, "Save42.lsd") &&
			(dwDesiredAccess & GENERIC_WRITE) == GENERIC_WRITE)
		{
			lpFileName = savefile.c_str();
			dwDesiredAccess = FILE_APPEND_DATA;
			dwCreationDisposition = OPEN_ALWAYS;
		}
		return ((createFile_t)CreateFile_hook.GetTrampoline())(lpFileName, dwDesiredAccess, a, b, dwCreationDisposition, c, d);
	}

	__stdcall void MyGetLocalTime(LPSYSTEMTIME st) {
		// When converted to a Delphi Date this is 0
		st->wYear = 1899;
		st->wMonth = 12;
		st->wDayOfWeek = 6; // Saturday
		st->wDay = 30;
		st->wHour = 0;
		st->wMinute = 0;
		st->wSecond = 0;
		st->wMilliseconds = 0;
	}

	void HookGetAsyncKeyState() {
		// Hook GetAsyncKeyState
		GetAsyncKeyState_hook.Install((void *)GetAsyncKeyState, (void *)MyGetAsyncKeyState);
		CreateFile_hook.Install((void *)CreateFileA, (void *)MyCreateFileA);
	};

	void HookCreateFile(const std::string regtest_savegame) {
		CreateFile_hook.Install((void *)CreateFileA, (void *)MyCreateFileA);
		savefile = regtest_savegame;
	}

	void HookRng() {
		// Hook RNG
		memcpy((DWORD*)0x00403054, random_asm, 6);
	}

	void HookGetLocalTime() {
		GetLocalTime_hook.Install((void *)GetLocalTime, (void *)MyGetLocalTime);
	}
}
