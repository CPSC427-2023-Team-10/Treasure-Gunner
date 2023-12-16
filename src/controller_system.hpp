#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "ui_system.hpp"
#include "sound_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class ControllerSystem
{
public:
	static void set_player(Entity given_player);
	static void set_ui_system(UISystem* given_ui_system);
	static void set_sound_system(SoundSystem given_sound_system);
	static void on_key_controller(int key, int, int action, int mod, RenderSystem* renderer);
	static void on_mouse_move_controller(vec2 pos);
	static void on_mouse_button_controller(int button, int action, int mod);
};
