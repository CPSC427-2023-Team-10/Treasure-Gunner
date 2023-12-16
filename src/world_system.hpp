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
#include "sound_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// Creates a window
	GLFWwindow* create_window();

	// starts the game
	void init(RenderSystem* renderer, UISystem* ui_system);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// Check for collisions
	void handle_collisions();

	// Should the game be over ?
	bool is_over()const;
	bool load_game();
	void save_game();
	void start_game();

	void check_ui();
	
private:
	// Input callback functions
	void on_key(int key, int, int action, int mod);

	void on_scroll(double x, double y);

	// restart level
	void enter_room(int from, int type);
	void addRandomMovement(Boid& boid);
	void on_mouse_button(int button, int action, int mod);

	void explode(vec2 position, float angle);

	// OpenGL window handle
	GLFWwindow* window;

	// Game state
	RenderSystem* renderer;
	UISystem* ui_system;
	float current_speed;
	float next_turtle_spawn;
	float next_fish_spawn;
	Entity player;

	// Reset the world state to its initial state
	void restart_game();



	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
