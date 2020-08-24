#include <DynRPG/DynRPG.h>
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


// TODO: CLI parsing
// --replay-input NAME: Replay file NAME
// --regression-test NAME: Concat saves to NAME every frame

bool onStartup(char *pluginName) {
	DWORD old_protect;
	VirtualProtect((char*)0x400000, 0x100000, PAGE_EXECUTE_READWRITE, &old_protect);

	Hooks::Install();
	Input::Init("input.log");

	return true;
}

void onFrame (RPG::Scene scene) {
	printf("%d ", RPG::system->frameCounter);

	if (Input::IsDone()) {
		printf("Replay ended\n");
		return;
	}

	Input::Update();

	printf("\n");

	// TODO: Write Concatted save
	RPG::fileSaveLoad->saveFile(RPG::system->frameCounter);
}


void onExit() {
	// no-op
}
