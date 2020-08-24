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
	}
}

bool onStartup(char *pluginName) {
	DWORD old_protect;
	VirtualProtect((char*)0x400000, 0x100000, PAGE_EXECUTE_READWRITE, &old_protect);

	ParseCommandLine();

	if (config.do_input_playback) {
		Hooks::HookGetAsyncKeyState();
		Input::Init(config.input_file);
	}

	return true;
}

void onFrame (RPG::Scene scene) {
	if (config.do_input_playback) {
		printf("%d ", RPG::system->frameCounter);

		if (Input::IsDone()) {
			printf("Replay ended\n");
			PostMessage(NULL, WM_QUIT, 0, 0);
			return;
		}

		Input::Update();

		printf("\n");
	}

	// TODO: Write Concatted save
	if (config.do_regtest) {
		RPG::fileSaveLoad->saveFile(RPG::system->frameCounter);
	}
}


void onExit() {
	// no-op
}
