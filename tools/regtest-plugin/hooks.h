#pragma once

#include "input.h"
#include <type_traits>

namespace Hooks {
	subhook::Hook GetAsyncKeyState_hook;
	subhook::Hook CreateFile_hook;
	subhook::Hook GetLocalTime_hook;
	subhook::Hook MainLoop_hook;
	subhook::Hook ResizeSubcommand_hook;
	std::string savefile;

	// chosen by a fair dice roll.
	unsigned char random_asm[] = {
		0xb8, 0x04, 0x00, 0x00, 0x00, 0xc3
	}; // mov eax, 4; ret

	using createFile_t = std::add_pointer<decltype(CreateFileA)>::type;
	using mainLoopHook_t = __attribute__((regparm(1))) void (*)(void*);
	using resizeSubcommand_t = __attribute__((regparm(2))) void (*)(void*, int);

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

	__attribute__((regparm(2))) void MyResizeSubcommand(void* eventline, int new_size) {
		// RPG::EventScriptData does not have the fields
		using eventdata = struct {
			int gap[9];
			int subcommand_size;
			char* subcommand;
		};

		auto* data = reinterpret_cast<eventdata*>(eventline);

		if (new_size > data->subcommand_size) {
			char* subcommand_out;
			int old_size = data->subcommand_size;
			data->subcommand_size = new_size;
			asm volatile("call *%%esi"
				: "=a" (subcommand_out)
				: "S" (0x4027AC), "a" (&data->subcommand), "d" (new_size)
				: "ecx", "cc", "memory");
			memset(subcommand_out + old_size, '\xFF', new_size - old_size);
		}
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

	void HookMainLoop(mainLoopHook_t hook_fn) {
		// Hooks the UpdateInput function (directly after "inc framecounter")
		// in the RPG_RT main loop
		MainLoop_hook.Install((void *)0x46D030, (void *)hook_fn);
	}

	void HookResizeSubcommand() {
		// Initializes the subcommand array to known (255) values
		ResizeSubcommand_hook.Install((void *)0x4ABE80, (void *)MyResizeSubcommand);
	}
}
