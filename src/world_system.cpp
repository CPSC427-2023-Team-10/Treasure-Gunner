// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "controller_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <random>
#include <algorithm>

#include "physics_system.hpp"
#include <room_generation.hpp>

// GLFW Image Loader
#include "../ext/project_path.hpp"
#include <GLFW/glfw3.h>

// Serialization
#include <fstream>
#include <string>
#include <iostream>
#include <glm/trigonometric.hpp>

// Game configuration
const size_t BOSS_DELAY_MS = 9000 * 3;
const float INITIAL_DODGE_PARTICLE_POSITION = -100;
SoundSystem soundSystem;

bool tutorial_ongoing = true;
bool shop_room = false;

// Create the fish world
WorldSystem::WorldSystem()
	: next_turtle_spawn(0.f)
	, next_fish_spawn(0.f) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {
	// Destroy music components
	Mix_CloseAudio();

	// Destroy all created components
	registry.clear_all_components();
	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	// Resolution selection
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);

	// ChatGPT Resolution selection
	int width = (16 * videoMode->height) / 9;

	// Check if the calculated width fits within the monitor width
	if (width <= videoMode->height) {
		resolutionHeight = videoMode->height;
		resolutionWidth = width;
		//return { width, monitorHeight };
	} else {
		// If the calculated width exceeds the monitor width, adjust the height
		resolutionHeight = (9 * videoMode->width) / 16;
		resolutionWidth = videoMode->width;
		// int height = (9 * monitorWidth) / 16;
		// return { monitorWidth, height };
	}

#if DEBUG_OPENGL
	resolutionWidth *= 0.85f;
	resolutionHeight *= 0.85f;

	resolutionScale = (float)resolutionWidth / (float)window_width_px;
	// End resolution selection

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(resolutionWidth, resolutionHeight, "Treasure Gunner", nullptr, nullptr);
#else
	// ChatGPT Max resolution mode selection

	int maxResolutionWidth = 0;
	int maxResolutionHeight = 0;
	//float maxResolutionAspectRatio = 0.0f;

	int count;
	const GLFWvidmode* modes = glfwGetVideoModes(primaryMonitor, &count);

	for (int i = 0; i < count; i++) {
		int width = modes[i].width;
		int height = modes[i].height;
		float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

		// Check if the resolution is 16:9 and higher than the current maximum
		if (aspectRatio == 16.0f / 9.0f && width > maxResolutionWidth) {
			maxResolutionWidth = width;
			maxResolutionHeight = height;
			//maxResolutionAspectRatio = aspectRatio;
		}
	}

	resolutionWidth = maxResolutionWidth;
	resolutionHeight = maxResolutionHeight;

	if (maxResolutionWidth > 0) {
		//std::cout << "Selected Resolution: " << maxResolutionWidth << "x" << maxResolutionHeight << " (Aspect Ratio: " << maxResolutionAspectRatio << ")\n";
		printf("Chose resolution %d, %d\n", resolutionWidth, resolutionHeight);
	}
	else {
		//std::cout << "No suitable resolution found.\n";
		printf("No resolution found\n");
	}

	resolutionScale = (float)resolutionWidth / (float)window_width_px;

	int posX = (videoMode->width - resolutionWidth) / 2;
	int posY = (videoMode->height - resolutionHeight) / 2;
	window = glfwCreateWindow(resolutionWidth, resolutionHeight, "Treasure Gunner", nullptr, nullptr);
	glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), posX, posY, resolutionWidth, resolutionHeight, GLFW_DONT_CARE);
#endif

	//window = glfwCreateWindow(window_width_px / 2, window_height_px / 2, "Treasure Gunner", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

#if DEBUG_OPENGL
	glfwSetWindowPos(window, 75, 100);
#endif

	// Load the image
	std::string imgPath = std::string(PROJECT_SOURCE_DIR) + "/data/temp_icon.png";
	// Load your favicon image
	GLFWimage images[1];
	images[0].pixels = stbi_load(imgPath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
	glfwSetWindowIcon(window, 1, images);
	stbi_image_free(images[0].pixels);

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((ControllerSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move_controller({ _0, _1 }); };
	auto mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(_0, _1, _2); };
	auto scroll_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_scroll(_0, _1); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, mouse_button_redirect);
	glfwSetScrollCallback(window, scroll_redirect);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	soundSystem.loadSFX();
	ControllerSystem::set_sound_system(soundSystem);
	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg, UISystem* ui_system) {
	this->renderer = renderer_arg;
	this->ui_system = ui_system;
	ControllerSystem::set_ui_system(ui_system);
	// Set all states to default
	start_game();
	soundSystem.playBGM();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& motion_container = registry.motions;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (int i = (int)motion_container.components.size()-1; i>=0; --i) {
	    Motion& motion = motion_container.components[i];
		if (motion.position.x + abs(motion.scale.x) < 0.f) {
			if(!registry.players.has(motion_container.entities[i])) // don't remove the player
				registry.remove_all_components_of(motion_container.entities[i]);
		}
	}

	/*for (Entity entity : registry.damageTimers.entities) {
		if (registry.players.has(entity)) {
			DamageTimer& timer = registry.damageTimers.get(entity);
			timer.timer_ms -= elapsed_ms_since_last_update;
			if (timer.timer_ms < 0) {
				vec3& color = registry.colors.get(player);
				color = { 1, 1, 1 };
				registry.damageTimers.remove(entity);
			}
		}
	}*/
	// Done more efficiently with
	for (int i = 0; i < registry.players.components.size(); i++)
	{
		Entity entity = registry.players.entities[i];
		if (registry.damageTimers.has(entity))
		{
			DamageTimer& timer = registry.damageTimers.get(entity);
			timer.timer_ms -= elapsed_ms_since_last_update;
			if (timer.timer_ms < 0) {
				vec3& color = registry.colors.get(player);
				color = { 1, 1, 1 };
				registry.damageTimers.remove(entity);
			}
		}
	}

	for (Entity entity : registry.enemies.entities) {
		if (registry.enemies.get(entity).id == ENEMY_ID::BOMBER) {
			Motion& motion = registry.motions.get(entity);
			float xFactor = (registry.motions.get(player).position.x > motion.position.x) ? 100 : -100;
			float yFactor = (registry.motions.get(player).position.y > motion.position.y) ? 100 : -100;
			motion.velocity = vec2(xFactor, yFactor);
		}
	}

	int hasFlamethrower = 0;
	for (Enemy enemy : registry.enemies.components) {
		if (enemy.id == ENEMY_ID::FLAMETHROWER) {
			hasFlamethrower = 1;
		}
	}
	if (hasFlamethrower && !soundSystem.playingFire) {
		soundSystem.playFire();
	}
	else if (!hasFlamethrower && soundSystem.playingFire){
		soundSystem.stopFire();
	}

	ScreenState& screen = registry.screenStates.components[0];
	float min_timer_ms = 2400.f;
	//for (Entity entity : registry.transitionTimers.entities) {
	// More efficient without the get
	for (int i = 0, count = registry.transitionTimers.size(); i < count; i++) {
		//TransitionTimer& timer = registry.transitionTimers.get(entity);
		TransitionTimer& timer = registry.transitionTimers.components[i];
		Entity entity = registry.transitionTimers.entities[i];

		// progress timer
		timer.timer_ms -= elapsed_ms_since_last_update;
		
		if (timer.timer_ms < min_timer_ms) {
			min_timer_ms = timer.timer_ms;
		}

		if (timer.timer_ms < 0) {
			for (auto& entity : registry.walls.entities)
			{
				if (registry.renderRequests.has(entity)) {
					if(registry.renderRequests.get(entity).used_texture == TEXTURE_ASSET_ID::OPEN_DOOR)
						registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::CLOSED_DOOR;
				}

			}
			soundSystem.playCloseDoor();

			registry.transitionTimers.remove(entity);
			registry.collisionMeshes.get(player).is_solid = true;
			if (registry.enemies.components.size() == 0) {
				soundSystem.playOpenDoor();
				for (auto& entity : registry.doors.entities)
				{
					openDoor(entity);

				}
			}
			screen.screen_darken_factor = 0.f;
			return true;
		}

		// fade in and out
		if (timer.timer_ms < 300) {
			screen.screen_darken_factor = timer.timer_ms / 300.f;
		}
		else if (timer.timer_ms > 300) {
			screen.screen_darken_factor = (600 - timer.timer_ms) / 300.f;
		}

		if (timer.timer_ms < 300 && !timer.entering){
			soundSystem.playSteps();
			enter_room(timer.from, timer.type);
			
			createRubble(renderer, (timer.from + 2) % 4);
			
			timer.entering = true;

			return true;
		}
	}

	//for (Entity entity : registry.animations.entities) {
	// More efficient without the get
	for (int i = 0, count = registry.animations.size(); i < count; i++) {
		Animation& animation = registry.animations.components[i];
		Entity entity = registry.animations.entities[i];
		//Animation& animation = registry.animations.get(entity);

		if (registry.damageTimers.has(entity)) {
			DamageTimer& damage = registry.damageTimers.get(entity);
			if (damage.timer_ms > 0) {
				damage.timer_ms -= elapsed_ms_since_last_update;
			}
			else {
				if (registry.enemies.has(entity)) {
					Enemy& enemy = registry.enemies.get(entity);
					if (enemy.id == ENEMY_ID::NORMAL) {
						animation.row = 4;
					}
					else if (enemy.id == ENEMY_ID::ELITE) {
						animation.row = 15;
					}
					else if (enemy.id == ENEMY_ID::DUMMY) {
						animation.row = 19;
					}
					else if (enemy.id == ENEMY_ID::ZAPPER) {
						animation.row = 34;
					}
					else if (enemy.id == ENEMY_ID::FLAMETHROWER) {
						animation.row = 36;
					}
					else if (enemy.id == ENEMY_ID::BOMBER) {
						animation.row = 32;
					}
				}
				else if (registry.bosses.has(entity)) {

					if (registry.bosses.get(entity).id == BOSS_ID::IDLE) {
						animation.row = 0;
					}
					else {
						animation.row = 2;
					}
				}
				damage.timer_ms = 100.f;
				registry.damageTimers.remove(entity);
			}
		}
		animation.timer += elapsed_ms_since_last_update;
		if (animation.timer > animation.speed) {
			animation.frame = (animation.frame + 1) % animation.num_frames;
			animation.timer = 0;
		}
	}

	// for (Entity entity : registry.bosses.entities) {
	for (int i = 0, count = registry.bosses.size(); i < count; i++) {
		Boss& boss = registry.bosses.components[i];
		Entity entity = registry.bosses.entities[i];
		// Boss& boss = registry.bosses.get(entity);

		boss.cooldown -= elapsed_ms_since_last_update * current_speed;
		if (boss.cooldown < 0.f) {
			if (!registry.roarTimers.has(entity) && !registry.summonTimers.has(entity)) {
				soundSystem.playBossRoar();
				boss.id = BOSS_ID::ACTION;

				Animation& animation = registry.animations.get(entity);
				if (registry.damageTimers.has(entity)) {
					animation.row = 3;
				}
				else {
					animation.row = 2;
				}

				if (rand() % 100 > 60 && registry.enemies.components.size() == 0) {
					createMegaSwarm(renderer, player, vec2(500, 500), vec2(700, 700), 3, 3, true);
					registry.guns.get(entity).is_firing = false;
					registry.summonTimers.emplace(entity);
				}
				else {
					if (registry.guns.has(entity)) {
						registry.guns.remove(entity);
					}

					equipGun(entity, GUN_ID::OMNI_SHOT).is_firing = true;
					registry.roarTimers.emplace(entity);
				}
			}
			// Reset timer
			boss.cooldown = (BOSS_DELAY_MS / 2) + uniform_dist(rng) * (BOSS_DELAY_MS / 2);
		}
	}

	// for (Entity entity : registry.guns.entities) {
	for (int i = 0, count = registry.guns.size(); i < count; i++) {
		Gun& gun = registry.guns.components[i];
		Entity entity = registry.guns.entities[i];
		// Gun& gun = registry.guns.get(entity);
		
		bool shot_by_player = registry.players.has(entity);
		bool shot_by_boss = registry.bosses.has(entity);
		gun.timer_ms = std::max(gun.timer_ms - elapsed_ms_since_last_update, 0.f);
		float speed = 0;
		float dist = 0;
		float damage = 0;
		if (gun.is_firing && gun.timer_ms <= 0) {
			Motion motion = registry.motions.get(entity);
			vec2 offset = { 0,0 };
			if (shot_by_player) {
				offset = {80 * cos(motion.angle), 80 * sin(motion.angle)};
			}
			if (gun.gun_id == GUN_ID::STRAIGHT_SHOT) {
				if (shot_by_player) {
					speed = 600;
					dist = registry.players.get(player).range;
					damage = 10;
					createBullet(motion.position + offset, motion.angle, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				}
				else {
					speed = 400;
					dist = 800;
					damage = 2;
					createBullet(motion.position + offset, motion.angle, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				}
			}
			else if (gun.gun_id == GUN_ID::SPLIT_SHOT) {
				float deg_to_rad = M_PI / 180.0;
				speed = 500;
				dist = registry.players.get(player).range;
				damage = 5;
				createBullet(motion.position + offset, motion.angle - 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position + offset, motion.angle, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position + offset, motion.angle + 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
			}
			else if (gun.gun_id == GUN_ID::BOUNCING_SHOT) {
				speed = 600;
				dist = registry.players.get(player).range;
				damage = 8;
				createBullet(motion.position + offset, motion.angle, speed, true, dist, shot_by_player, false, damage, 1, gun.gun_id);
			}
			else if (gun.gun_id == GUN_ID::SPLINE_SHOT) {
				speed = 600;
				dist = 600;
				damage = 10;
				createSplineBullet(motion.position + offset, registry.motions.get(player).position, motion.angle, speed, true, dist, shot_by_player, shot_by_boss, damage);
			}
			else if (gun.gun_id == GUN_ID::SPLIT_SPLINE_SHOT) {
				float deg_to_rad = M_PI / 180.0;
				speed = 800;
				dist = 800;
				damage = 5;
				createSplineBullet(motion.position, registry.motions.get(player).position, motion.angle - 30 * deg_to_rad, speed, false, dist, shot_by_player, shot_by_boss, damage);
				createSplineBullet(motion.position, registry.motions.get(player).position, motion.angle - 15 * deg_to_rad, speed, false, dist, shot_by_player, shot_by_boss, damage);
				createSplineBullet(motion.position, registry.motions.get(player).position, motion.angle, speed, false, dist, shot_by_player, shot_by_boss, damage);
				createSplineBullet(motion.position, registry.motions.get(player).position, motion.angle + 15 * deg_to_rad, speed, false, dist, shot_by_player, shot_by_boss, damage);
				createSplineBullet(motion.position, registry.motions.get(player).position, motion.angle + 30 * deg_to_rad, speed, false, dist, shot_by_player, shot_by_boss, damage);
			}
			else if (gun.gun_id == GUN_ID::RAPID_SHOT) {
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> angle_distribution(-7.5, 7.5);
				float angle_offset = angle_distribution(gen);
				float deg_to_rad = M_PI / 180.0;
				speed = 600;
				dist = registry.players.get(player).range;
				damage = 4;
				createBullet(motion.position + offset, motion.angle + angle_offset * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
			}
			else if (gun.gun_id == GUN_ID::LONG_SHOT) {
				speed = 2000;
				dist =registry.players.get(player).range;
				damage = 25;
				createBullet(motion.position + offset, motion.angle, speed, false, dist + 500, shot_by_player, true, damage, 2, gun.gun_id);
			}
			else if (gun.gun_id == GUN_ID::OMNI_SHOT) {
				float deg_to_rad = M_PI / 180.0;
				float other_angle = motion.angle + M_PI;
				float side_angle1 = motion.angle + M_PI / 2;
				float side_angle2 = motion.angle - M_PI / 2;

				speed = 500;
				dist = 1000;
				damage = 7;
				createBullet(motion.position, motion.angle - 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, motion.angle - 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, motion.angle, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, motion.angle + 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, motion.angle + 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);

				createBullet(motion.position, other_angle - 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, other_angle - 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, other_angle, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, other_angle + 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, other_angle + 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);

				createBullet(motion.position, side_angle1 - 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle1 - 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle1, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle1 + 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle1 + 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);

				createBullet(motion.position, side_angle2 - 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle2 - 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle2, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle2 + 15 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, side_angle2 + 30 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
			}
			else if (gun.gun_id == GUN_ID::ZAP_SHOT) {
				float deg_to_rad = M_PI / 180.0;
				float other_angle = motion.angle + M_PI;
				speed = 700;
				dist = 800;
				damage = 10;
				createBullet(motion.position, motion.angle - 45 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, motion.angle + 45 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, other_angle - 45 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
				createBullet(motion.position, other_angle + 45 * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
			}
			else if (gun.gun_id == GUN_ID::FIRE_SHOT) {
				std::random_device rd;
				std::mt19937 gen(rd());
				std::uniform_real_distribution<float> angle_distribution(-7.5, 7.5);
				float angle_offset = angle_distribution(gen);
				float deg_to_rad = M_PI / 180.0;
				speed = 500;
				dist = 400;
				damage = 2;
				createBullet(motion.position + offset, motion.angle + angle_offset * deg_to_rad, speed, false, dist, shot_by_player, false, damage, 1, gun.gun_id);
			}

			if (shot_by_player) {
				soundSystem.playPlayerShoot();
			}
			else if (shot_by_boss) {
				soundSystem.playBossShoot();
			}
			else if (gun.gun_id == GUN_ID::FIRE_SHOT){
				
			}
			else {
				soundSystem.playEnemyShoot();
			}
			
			gun.timer_ms = gun.cooldown_ms;
		}
	}

	for (int i = 0; i < registry.gunStatuses.components.size(); i++) {
		GunStatus& gunStatus = registry.gunStatuses.components[i];
		gunStatus.timer_ms = std::max(gunStatus.timer_ms - elapsed_ms_since_last_update, 0.f);
	}

	for (int i = 0; i < registry.deathTimers.components.size(); i++) {
		DeathTimer& timer = registry.deathTimers.components[i];
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < 0) {
			registry.remove_all_components_of(registry.deathTimers.entities[i]);
			restart_game();
		}
	}

	// not changing this, how does it even work with the remove call inside the iterator?
	for (Entity entity : registry.notEnoughCoinsTimer.entities) {
		NotEnoughCoinsTimer& timer = registry.notEnoughCoinsTimer.get(entity);
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < 0) {
			registry.remove_all_components_of(entity);
		}
	}

	// for (Entity entity : registry.introTimers.entities) {
	for (int i = 0, count = registry.introTimers.size(); i < count; i++) {
		IntroTimer& timer = registry.introTimers.components[i];
		Entity entity = registry.introTimers.entities[i];
		// IntroTimer& timer = registry.introTimers.get(entity);
		
		timer.timer_ms -= elapsed_ms_since_last_update;
		soundSystem.playBossRoar();
		if (timer.timer_ms < 0) {
			Animation& animation = registry.animations.get(entity);
			animation.frame = 0;
			animation.row = 0;
			animation.num_frames = 11;
			animation.speed = 120.f;

			registry.collisionMeshes.get(entity).is_static = false;

			registry.guns.get(entity).is_firing = true;

			auto flock = Entity();
			registry.flocks.emplace(flock);
			auto& flockComponent = registry.flocks.get(flock);
			flockComponent.target = player;
			flockComponent.boids.push_back(entity);
			createSwarm(renderer, player, vec2(500, 500), vec2(700, 700), 3, 1, true);
			registry.introTimers.remove(entity);
		}
	}

	// for (Entity entity : registry.roarTimers.entities) {
	for (int i = 0, count = registry.roarTimers.size(); i < count; i++) {
		RoarTimer& timer = registry.roarTimers.components[i];
		Entity entity = registry.roarTimers.entities[i];
		// RoarTimer& timer = registry.roarTimers.get(entity);
		
		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < 0) {
			Animation& animation = registry.animations.get(entity);
			if (registry.damageTimers.has(entity)) {
				animation.row = 1;
			}
			else {
				animation.row = 0;
			}

			if (registry.guns.has(entity)) {
				registry.guns.remove(entity);
			}

			equipGun(entity, GUN_ID::SPLIT_SPLINE_SHOT).is_firing = true;
			registry.bosses.get(entity).id = BOSS_ID::IDLE;
			registry.roarTimers.remove(entity);
		}
	}

	// Does removals so we need the iterator
	for (Entity entity : registry.summonTimers.entities) {
		SummonTimer& timer = registry.summonTimers.get(entity);

		timer.timer_ms -= elapsed_ms_since_last_update;
		if (timer.timer_ms < 0) {
			Animation& animation = registry.animations.get(entity);

			if (registry.bosses.has(entity)) {
				if (registry.damageTimers.has(entity)) {
					animation.row = 1;
				}
				else {
					animation.row = 0;
				}
				registry.bosses.get(entity).id = BOSS_ID::IDLE;
			}
			else {
				ENEMY_ID id = registry.enemies.get(entity).id;
				if (id == ENEMY_ID::NORMAL) {
					animation.row = 4;
					animation.num_frames = 4;
					animation.speed = 100.f;
				}
				else if (id == ENEMY_ID::ELITE) {
					animation.row = 15;
					animation.num_frames = 4;
					animation.speed = 100.f;
				}
				else if (id == ENEMY_ID::ZAPPER) {
					animation.row = 34;
					animation.num_frames = 6;
					animation.speed = 100.f;
				}
				else if (id == ENEMY_ID::FLAMETHROWER) {
					animation.row = 36;
					animation.num_frames = 2;
					animation.speed = 200.f;
				}
				else if (id == ENEMY_ID::BOMBER) {
					animation.row = 32;
					animation.num_frames = 4;
					animation.speed = 100.f;
				}
			}
			
			if (registry.guns.has(entity))
			{
				registry.guns.get(entity).is_firing = true;
			}
			registry.summonTimers.remove(entity);
		}
	}

	// for (Entity entity : registry.dialogues.entities) {
	for (int i = 0, count = registry.dialogues.size(); i < count; i++) {
		Dialogue& dialogue = registry.dialogues.components[i];
		Entity entity = registry.dialogues.entities[i];
		// Dialogue& dialogue = registry.dialogues.get(entity);

		if (dialogue.current_page >= dialogue.dialogue_pages.size()) {
			registry.remove_all_components_of(entity);
			continue;
		}
		dialogue.timer_ms -= elapsed_ms_since_last_update;
		if (dialogue.timer_ms < 0) {
			dialogue.timer_ms = dialogue.timer_ms_per_character;
			dialogue.current_character = std::min((int)dialogue.dialogue_pages[dialogue.current_page].length(), dialogue.current_character + 1);
		}
	}

	// There are removals within the loop so we need the iterator
	for (Entity particle : registry.particleSystems.entities) {
		ParticleSystem& particleSystem = registry.particleSystems.get(particle);

		if (particleSystem.lifetime < 0) {
			if (particleSystem.texture == TEXTURE_ASSET_ID::PRIZE && particleSystem.has_spawned) {
				ui_system->exit_state(UI_STATE_ID::PLAYING);
				ui_system->enter_state(UI_STATE_ID::SCORE);
			}
			particleSystem.has_spawned = false;
			registry.remove_all_components_of(particle);
		} else {
			if (particleSystem.texture == TEXTURE_ASSET_ID::SMOKE && registry.dodgeTimers.has(player)) {
				vec2 relativePosition = registry.dodgeTimers.get(player).initialPosition - registry.dodgeTimers.get(player).finalPosition;
				vec2 playerMovement = registry.dodgeTimers.get(player).initialPosition - registry.motions.get(player).position;
				particleSystem.angle = (atan2(relativePosition.x, relativePosition.y)) * 180.0f / M_PI;
				int foundIndex = -1;
				for (int i = 0; i < particleSystem.num_particles - 1; i++) {
					if (particleSystem.particles[i].lifetime <= 0) {
						particleSystem.particles[i].position.x = INITIAL_DODGE_PARTICLE_POSITION;
						particleSystem.particles[i].position.y = INITIAL_DODGE_PARTICLE_POSITION;
					}
					else if (particleSystem.particles[i].position.x == INITIAL_DODGE_PARTICLE_POSITION && particleSystem.particles[i].position.y == INITIAL_DODGE_PARTICLE_POSITION) {
						foundIndex = i;
						particleSystem.particles[i].lifetime = 200.f;
						break;
					}
					else {
						particleSystem.particles[i].lifetime -= elapsed_ms_since_last_update;
					}
				}


				if (foundIndex != -1 && playerMovement != vec2(0, 0) && particleSystem.particles[foundIndex].lifetime >= 0) {
					float incX = relativePosition.x / particleSystem.num_particles;
					float incY = relativePosition.y / particleSystem.num_particles;

					if (abs(incX * foundIndex) <= abs(playerMovement.x) && abs(incY * foundIndex) <= abs(playerMovement.y)) {
						particleSystem.particles[foundIndex].position.x = registry.motions.get(player).position.x;
						particleSystem.particles[foundIndex].position.y = registry.motions.get(player).position.y;
					}
				}

				particleSystem.has_spawned = true;

			}
			// partially used gpt
			if (particleSystem.texture == TEXTURE_ASSET_ID::DEATH || particleSystem.texture == TEXTURE_ASSET_ID::DEATH_GREEN || particleSystem.texture == TEXTURE_ASSET_ID::DEATH_BLUE || particleSystem.texture == TEXTURE_ASSET_ID::DEATH_BOSS || particleSystem.texture == TEXTURE_ASSET_ID::DEATH_MINE || particleSystem.texture == TEXTURE_ASSET_ID::DEATH_GARBAGE || particleSystem.texture == TEXTURE_ASSET_ID::DEATH_BOMB) {
				if (!particleSystem.has_spawned) {
					for (int i = 0; i < particleSystem.num_particles; i++) {
						particleSystem.particles[i].position.y -= i * 10.f;
						particleSystem.particles[i].velocity.x = 0.0f;
						particleSystem.particles[i].velocity.y = -100.0f;
						particleSystem.particles[i].angle = i * 50.0f;
					}
					particleSystem.has_spawned = true;
				}

				for (int i = 0; i < particleSystem.num_particles; i++) {
					particleSystem.particles[i].angle += elapsed_ms_since_last_update * 0.4f;
					float wave = sin(particleSystem.particles[i].angle * (M_PI / 180.0f)) * 80.0f;
					particleSystem.particles[i].velocity.x = wave;

					particleSystem.particles[i].position += particleSystem.particles[i].velocity * (elapsed_ms_since_last_update / 1000.0f);
				}
			}

			if(particleSystem.texture == TEXTURE_ASSET_ID::HEART_PARTICLE)  {
				if (!particleSystem.has_spawned) {
					for (int i = 0; i < particleSystem.num_particles; i++) {
						float angleVariation = (float)rand() / (float)RAND_MAX * M_PI / 4 - M_PI / 8;
						float angle = M_PI / 2 + angleVariation; 
						float speed = 2.f * (float)rand() / (float)RAND_MAX * 100 + 500;
						vec2 velocity = {5* speed * cos(angle) * 0.1f, speed * sin(angle)};
						particleSystem.particles[i].velocity = velocity;
					};
					particleSystem.has_spawned = true;
				}
				for (size_t i = 0; i < particleSystem.num_particles; i++) {
					particleSystem.particles[i].velocity.y += 980.0f * elapsed_ms_since_last_update / 2000.0f; 
					particleSystem.particles[i].velocity.x *= 0.99f;  // Apply drag
					particleSystem.particles[i].position += particleSystem.particles[i].velocity * elapsed_ms_since_last_update / 1000.0f;

					// Simple bounce on ground collision, assuming ground is at y = 0
					vec2 initial_position = particleSystem.init_position;
					if (particleSystem.particles[i].position.y >= initial_position.y) {
						particleSystem.particles[i].position.y = initial_position.y;
						particleSystem.particles[i].velocity.y = -particleSystem.particles[i].velocity.y * 0.7;
					}

				}

			}

			if(particleSystem.texture == TEXTURE_ASSET_ID::COIN)  {
				vec2 player_position = registry.motions.get(player).position;
				if (!particleSystem.has_spawned) {
					for (int i = 0; i < particleSystem.num_particles; i++) {
						float random_offset_x = rand() % 50 + 1;
						float random_offset_y = rand() % 50 + 1;

						particleSystem.particles[i].position += vec2(random_offset_x, random_offset_y);
						vec2 direction_to_player = normalize(player_position - particleSystem.particles[i].position);
						float speed = 500.0f;
						particleSystem.particles[i].velocity = direction_to_player * speed;
					}
					particleSystem.has_spawned = true;
				}

				for (size_t i = 0; i < particleSystem.num_particles; i++) {
					if (particleSystem.particles[i].lifetime > 0) {
						vec2 direction_to_player = normalize(player_position - particleSystem.particles[i].position);
						float distance_to_player = length(player_position - particleSystem.particles[i].position);

						float attraction_strength = 10000.0f;
						float max_velocity = 500.0f;
						attraction_strength *= (1.0f - distance_to_player / 10000.0f);
						particleSystem.particles[i].velocity += direction_to_player * attraction_strength * elapsed_ms_since_last_update / 1000.0f;

						if (length(particleSystem.particles[i].velocity) > max_velocity) {
							particleSystem.particles[i].velocity = normalize(particleSystem.particles[i].velocity) * max_velocity;
						}

						particleSystem.particles[i].velocity *= 0.99f;

						particleSystem.particles[i].position += particleSystem.particles[i].velocity * elapsed_ms_since_last_update / 1000.0f;
						if (distance_to_player < 50.0f) {
							Player& player = registry.players.components[0];
							player.coins += 5;
							particleSystem.particles[i].lifetime = 0.0f;
						}
					}
					else {
						particleSystem.particles[i].position.x = INITIAL_DODGE_PARTICLE_POSITION;
						particleSystem.particles[i].position.y = INITIAL_DODGE_PARTICLE_POSITION;
					}
				}
			}

			if (particleSystem.texture == TEXTURE_ASSET_ID::PRIZE) {
				if (!particleSystem.has_spawned) {
					for (int i = 0; i < particleSystem.num_particles; i++) {
						vec2 initial_position = (i % 2 == 0) ? vec2(0, window_height_px) : vec2(window_width_px, window_height_px);

						float angleVariation = (float)rand() / (float)RAND_MAX * M_PI / 8 - M_PI / 16;
						float angle = ((i % 2 == 0) ? -M_PI / 3 : -5 * M_PI / 7) + angleVariation;
						float speed = 1000 + (float)rand() / (float)RAND_MAX * 500;
						vec2 velocity = { speed * cos(angle), speed * sin(angle) };

						particleSystem.particles[i].position = initial_position;
						particleSystem.particles[i].velocity = velocity;
					};
					particleSystem.has_spawned = true;
				}
				for (size_t i = 0; i < particleSystem.num_particles; i++) {
					particleSystem.particles[i].velocity.y += 980.0f * elapsed_ms_since_last_update / 1000.0f;
					particleSystem.particles[i].position += particleSystem.particles[i].velocity * elapsed_ms_since_last_update / 1000.0f;
					if (particleSystem.lifetime < 2000) {
						screen.screen_darken_factor += elapsed_ms_since_last_update / 300000.0f;
					}
				}
			}
			
			if (particleSystem.texture == TEXTURE_ASSET_ID::SPARK_BASE) {
				if (!particleSystem.has_spawned) {
					vec2 particlePosition = particleSystem.position;
					vec2 velocity = particleSystem.velocity;
					// ChatGPT velocity randomization
					float originalAngle = glm::atan(velocity.y, velocity.x);
					particleSystem.angle = glm::degrees(originalAngle);

					for (int i = 0; i < particleSystem.num_particles; i++) {
						// Generate a random angle between +30 and -30 degrees in radians
						float randomAngle = rand() % 600; // start at 600 to capture some decimal places
						randomAngle = (randomAngle - 300.0f) / 10.0f; // Divide as a float afterwards

						// Add the random angle to the original angle
						float finalAngle = originalAngle + glm::radians(randomAngle);
						particleSystem.particles[i].velocity = vec2(glm::cos(finalAngle), glm::sin(finalAngle)) * 400.0f;
						particleSystem.particles[i].angle = finalAngle;
					}
					particleSystem.has_spawned = true;
				}

				for (size_t i = 0; i < particleSystem.num_particles; i++) {
					particleSystem.particles[i].velocity.x *= 0.99f;
					particleSystem.particles[i].velocity.y *= 0.99f;
					particleSystem.particles[i].position += particleSystem.particles[i].velocity * (elapsed_ms_since_last_update / 1000.0f);
				}
			}
			if (particleSystem.texture == TEXTURE_ASSET_ID::BULLET_DEAD_BASE ||
				particleSystem.texture == TEXTURE_ASSET_ID::ENEMY_BULLET_DEAD ||
				particleSystem.texture == TEXTURE_ASSET_ID::SPLINE_BULLET_DEAD ||
				particleSystem.texture == TEXTURE_ASSET_ID::BULLET_DEAD_ZAPPER)
			{
				// This is the same behaviour as sparks but it's creation conditions are different. Leaving both
				if (!particleSystem.has_spawned) {
					vec2 particlePosition = particleSystem.position;
					vec2 velocity = particleSystem.velocity;
					// ChatGPT velocity randomization
					float originalAngle = glm::atan(velocity.y, velocity.x);
					particleSystem.angle = glm::degrees(originalAngle);
					for (int i = 0; i < particleSystem.num_particles; i++) {
						// Generate a random angle between +30 and -30 degrees in radians
						float randomAngle = rand() % 300; // start at 600 to capture some decimal places
						randomAngle = (randomAngle - 150.0f) / 10.0f; // Divide as a float afterwards

						// Add the random angle to the original angle
						float finalAngle = originalAngle + glm::radians(randomAngle);
						particleSystem.particles[i].velocity = vec2(glm::cos(finalAngle), glm::sin(finalAngle)) * glm::length(velocity);
						particleSystem.particles[i].angle = finalAngle;
					}
					particleSystem.has_spawned = true;
				}

				for (size_t i = 0; i < particleSystem.num_particles; i++) {
					particleSystem.particles[i].velocity.x *= 0.99f;
					particleSystem.particles[i].velocity.y *= 0.99f;
					particleSystem.particles[i].position += particleSystem.particles[i].velocity * (elapsed_ms_since_last_update / 1000.0f);
				}
			}
			particleSystem.lifetime -= elapsed_ms_since_last_update;
		}

	};
	return true;
}

void WorldSystem::check_ui() {
	// Check for UI events
	if (ui_system->pressed_button == UI_BUTTON_ID::LOAD_GAME) {
		// load game
		if (load_game()) {
			ui_system->exit_state(UI_STATE_ID::MENU);
			ui_system->exit_state(UI_STATE_ID::SCORE);
			ui_system->enter_state(UI_STATE_ID::PLAYING);
		}
		else {
			Alert alert = {"No save file found!"};
			registry.alerts.insert(Entity(), alert);
		}
		
	}
	else  if (ui_system->pressed_button == UI_BUTTON_ID::SAVE_GAME) {
		// save game
		if (tutorial_ongoing) {
			Alert alert = { "You cannot save in the tutorial!" };
			registry.alerts.insert(Entity(), alert);
		} else if (!shop_room || registry.transitionTimers.size() > 0) {
			// cant save game
			Alert alert = { "You can only save in a Shop Room!" };
			registry.alerts.insert(Entity(), alert);
		}
		else {
			save_game();
			Alert alert = { "Game saved successfully!" };
			registry.alerts.insert(Entity(), alert);
		}
	}
	else if (ui_system->pressed_button == UI_BUTTON_ID::NEW_GAME) {
		// restart game
		start_game();
		ui_system->exit_state(UI_STATE_ID::MENU);
		ui_system->exit_state(UI_STATE_ID::SCORE);
		ui_system->enter_state(UI_STATE_ID::PLAYING);
	}
	else if (ui_system->pressed_button == UI_BUTTON_ID::MAIN_MENU) {
		// restart music
		soundSystem.playBGM();
	}
}

// Reset the world state to its initial state
void WorldSystem::start_game() {
	tutorial_ongoing = true;
	shop_room = false;
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
	    registry.remove_all_components_of(registry.motions.entities.back());
	registry.dialogues.clear();
	registry.alerts.clear();
	registry.messages.clear();
	registry.deathTimers.clear();
	// Debugging for memory/component leaks
	registry.list_all_components();
	ScreenState& screen = registry.screenStates.components[0];
	screen.screen_darken_factor = 0.f;

	// Create a new salmon
	player = createPlayer(renderer, { 200, 200 });
	ControllerSystem::set_player(player);
	registry.colors.insert(player, { 1, 1, 1 });

	soundSystem.playBGM();
	// Create floor
	createTutorialRoomOne(renderer,player);

	for (Entity particle : registry.particleSystems.entities) {
		ParticleSystem& particleSystem = registry.particleSystems.get(particle);
		registry.remove_all_components_of(particle);
	}
}

void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	tutorial_ongoing = true;
	shop_room = false;
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// All that have a motion, we could also iterate over all fish, turtles, ... but that would be more cumbersome
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	registry.dialogues.clear();
	registry.alerts.clear();
	registry.messages.clear();
	registry.deathTimers.clear();
	// Debugging for memory/component leaks
	registry.list_all_components();
	

	// Create a player
	player = createPlayer(renderer, { 200, 200 });
	ControllerSystem::set_player(player);
	registry.colors.insert(player, { 1, 1, 1 });

	// Create floor
	createTutorialRoomFour(renderer, player);
	
	for (auto& entity : registry.doors.entities)
		{
                	openDoor(entity);
		}

	createRubble(renderer, 3);
	soundSystem.playBGM();

	for (auto& entity : registry.walls.entities)
	{
		if (registry.renderRequests.has(entity)) {
			if (registry.renderRequests.get(entity).used_texture == TEXTURE_ASSET_ID::OPEN_DOOR)
				registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::CLOSED_DOOR;
		}

	}

	for (Entity particle : registry.particleSystems.entities) {
		ParticleSystem& particleSystem = registry.particleSystems.get(particle);
		registry.remove_all_components_of(particle);
	}
}

void WorldSystem::enter_room(int from, int type) {
	// Debugging for memory/component leaks
	registry.players.get(player).last_door = from;
	registry.list_all_components();
	registry.dialogues.clear();
	// Reset the game speed
	current_speed = 1.f;

	for (auto& entity : registry.motions.entities)
	{
		if (registry.players.has(entity) || registry.transitionTimers.has(entity))
			continue;
		else {
			registry.remove_all_components_of(entity);
		}
	}
	for (Entity particle : registry.particleSystems.entities) {
		ParticleSystem& particleSystem = registry.particleSystems.get(particle);
		registry.remove_all_components_of(particle);
	}

	// Debugging for memory/component leaks
	registry.list_all_components();
	vec2 spawn = { 0,0 };
	vec2 end = { 0,0 };
	switch (from) {
		case 0:
			spawn = { window_width_px / 2, window_height_px };
			end = { window_width_px / 2, window_height_px-200 };
			break;
		case 1:
			end = { 200 , window_height_px / 2 };
			spawn = { 0 , window_height_px / 2 };
			break;
		case 2:
			end = { window_width_px / 2, 200};
			spawn = { window_width_px / 2, 0 };
			break;
		case 3:
			end = { window_width_px-200 , window_height_px / 2 };
			spawn = { window_width_px , window_height_px / 2 };
			break;
	}
	registry.dodgeTimers.remove(player);
	if (!registry.dodgeTimers.has(player))
	{
		registry.dodgeTimers.emplace(player);
		registry.motions.get(player).position = spawn;
		registry.dodgeTimers.get(player).initialPosition = spawn;
		registry.dodgeTimers.get(player).finalPosition = end;
	}
	
	switch (type) {
	case 0:
		soundSystem.playBGM();
		createEnemyRoom(renderer, player, from);
		tutorial_ongoing = false;
		shop_room = false;
		break;
	case 1:
		soundSystem.playShopBGM();
		createShopRoom(renderer, player, from);
		tutorial_ongoing = false;
		shop_room = true;
		break;
	case 2:
		createTutorialRoomTwo(renderer, player);
		tutorial_ongoing = true;
		shop_room = false;
		break;
	case 3:
		createTutorialRoomThree(renderer,player);
		tutorial_ongoing = true;
		shop_room = false;
		break;
	case 4:
		createTutorialRoomFour(renderer, player);
		tutorial_ongoing = true;
		shop_room = false;
		break;
	case 5:
		soundSystem.playBossBGM();
		createBossRoom(renderer, player, from);
		tutorial_ongoing = false;
		shop_room = false;
		break;
	}
}



// Helper function to get the motion scale of an entity
vec2 getScale(const Entity entity) {
	return {
		abs(registry.motions.get(entity).scale.x),
		abs(registry.motions.get(entity).scale.y)
	};
}

// Helper function to get the motion position of an entity
vec2 getPosition(const Entity entity) {
	return registry.motions.get(entity).position;
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {

		Collision collision = collisionsRegistry.components[i];
		Entity entity1 = collisionsRegistry.entities[i];
		Entity entity2 = collision.other_entity;

		// It is entirely possible that one of the entities has already been removed by a previous collision
		if (!registry.collisionMeshes.has(entity2) || !registry.collisionMeshes.has(entity1)) continue;

		CollisionMesh& mesh1 = registry.collisionMeshes.get(entity1);
		CollisionMesh& mesh2 = registry.collisionMeshes.get(entity2);

		// Handle collisions with solid surfaces
		if (mesh1.is_solid && mesh2.is_solid) {
			
			// Check projectile collisions
			if (registry.projectiles.has(entity1) || registry.projectiles.has(entity2)) {
				Entity projectile_entity = registry.projectiles.has(entity1) ? entity1 : entity2;
				Entity other_entity = registry.projectiles.has(entity1) ? entity2 : entity1;
				Projectile& projectile = registry.projectiles.get(projectile_entity);
				bool destroy_projectile = true;
				// Ricochet projectiles off of walls
				if (projectile.can_richochet && registry.walls.has(other_entity)) {
					// handle ricochet
					Motion& motion = registry.motions.get(projectile_entity);

					vec2 d = motion.velocity;
					vec2 n = collision.overlap_normal;					
					motion.velocity = d - 2 * dot(d, n) * n;
					motion.angle = atan(motion.velocity.y / motion.velocity.x);
					Entity particles = createParticleSystem(motion.position, collision.overlap_normal, 7, 150.0f, 0.f, 16, TEXTURE_ASSET_ID::SPARK_BASE);
					ParticleSystem& particleSys = registry.particleSystems.get(particles);
					particleSys.texture_glow = TEXTURE_ASSET_ID::SPARK_GLOW;
					destroy_projectile = false;
				} else {
					// Handle projectile damage
					if (registry.enemies.has(other_entity)) {
						Enemy& enemy = registry.enemies.get(other_entity);
						enemy.health -= projectile.damage;
						soundSystem.playEnemyDamage();

						if (registry.animations.has(other_entity)) {
							Animation& animation = registry.animations.get(other_entity);
							if (enemy.id == ENEMY_ID::NORMAL) {
								animation.row = 5;
							}
							else if (enemy.id == ENEMY_ID::ELITE) {
								animation.row = 16;
							}
							else if (enemy.id == ENEMY_ID::DUMMY) {
								animation.row = 20;
							}
							else if (enemy.id == ENEMY_ID::ZAPPER) {
								animation.row = 35;
							}
							else if (enemy.id == ENEMY_ID::FLAMETHROWER) {
								animation.row = 37;
							}
							else if (enemy.id == ENEMY_ID::BOMBER) {
								animation.row = 33;
							}
						}

                        
						if (registry.damageTimers.has(other_entity))
						{
							DamageTimer& timer = registry.damageTimers.get(other_entity);
							timer.timer_ms = 100.f;
						}
						else
						{
							registry.damageTimers.emplace(other_entity);
						}
						
						if (projectile.piercing) {
							destroy_projectile = false;
							projectile.hit_entities.push_back(other_entity);
						}

						if (projectile.can_richochet) {
							destroy_projectile = false;
							projectile.hit_entities.push_back(other_entity);
							projectile.can_richochet = false;
							Motion& motion = registry.motions.get(projectile_entity);

							vec2 d = motion.velocity;
							vec2 n = collision.overlap_normal;
							motion.velocity = d - 2 * dot(d, n) * n;
							motion.angle = atan(motion.velocity.y / motion.velocity.x);
						}

						if (enemy.health <= 0) {
							// Remove enemy
							// Drop Coins
							soundSystem.playEnemyDeath();
							createParticleSystem(registry.motions.get(projectile_entity).position, { 0, 0 }, enemy.coin_drop / 5, 5000.f, 0.f, 32, TEXTURE_ASSET_ID::COIN);
							if (enemy.id == ENEMY_ID::NORMAL) {
								createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 3000.f, 0.f, 52, TEXTURE_ASSET_ID::DEATH_BLUE);
							} else if (enemy.id == ENEMY_ID::ELITE) {
								createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 3000.f, 0.f, 74, TEXTURE_ASSET_ID::DEATH_GREEN);
							} else if (enemy.id == ENEMY_ID::DUMMY) {
								createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 3000.f, 0.f, 52, TEXTURE_ASSET_ID::DEATH);
							} else if (enemy.id == ENEMY_ID::BOMBER) {
								Motion motion = registry.motions.get(other_entity);
								explode(motion.position, motion.angle);
							} else if (enemy.id == ENEMY_ID::ZAPPER) {
								createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 3000.f, 0.f, 62, TEXTURE_ASSET_ID::DEATH_MINE);
							} else if (enemy.id == ENEMY_ID::FLAMETHROWER) {
								createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 3000.f, 0.f, 62, TEXTURE_ASSET_ID::DEATH_GARBAGE);
							}


							// Add to coin count after clearing a room
							Player& player = registry.players.components[0];
							registry.remove_all_components_of(other_entity);
							if (registry.enemies.components.size() == 0) {
								player.roomsCleared += 1;
								
								for (auto& entity : registry.doors.entities)
								{
									soundSystem.playOpenDoor();
									openDoor(entity);
								}
							}
						}


					}
					else if (registry.bosses.has(other_entity) && !registry.introTimers.has(other_entity)) {
						Boss& boss = registry.bosses.get(other_entity);
						boss.health -= projectile.damage;

						if (boss.health < 0) {
							boss.health = 0;
						}

						soundSystem.playBossDamage();
						if (registry.animations.has(other_entity)) {
							Animation& animation = registry.animations.get(other_entity);

							if (boss.id == BOSS_ID::IDLE) {
								animation.row = 1;
							}
							else {
								animation.row = 3;
							}
							
						}

						if (projectile.can_richochet) {
							destroy_projectile = false;
							projectile.hit_entities.push_back(other_entity);
							projectile.can_richochet = false;
							Motion& motion = registry.motions.get(projectile_entity);

							vec2 d = motion.velocity;
							vec2 n = collision.overlap_normal;
							motion.velocity = d - 2 * dot(d, n) * n;
							motion.angle = atan(motion.velocity.y / motion.velocity.x);
						}


						if (registry.damageTimers.has(other_entity))
						{
							DamageTimer& timer = registry.damageTimers.get(other_entity);
							timer.timer_ms = 100.f;
						}
						else
						{
							registry.damageTimers.emplace(other_entity);
						}
						if (projectile.piercing) {
							destroy_projectile = false;
							projectile.hit_entities.push_back(other_entity);
						}

						if (boss.health <= 0) {
							// Remove enemy
							// Drop Coins
							createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, boss.coin_drop / 5, 5000.f, 0.f, 32, TEXTURE_ASSET_ID::COIN);
							createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 3000.f, 0.f, 100, TEXTURE_ASSET_ID::DEATH_BOSS);
							Entity message = Entity();
							registry.messages.insert(message,
								{
									"BOSS DEFEATED!",
									vec2(window_width_px / 2, window_height_px / 2),
									FONT_SIZE_ID::EXTRA_LARGE });
							createParticleSystem({ -100, -100 }, { 0, 0 }, 300, 5000.f, 0.f, 32, TEXTURE_ASSET_ID::PRIZE);
							registry.notEnoughCoinsTimer.emplace(message);
							NotEnoughCoinsTimer& timer = registry.notEnoughCoinsTimer.components[0];
							timer.timer_ms = 5000.f;
							// Add to coin count after clearing a room
							registry.victories.emplace(player);
							registry.remove_all_components_of(other_entity);
							for (Entity entity: registry.enemies.entities ) {
								registry.remove_all_components_of(entity);
							}
							soundSystem.playBossDeath();
						}
					}
					else if (registry.players.has(other_entity)) {
						Player& player = registry.players.get(other_entity);

						if (!registry.victories.has(other_entity)) {
							player.health -= projectile.damage;

							if (player.health < 0) {
								player.health = 0;
							}

							if (!registry.damageTimers.has(other_entity) ){
								vec3& color = registry.colors.get(other_entity);
								color = { 1, 0, 0 };
								registry.damageTimers.emplace(other_entity);
							}
							soundSystem.playPlayerDamage();
						}
						if (projectile.piercing) {
							destroy_projectile = false;
							projectile.hit_entities.push_back(other_entity);
						}

						if (player.health <= 0 && registry.deathTimers.size() == 0) {
							// Handle player death


							createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, player.coins/ 5, 5000.f, 0.f, 32, TEXTURE_ASSET_ID::HEART_PARTICLE);
							createParticleSystem(registry.motions.get(other_entity).position, { 0, 0 }, 1, 5000.f, 0.f, 64, TEXTURE_ASSET_ID::DEATH);

							registry.guns.clear();
							registry.collisionMeshes.remove(other_entity);
							registry.renderRequests.remove(other_entity);
							Hotbar& hotbar = registry.hotbars.get(other_entity);
							for (int i = 0; i < hotbar.capacity; i++) {
								hotbar.guns[i] = GUN_ID::NO_GUN;
							}
							Entity deathMessage = Entity();
							registry.deathTimers.emplace(deathMessage);
							soundSystem.playPlayerDeath();
							registry.messages.insert(deathMessage,
								{ "You Died!",
								vec2(window_width_px / 2, window_height_px / 2),
								FONT_SIZE_ID::EXTRA_LARGE});
						}
						
					}
					else if (registry.itemContainers.has(other_entity)) {
						// Shatter glass container if have enough money
						float cost = registry.itemContainers.get(other_entity).cost;
						Player& player = registry.players.components[0];
						if ((player.coins - cost) >= 0) {
							soundSystem.playGlassBreak();
							player.coins -= cost;
							Animation& animation = registry.animations.get(other_entity);
							animation.frame = 0;
							if (registry.healthItems.has(other_entity)) {
								animation.row = 8;
							}
							else if (registry.gunItems.has(other_entity)) {
								if (registry.gunItems.get(other_entity).id == GUN_ID::BOUNCING_SHOT) {
									animation.row = 24;
								}
								else if (registry.gunItems.get(other_entity).id == GUN_ID::SPLIT_SHOT) {
									animation.row = 10;
								}
								else if (registry.gunItems.get(other_entity).id == GUN_ID::LONG_SHOT) {
									animation.row = 28;
								}
								else if (registry.gunItems.get(other_entity).id == GUN_ID::RAPID_SHOT) {
									animation.row = 30;
								}
							}
							else if (registry.rangeItems.has(other_entity)) {
								animation.row = 26;
							}
							registry.itemContainers.remove(other_entity);
							registry.items.get(other_entity).shattered = true;
						}
						else {
							// Produce "Not Enough Coins!" message if not enough money
							if (registry.notEnoughCoinsTimer.components.size() >= 1)
							{
								NotEnoughCoinsTimer& timer = registry.notEnoughCoinsTimer.components[0];
								timer.timer_ms = 1000.f;
							}
							else
							{
								Entity message = Entity();
								registry.notEnoughCoinsTimer.emplace(message);
								registry.messages.insert(message,
									{ 
										"Not Enough Coins!",
										vec2(window_width_px / 2, window_height_px / 2),
										FONT_SIZE_ID::EXTRA_LARGE});
								soundSystem.playError();
							}
						}
					}
				}

				if (destroy_projectile) {
					registry.remove_all_components_of(projectile_entity);
				}
			}

			// Check player collisions
			if (registry.players.has(entity1)) {
				if (registry.enemies.has(entity2) && registry.enemies.get(entity2).id == ENEMY_ID::BOMBER){
					if (!registry.summonTimers.has(entity2))
					{
						explode(registry.motions.get(entity2).position, registry.motions.get(entity2).angle);
						registry.remove_all_components_of(entity2);
						if (registry.enemies.components.size() == 0) {
							registry.players.get(player).roomsCleared += 1;
							for (auto& entity : registry.doors.entities)
							{
								soundSystem.playOpenDoor();
								openDoor(entity);
							}
						}
					}
				}
				else if (registry.healthItems.has(entity2) && !registry.itemContainers.has(entity2)) {
					Animation& animation = registry.animations.get(entity2);
					animation.frame = 0;
					animation.row = 6;
					animation.num_frames = 1;

					Player& player = registry.players.get(entity1);
					player.health += registry.healthItems.get(entity2).restore;

					if (player.health > player.max_health) {
						player.health = player.max_health;
					}
					soundSystem.playHeal();
					registry.healthItems.remove(entity2);
					registry.collisionMeshes.remove(entity2);
					registry.items.get(entity2).item_id = ITEM_ID::NO_ITEM;
				}

				if (registry.gunItems.has(entity2) && !registry.itemContainers.has(entity2)) {
					Animation& animation = registry.animations.get(entity2);
					animation.frame = 0;
					animation.row = 6;
					animation.num_frames = 1;

					if (registry.gunItems.get(entity2).id == GUN_ID::BOUNCING_SHOT) {
						registry.hasBounces.emplace(entity1);
					}
					else if (registry.gunItems.get(entity2).id == GUN_ID::SPLIT_SHOT) {
						registry.hasSplits.emplace(entity1);
					}
					else if (registry.gunItems.get(entity2).id == GUN_ID::LONG_SHOT) {
						registry.hasLongs.emplace(entity1);
					}
					else if (registry.gunItems.get(entity2).id == GUN_ID::RAPID_SHOT) {
						registry.hasRapids.emplace(entity1);
					}

					Hotbar& hotbar = registry.hotbars.get(entity1);
					for (int i = 0; i < hotbar.capacity; i++) {
						if (hotbar.guns[i] == GUN_ID::NO_GUN) {
							hotbar.guns[i] = registry.gunItems.get(entity2).id;
							break;
						}
					}
					soundSystem.playPowerUp();

					registry.gunItems.remove(entity2);
					registry.collisionMeshes.remove(entity2);
					registry.items.get(entity2).item_id = ITEM_ID::NO_ITEM;
				}

				if (registry.rangeItems.has(entity2) && !registry.itemContainers.has(entity2)) {
					Animation& animation = registry.animations.get(entity2);
					animation.frame = 0;
					animation.row = 6;
					animation.num_frames = 1;

					Player& player = registry.players.get(entity1);
					player.range += registry.rangeItems.get(entity2).increase;

					if (player.range > player.max_range) {
						player.range = player.range;
					}
					soundSystem.playPowerUp();
					registry.rangeItems.remove(entity2);
					registry.collisionMeshes.remove(entity2);
					registry.items.get(entity2).item_id = ITEM_ID::NO_ITEM;
				}

				//handle_player_collisions(entity1, entity2);
				if (registry.dodgeTimers.has(entity1)) {
					registry.dodgeTimers.remove(entity1);
				}
			}
			else if (registry.players.has(entity2)) {
				if (registry.healthItems.has(entity1) && !registry.itemContainers.has(entity1)) {
					Animation& animation = registry.animations.get(entity1);
					animation.frame = 0;
					animation.row = 6;
					animation.num_frames = 1;

					Player& player = registry.players.get(entity2);
					player.health += registry.healthItems.get(entity1).restore;

					if (player.health > player.max_health) {
						player.health = player.max_health;
					}
					soundSystem.playHeal();
					registry.healthItems.remove(entity1);
					registry.collisionMeshes.remove(entity1);
					registry.items.get(entity1).item_id = ITEM_ID::NO_ITEM;
				}

				if (registry.gunItems.has(entity1) && !registry.itemContainers.has(entity1)) {
					Animation& animation = registry.animations.get(entity1);
					animation.frame = 0;
					animation.row = 6;
					animation.num_frames = 1;

					if (registry.guns.has(entity2)) {
						registry.guns.remove(entity2);
					}

					if (registry.gunItems.get(entity1).id == GUN_ID::BOUNCING_SHOT) {
						registry.hasBounces.emplace(entity2);
					}
					else if (registry.gunItems.get(entity1).id == GUN_ID::SPLIT_SHOT) {
						registry.hasSplits.emplace(entity2);
					}
					else if (registry.gunItems.get(entity1).id == GUN_ID::LONG_SHOT) {
						registry.hasLongs.emplace(entity2);
					}
					else if (registry.gunItems.get(entity1).id == GUN_ID::RAPID_SHOT) {
						registry.hasRapids.emplace(entity2);
					}

					Hotbar& hotbar = registry.hotbars.get(entity2);
					for (int i = 0; i < hotbar.capacity; i++) {
						if (hotbar.guns[i] == GUN_ID::NO_GUN) {
							hotbar.guns[i] = registry.gunItems.get(entity1).id;
							break;
						}
					}
					soundSystem.playPowerUp();
					registry.gunItems.remove(entity1);
					registry.collisionMeshes.remove(entity1);
					registry.items.get(entity1).item_id = ITEM_ID::NO_ITEM;
				}

				if (registry.rangeItems.has(entity1) && !registry.itemContainers.has(entity1)) {
					Animation& animation = registry.animations.get(entity1);
					animation.frame = 0;
					animation.row = 6;
					animation.num_frames = 1;

					Player& player = registry.players.get(entity2);
					player.range += registry.rangeItems.get(entity1).increase;

					if (player.range > player.max_range) {
						player.range = player.max_range;
					}
					soundSystem.playPowerUp();
					registry.rangeItems.remove(entity1);
					registry.collisionMeshes.remove(entity1);
					registry.items.get(entity1).item_id = ITEM_ID::NO_ITEM;
				}
				// Handle player collisions here
				//handle_player_collisions(entity2, entity1);
				if (registry.dodgeTimers.has(entity2)) {
					registry.dodgeTimers.remove(entity2);
				}
			}
		}
		else { //player collides with open door
			if ((registry.players.has(entity1) || registry.players.has(entity2)) && !registry.transitionTimers.has(entity1) ) {
				printf("entered door\n");
				{
					if (!registry.dodgeTimers.has(entity1) && !registry.transitionTimers.has(entity1)) {
						mesh1.is_solid = false;	
						soundSystem.playSteps();
						TransitionTimer& transitionTimer = registry.transitionTimers.emplace(entity1);
						Door& door = registry.doors.get(entity2);
						transitionTimer.from = door.direction;
						transitionTimer.type = door.type;
						printf("%d\n", transitionTimer.from);
						registry.dodgeTimers.emplace(entity1);
						registry.dodgeTimers.get(entity1).initialPosition = registry.motions.get(entity1).position;
						switch (door.direction) {
							case 0:
								registry.dodgeTimers.get(entity1).finalPosition = { registry.motions.get(entity1).position.x
							, registry.motions.get(entity1).position.y - 300};
								break;
							case 1:
								registry.dodgeTimers.get(entity1).finalPosition = { registry.motions.get(entity1).position.x
							+ 300, registry.motions.get(entity1).position.y };
								break;
							case 2:
								registry.dodgeTimers.get(entity1).finalPosition = { registry.motions.get(entity1).position.x
							, registry.motions.get(entity1).position.y + 300};
								break;
							case 3:
								registry.dodgeTimers.get(entity1).finalPosition = { registry.motions.get(entity1).position.x
							- 300, registry.motions.get(entity1).position.y };
								break;
						}
					}
				}
			}
		}
	}
	// Remove all collisions from this simulation step
	registry.collisions.clear();
	return;
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// On key callback
void WorldSystem::on_key(int key, int, int action, int mod) {

	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
        	restart_game();
	}

	// Debugging
	if (key == GLFW_KEY_D) {
		if (action == GLFW_RELEASE)
			debugging.in_debug_mode = false;
		else
			debugging.in_debug_mode = true;
	}

#ifdef DEBUG_OPENGL
	// cheat: add room clear count
	if (action == GLFW_PRESS && key == GLFW_KEY_9) {
		Player& player = registry.players.components[0];
		player.roomsCleared += 1;
	}
	// cheat: add coins
	if (action == GLFW_PRESS && key == GLFW_KEY_8) {
		Player& player = registry.players.components[0];
		player.coins += 100;
	}
#endif

	if (action == GLFW_PRESS && key == GLFW_KEY_1) {
		Hotbar& hotbar = registry.hotbars.get(player);
		hotbar.selected = 0;
		swapGun(player, hotbar.selected);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_2) {
		Hotbar& hotbar = registry.hotbars.get(player);
		hotbar.selected = 1;
		swapGun(player, hotbar.selected);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_3) {
		Hotbar& hotbar = registry.hotbars.get(player);
		hotbar.selected = 2;
		swapGun(player, hotbar.selected);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_4) {
		Hotbar& hotbar = registry.hotbars.get(player);
		hotbar.selected = 3;
		swapGun(player, hotbar.selected);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_5) {
		Hotbar& hotbar = registry.hotbars.get(player);
		hotbar.selected = 4;
		swapGun(player, hotbar.selected);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_6) {
		Hotbar& hotbar = registry.hotbars.get(player);
		hotbar.selected = 5;
		swapGun(player, hotbar.selected);
	}


	static Entity help_screen = Entity();
	// Esc Pressed
	if (key == GLFW_KEY_ESCAPE) {
		if (action == GLFW_PRESS) {
			if (!ui_system->in_state(UI_STATE_ID::CONTROLS) && !ui_system->in_state(UI_STATE_ID::MENU) && !ui_system->in_state(UI_STATE_ID::SCORE) && !registry.alerts.size()) {
				ui_system->toggle_state(UI_STATE_ID::PAUSE);
			}
		}
	}

	// Control the current speed with `<` `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_COMMA) {
		current_speed -= 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT) && key == GLFW_KEY_PERIOD) {
		current_speed += 0.1f;
		printf("Current speed = %f\n", current_speed);
	}
	current_speed = fmax(0.f, current_speed);


	// Disabled because it doesn't work.
	/*if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) {
		if (glfwGetWindowMonitor(window)) {
			// Switch to windowed mode
			glfwSetWindowMonitor(window, nullptr, 50, 50, resolutionWidth, resolutionHeight, GLFW_DONT_CARE);
		} else {
			// Switch to fullscreen mode
			glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, resolutionWidth, resolutionHeight, GLFW_DONT_CARE);
		}
	}*/

	ControllerSystem::on_key_controller(key, 0, action, mod, renderer);
}

void WorldSystem::on_scroll(double xoffset, double yoffset) {
	if (yoffset > 0) {
		Hotbar& hotbar = registry.hotbars.get(player);
		if (hotbar.selected > 0) {
			hotbar.selected -= 1;
			swapGun(player, hotbar.selected);
		}
	}
	else if (yoffset < 0) {
		Hotbar& hotbar = registry.hotbars.get(player);
		if (hotbar.selected < hotbar.capacity - 1) {
			hotbar.selected += 1;
			swapGun(player, hotbar.selected);
		}
	}
}

bool WorldSystem::load_game() {
	current_speed = 1.f;
	while (registry.motions.entities.size() > 0)
		registry.remove_all_components_of(registry.motions.entities.back());
	registry.dialogues.clear();
	registry.alerts.clear();
	registry.messages.clear();
	registry.deathTimers.clear();
	// Debugging for memory/component leaks
	registry.list_all_components();
	registry.screenStates.components[0].screen_darken_factor = 0.0;
	for (Entity particle : registry.particleSystems.entities) {
		ParticleSystem& particleSystem = registry.particleSystems.get(particle);
		registry.remove_all_components_of(particle);
	}
	
	printf("Loading game...\n");
	player = createPlayer(renderer, { 200, 200 });
	Player& player_state = registry.players.get(player);
	Hotbar& hotbar_state = registry.hotbars.get(player);
	Motion& motion_state = registry.motions.get(player);
	tutorial_ongoing = false;
	shop_room = true;

	std::string filePath = std::string(PROJECT_SOURCE_DIR) + "/data/save.json";
	try {
		std::ifstream file(filePath);
		if (!file) {
			std::cout << "Error opening file: " << filePath << std::endl;
			return false;
		}
		char ch;

		// Load player component
		float mouse_x, mouse_y, health, max_health, range, max_range;
		int coins, immune, last_door, roomsCleared;
		file >> ch >> mouse_x >> ch >> mouse_y >> ch >> health >> ch >> max_health >> ch >> range >> ch 
			>> max_range >> ch >> coins >> ch >> immune >> ch >> roomsCleared >> ch >> last_door >> ch;
		player_state.mouse_position = { mouse_x, mouse_y };
		player_state.health = health;
		player_state.max_health = max_health;
		player_state.range = range;
		player_state.max_range = max_range;
		player_state.coins = coins;
		player_state.immune = (bool) immune;
		player_state.roomsCleared = roomsCleared;
		player_state.last_door = last_door;

		// Load hotbar component
		int selected, capacity_dummy;
		const int capacity = hotbar_state.capacity;
		GUN_ID guns[capacity];
		file >> selected >> ch >> capacity_dummy >> ch;
		for (int i = 0; i < capacity; i++) {
			int gun;
			file >> gun >> ch;
			hotbar_state.guns[i] = (GUN_ID)gun;
		}
		hotbar_state.selected = selected;
		swapGun(player, selected);

		// Load doors
		int doors[6];
		for (int i = 0; i < 6; i = i + 2) {
			file >> doors[i] >> ch >> doors[i + 1] >> ch;
		}

		// Load motion
		float position_x, position_y, angle;
		file >> position_x >> ch >> position_y >> ch >> angle >> ch;
		motion_state.position = { position_x, position_y };
		motion_state.angle = angle;

		// Load guns
		bool hasBounces, hasSplits, hasLongs, hasRapids;
		file >> hasBounces >> ch >> hasSplits >> ch >> hasLongs >> ch >> hasRapids >> ch;
		if (hasBounces)
		{
			registry.hasBounces.emplace(player);
		}
		if (hasSplits)
		{
			registry.hasSplits.emplace(player);
		}
		if (hasLongs)
		{
			registry.hasLongs.emplace(player);
		}
		if (hasRapids)
		{
			registry.hasRapids.emplace(player);
		}

		// Load items
		int numItems;
		file >> numItems >> ch;
		vec2* positions = new vec2[numItems];
		ITEM_ID* item_ids = new ITEM_ID[numItems];
		GUN_ID* gun_ids = new GUN_ID[numItems];
		bool* shatters = new bool[numItems];
		for (int i = 0; i < numItems; i++) {
			float x, y;
			int item_id, gun_id, shattered;
			file >> x >> ch >> y >> ch >> item_id >> ch >> gun_id >> ch >> shattered >> ch;
			positions[i] = { x, y };
			item_ids[i] = (ITEM_ID)item_id;
			gun_ids[i] = (GUN_ID)gun_id;
			shatters[i] = (bool)shattered;
		}

		// Create Room and Start up
		createLastRoom(renderer, player, last_door, doors, numItems, positions, item_ids, gun_ids, shatters);
		ControllerSystem::set_player(player);
		registry.colors.insert(player, { 1, 1, 1 });
		soundSystem.playShopBGM();
	}
	catch (const std::exception& e) {
		std::cout << "Exception: " << e.what() << std::endl;
		return false;
	}
	return true;
}

void WorldSystem::save_game() {
	printf("Saving game");
	Entity player = registry.players.entities[0];
	Player& player_state = registry.players.get(player);
	Hotbar& hotbar_state = registry.hotbars.get(player);
	Motion& motion_state = registry.motions.get(player);

	std::string filePath = std::string(PROJECT_SOURCE_DIR) + "/data/save.json";
	try {
		std::ofstream file(filePath);
		if (!file) {
			std::cout << "Error opening file: " << filePath << std::endl;
			return;
		}
		// Save player component
		file << "[" << player_state.mouse_position.x << "," << player_state.mouse_position.y << ","
			<< player_state.health << "," << player_state.max_health << "," << player_state.range << ","
			<< player_state.max_range << "," << player_state.coins << "," << player_state.immune << "," 
			<< player_state.roomsCleared << "," << player_state.last_door;

		// Save hotbar component
		file << "," << hotbar_state.selected << "," << hotbar_state.capacity;
		int capacity = hotbar_state.capacity;
		for (int i = 0; i < capacity; i++) {
			file << "," << (int)hotbar_state.guns[i];
		}

		// Save doors
		for (int i = 0; i < 3; i++) {
			file << "," << registry.doors.components[i].direction << "," << registry.doors.components[i].type;
		}
		// Save motion
		file << "," << motion_state.position.x << "," << motion_state.position.y << "," << motion_state.angle;

		// Save guns
		file << "," << registry.hasBounces.has(player) << "," << registry.hasSplits.has(player) << "," << 
			registry.hasLongs.has(player) << "," << registry.hasRapids.has(player);

		// Save items
		int numItems = registry.items.components.size();
		file << "," << numItems;
		for (int i = 0; i < numItems; i++) {
			file << "," << registry.items.components[i].position.x << "," <<
				registry.items.components[i].position.y << "," <<
				(int) registry.items.components[i].item_id << "," <<
				(int) registry.items.components[i].gun_id << "," <<
				(int) registry.items.components[i].shattered;
		}

		file << "]";
		if (!file) {
			std::cout << "Error writing to file: " << filePath << std::endl;
			return;
		}
	}
	catch (const std::exception& e) {
		std::cout << "Exception: " << e.what() << std::endl;
	}
}

void WorldSystem::addRandomMovement(Boid& boid) {
	float randomness = 5.0f; // Adjust the randomness factor as needed
	float randomX = 0.1;
	float randomY = 0.1;
	float xFactor = (registry.motions.get(player).position.x > boid.position.x) ? 0.5: -0.5;

	float yFactor = (registry.motions.get(player).position.y > boid.position.y) ? 0.5 : -0.5;
	boid.velocity.x += randomX * xFactor;
	boid.velocity.y += randomY * yFactor;
}

void WorldSystem::explode(vec2 position, float angle) {
	soundSystem.playEnemyDeath();
	float deg_to_rad = M_PI / 180.0;
	float other_angle = angle + M_PI;
	float side_angle1 = angle + M_PI / 2;
	float side_angle2 = angle - M_PI / 2;
	createBullet(position, angle - 45 * deg_to_rad, 300, false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, angle, 300, false, 200, 0, false, 5, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, angle + 45 * deg_to_rad, 300,  false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);

	createBullet(position, other_angle - 45 * deg_to_rad, 300,  false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, other_angle, 300, false, 200, 0, false, 5, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, other_angle + 45 * deg_to_rad, 300, false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);

	createBullet(position, side_angle1 - 45 * deg_to_rad, 300, false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, side_angle1, 300, false, 200, 0, 0,  5, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, side_angle1 + 45 * deg_to_rad, 300, false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);

	createBullet(position, side_angle2 - 45 * deg_to_rad, 300, false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, side_angle2, 300, false, 200, 0, 0, 5, 1, GUN_ID::EXPLOSION_SHOT);
	createBullet(position, side_angle2 + 45 * deg_to_rad, 300, false, 200, 0, false, 10, 1, GUN_ID::EXPLOSION_SHOT);
	createParticleSystem(position, { 0, 0 }, 1, 3000.f, 0.f, 62, TEXTURE_ASSET_ID::DEATH_BOMB);

}

void WorldSystem::on_mouse_button(int button, int action, int mod)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && !registry.dodgeTimers.has(player)) {
		createParticleSystem({ -100, -100 }, { 0, 0 }, 5, 500.f, 0.f, 64, TEXTURE_ASSET_ID::SMOKE);
	}

	
	ControllerSystem::on_mouse_button_controller(button, action, mod);
}
