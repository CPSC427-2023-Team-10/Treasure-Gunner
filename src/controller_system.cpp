// Header
#include "controller_system.hpp"
#include "world_system.hpp"
#include "physics_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>

// keyboard configuration
bool w_pressed = false;
bool a_pressed = false;
bool s_pressed = false;
bool d_pressed = false;
bool e_pressed = false;
bool q_pressed = false;
bool allow_key_presses = true;
Entity player;
SoundSystem sound_system;
UISystem* ui_system;

void ControllerSystem::set_player(Entity given_player) {
	player = given_player;
}

void ControllerSystem::set_ui_system(UISystem* given_ui_system) {
	ui_system = given_ui_system;
}

void ControllerSystem::set_sound_system(SoundSystem given_sound_system) {
	sound_system = given_sound_system;
}

// On key callback
void ControllerSystem::on_key_controller(int key, int, int action, int mod, RenderSystem* renderer) {

	if (player == NULL) {
		printf("Player is null\n");
	}
	// setting speed of movement of player
	vec2 right_speed = {100.f, 0.f};
	vec2 left_speed = {-100.f, 0.f};
	vec2 up_speed = {0.f, -100.f};
	vec2 down_speed = {0.f, 100.f};
	float dodge_speed = 150.f;
	Animation& animation = registry.animations.get(player);

	// only changing salmon's speed if it has not collided
	// increasing speed in the appropriate direction if key pressed
	// decreasing speed in the appropriate direction if key released

	// also processing dodge mechanism
	if (action == GLFW_PRESS) {
		if (ui_system->in_pause_state()) {
			return;
		}
		float angle = registry.motions.get(player).angle;
		float x_speed = dodge_speed*cos(angle + M_PI/2);
		float y_speed = dodge_speed*sin(angle + M_PI/2);
		switch (key) {
			case GLFW_KEY_D:
				registry.players.get(player).right = true;
				break;
			case GLFW_KEY_A:
				registry.players.get(player).left = true;
				break;
			case GLFW_KEY_W:
				registry.players.get(player).up = true;
				break;
			case GLFW_KEY_S:
				registry.players.get(player).down = true;
				break;
			default:
				break;
		}
		if (registry.players.get(player).right ||
			registry.players.get(player).left ||
			registry.players.get(player).up ||
			registry.players.get(player).down) {
			animation.frame = 0;
			animation.row = 1;
			animation.num_frames = 4;
			animation.speed = 200.f;
		}
		
	} else if (action == GLFW_RELEASE) {
		switch (key) {
    		case GLFW_KEY_D:
				registry.players.get(player).right = false;
        		break;
    		case GLFW_KEY_A:
				registry.players.get(player).left = false;
        		break;
    		case GLFW_KEY_W:
				registry.players.get(player).up = false;
        		break;
    		case GLFW_KEY_S:
				registry.players.get(player).down = false;
        		break;
    		default:
        		break;
		}
		if (!registry.players.get(player).right &&
			!registry.players.get(player).left &&
			!registry.players.get(player).up &&
			!registry.players.get(player).down) {
			animation.frame = 0;
			animation.row = 0;
			animation.num_frames = 5;
			animation.speed = 200.f;
		}
	} 
}

void ControllerSystem::on_mouse_move_controller(vec2 mouse_position) {
	if (ui_system->in_pause_state()) {
		return;
	}
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// xpos and ypos are relative to the top-left of the window, the salmon's
	// default facing direction is (1, 0)
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (player == NULL) {
		printf("Player is null\n");
	}
	vec2 playerPosition = registry.motions.get(player).position;
	vec2 adjustedMousePosition = mouse_position * (1.0f / resolutionScale);

	registry.players.get(player).mouse_position = adjustedMousePosition;
	float angle = std::atan2(adjustedMousePosition[1]- playerPosition[1], adjustedMousePosition[0]- playerPosition[0]);
	registry.motions.get(player).angle = angle;
}

void ControllerSystem::on_mouse_button_controller(int button, int action, int mod)
{
	if (ui_system->wants_mouse_input()) {
		return;
	}
	float dodge_speed = 200.f;
	Animation& animation = registry.animations.get(player);
	
	if (player == NULL) {
		printf("Player is null\n");
	}

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT) {
		if (registry.guns.has(player)) {
			Gun& player_gun = registry.guns.get(player);
			player_gun.is_firing = true;
		}
		else if (registry.gunStatuses.has(player)) {
			GunStatus& player_gun_status = registry.gunStatuses.get(player);
			player_gun_status.is_firing = true;
		}
	} else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
		if (registry.guns.has(player)) {
			Gun& player_gun = registry.guns.get(player);
			player_gun.is_firing = false;
		}
		else if (registry.gunStatuses.has(player)) {
			GunStatus& player_gun_status = registry.gunStatuses.get(player);
			player_gun_status.is_firing = false;
		}
	}
	else if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (!registry.dodgeTimers.has(player) && !registry.deathTimers.size() > 0)
		{
			Motion& player_motion = registry.motions.get(player);
			float angle = player_motion.angle;
			float x_velocity = player_motion.velocity.x;
			float y_velocity = player_motion.velocity.y;
			if (x_velocity || y_velocity) {
				sound_system.playDodge();
			}
			int x_direction = x_velocity > 0 ? 1 : x_velocity == 0 ? 0 : -1;
			int y_direction = y_velocity > 0 ? 1 : y_velocity == 0 ? 0 : -1;
			registry.dodgeTimers.emplace(player);
			registry.dodgeTimers.get(player).initialPosition = registry.motions.get(player).position;
			if ((x_direction == 1 || x_direction == -1) && (y_direction == 1 || y_direction == -1)) {
				registry.dodgeTimers.get(player).finalPosition = { registry.motions.get(player).position.x +
					x_direction * dodge_speed * 0.7, registry.motions.get(player).position.y + y_direction * dodge_speed * 0.7 };
			}
			else {
				registry.dodgeTimers.get(player).finalPosition = { registry.motions.get(player).position.x +
					x_direction * dodge_speed, registry.motions.get(player).position.y + y_direction * dodge_speed };
			}


			// Code to gauge correct dodge animation direction relative to player assisted with Chat GPT

			// Convert angle to degrees
			float angleDegrees = angle * 180.0f / M_PI;

			// Determine the direction based on the angle
			if (angleDegrees > -45 && angleDegrees <= 45) {
				// Right
				if (registry.players.get(player).down) {
					animation.frame = 0;
					animation.row = 3;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).up) {
					animation.frame = 0;
					animation.row = 2;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).right) {
					animation.frame = 0;
					animation.row = 17;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).left) {
					animation.frame = 0;
					animation.row = 18;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
			}
			else if (angleDegrees > 45 && angleDegrees <= 135) {
				// Down
				if (registry.players.get(player).left) {
					animation.frame = 0;
					animation.row = 3;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).right) {
					animation.frame = 0;
					animation.row = 2;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).down) {
					animation.frame = 0;
					animation.row = 17;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).up) {
					animation.frame = 0;
					animation.row = 18;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
			}
			else if (angleDegrees > 135 || angleDegrees <= -135) {
				// Left
				if (registry.players.get(player).up) {
					animation.frame = 0;
					animation.row = 3;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).down) {
					animation.frame = 0;
					animation.row = 2;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).left) {
					animation.frame = 0;
					animation.row = 17;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).right) {
					animation.frame = 0;
					animation.row = 18;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
			}
			else {
				// Up
				if (registry.players.get(player).right) {
					animation.frame = 0;
					animation.row = 3;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).left) {
					animation.frame = 0;
					animation.row = 2;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).up) {
					animation.frame = 0;
					animation.row = 17;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
				else if (registry.players.get(player).down) {
					animation.frame = 0;
					animation.row = 18;
					animation.num_frames = 2;
					animation.speed = 50.f;
				}
			}
		}
	}
}
