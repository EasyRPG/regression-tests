#include <DynRPG/DynRPG.h>
#include <iostream>
#include <fstream>
#include <ctime>
#include <sstream>
#include <vector>
#include <algorithm>

#define SUBHOOK_STATIC
#define SUBHOOK_X86
#include "subhook.h"

#include "hooks.h"
#include "input.h"
#include "utils.h"

struct {
	std::string regtest_savegame;
	std::string input_file;
	bool do_regtest = false;
	bool do_input_playback = false;
	bool enable_hooks = true;
} config;

void ParseCommandLine() {
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	for (int i = 1; i < argc; ++i) {
		if (!lstrcmpW(argv[i], L"--replay-input")) {
			++i;
			if (i < argc) {
				config.input_file = wchar_to_char(argv[i]);
				config.do_input_playback = !config.input_file.empty();
				std::cout << "Replay: " << config.input_file << "\n";
			}
		}
		else if (!lstrcmpW(argv[i], L"--regression-test")) {
			++i;
			if (i < argc) {
				config.regtest_savegame = wchar_to_char(argv[i]);
				config.do_regtest = !config.regtest_savegame.empty();
				std::cout << "Regression Test: " << config.regtest_savegame << "\n";
			}
		}
		else if (!lstrcmpW(argv[i], L"--disable-hooks")) {
			config.enable_hooks = false;
			std::cout << "All hooks disabled\n";
		}
	}
}

__attribute__((regparm(1))) void MainLoopHook(void* a1);

bool onStartup(char *pluginName) {
	ParseCommandLine();

	if (!config.enable_hooks) {
		return true;
	}

	if (config.do_input_playback) {
		Hooks::HookGetAsyncKeyState();
		Input::Init(config.input_file);
	}

	if (config.do_regtest) {
		DeleteFileA(config.regtest_savegame.c_str());
		Hooks::HookCreateFile(config.regtest_savegame);
	}

	if (config.do_regtest || config.do_input_playback) {
		Hooks::HookMainLoop(MainLoopHook);
	}

	Hooks::HookRng();
	Hooks::HookGetLocalTime();
	Hooks::HookResizeSubcommand();

	return true;
}

void onNewGame() {
	if (!config.enable_hooks) {
		return;
	}

	// Touch all actor conditions once, this initializes the status array to 0....0
	// because Player is not savegame compatible here
	for (int i = 0; i < RPG::actors.count(); ++i) {
		RPG::Actor* actor = RPG::actors[i + 1];
		asm volatile("call *%%esi"
			:
			: "S" (0x4BFF8C), "a" (actor), "d" (1)
			: "ecx", "cc", "memory");
	}
}

__attribute__((regparm(1))) void MainLoopHook(void* a1) {
	if (!config.enable_hooks) {
		return;
	}

	static int last_written_save = 0;

	if (config.do_input_playback) {
		printf("%d ", RPG::system->frameCounter);

		if (Input::IsDone()) {
			printf("Replay ended\n");
			PostMessage(NULL, WM_QUIT, 0, 0);
			return;
		}

		Input::Update();

		// Invoke the input update timer function
		// RPG_RT invokes it through a timer but this is not deterministic
		asm volatile("call *%%esi"
			:
			: "S" (0x46CA14), "a" (RPG::input)
			: "ecx", "edx", "cc", "memory");


		printf("\n");
	}

	if (RPG::system->scene != RPG::SCENE_TITLE &&
			config.do_regtest &&
			RPG::system->frameCounter > last_written_save) {
		RPG::fileSaveLoad->saveFile(42);
		last_written_save = RPG::system->frameCounter;
	}

	((Hooks::mainLoopHook_t)Hooks::MainLoop_hook.GetTrampoline())(a1);
}


void onExit() {
	// no-op
}
