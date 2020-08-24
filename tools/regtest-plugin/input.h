#pragma once

#include "utils.h"

namespace Input {
	int last_read_frame = -1;
	std::ifstream replay_file;
	std::vector<std::string> keys; // 1st elem is frame number

	struct bm_struct {
		const char* button;
		int keycode;
		bool pressed;
	};

	std::vector<bm_struct> pressed_buttons = {
		{"UP", VK_UP, false},
		{"DOWN", VK_DOWN, false},
		{"LEFT", VK_LEFT, false},
		{"RIGHT", VK_RIGHT, false},
		{"DECISION", VK_RETURN, false},
		{"CANCEL", VK_ESCAPE, false},
		{"SHIFT", VK_LSHIFT, false},
		{"N0", VK_NUMPAD0, false},
		{"N1", VK_NUMPAD1, false},
		{"N2", VK_NUMPAD2, false},
		{"N3", VK_NUMPAD3, false},
		{"N4", VK_NUMPAD4, false},
		{"N5", VK_NUMPAD5, false},
		{"N6", VK_NUMPAD6, false},
		{"N7", VK_NUMPAD7, false},
		{"N8", VK_NUMPAD8, false},
		{"N9", VK_NUMPAD9, false},
		{"PLUS", VK_ADD, false},
		{"MINUS", VK_SUBTRACT, false},
		{"MULTIPLY", VK_MULTIPLY, false},
		{"DIVIDE", VK_DIVIDE, false},
		{"PERIOD", VK_DECIMAL, false},
		{"DEBUG_MENU", VK_F9, false},
		{"DEBUG_THROUGH", VK_CONTROL, false}
	};

	void Init(const std::string& file) {
		replay_file.open(file);
	}

	bool IsPressed(int key) {
		auto it = std::find_if(pressed_buttons.begin(), pressed_buttons.end(),
			[key](bm_struct& bm) { return bm.keycode == key; });
		return it != pressed_buttons.end() && it->pressed;
	}

	bool IsDone() {
		return replay_file.eof();
	}

	void Update() {
		if (last_read_frame == -1) {
			for (auto& bm : pressed_buttons) {
				bm.pressed = false;
			}

			std::string line = read_line(replay_file);

			while (!starts_with(line, "F") && !IsDone()) {
				line = read_line(replay_file);
			}
			if (IsDone()) {
				return;
			}

			keys = tokenize(line.substr(2));
			if (!keys.empty()) {
				last_read_frame = atoi(keys[0].c_str());
			}
		}
		if (RPG::system->frameCounter == last_read_frame) {
			bool first = true;
			for (const auto& key : keys) {
				auto it = std::find_if(pressed_buttons.begin(), pressed_buttons.end(),
					[&key](bm_struct& bm) { return !strcmp(bm.button, key.c_str()); });
				if (it != pressed_buttons.end()) {
					if (!first) {
						printf(",");
					}
					first = false;
					printf("%s", it->button);
					it->pressed = true;
				}
			}
			last_read_frame = -1;
		}
	}
}
