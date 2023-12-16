
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "ui_system.hpp"
using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world_system;
	RenderSystem render_system;
	PhysicsSystem physics_system;
	AISystem ai_system;
	UISystem ui_system;

	// Initializing window
	GLFWwindow* window = world_system.create_window();
	printf("create window success!\n");
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	render_system.init(window, &ui_system);
	printf("render_system init success!\n");
	world_system.init(&render_system, &ui_system);
	printf("world_system init success!\n");
	ui_system.init(window);
	printf("ui_system init success!\n");

	// Render one frame of the UI system to avoid any flicker on startup
	ui_system.render();

	// variable timestep loop
	auto t = Clock::now();
	while (!world_system.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;

		while (ui_system.in_pause_state()) {
			glfwPollEvents();
			render_system.draw();
			world_system.check_ui();
			if (glfwWindowShouldClose(window)) {
				return EXIT_SUCCESS;
			}
		}

		t = Clock::now();
		world_system.step(elapsed_ms);
		physics_system.step(elapsed_ms);
		ai_system.step(elapsed_ms);

		world_system.handle_collisions();

		render_system.draw();
	}

	return EXIT_SUCCESS;
}
