#pragma once

#include <array>
#include <utility>
#include <vector>
#include <unordered_set>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include "ui_system.hpp"

#define MAX_PARTICLES 768
#define PARTICLE_BUFFER_SIZE MAX_PARTICLES * 2 * sizeof(float)
#define BUFFER_SIZE(numParticles) numParticles * 2 * sizeof(float)
#define MAX_PARTICLES_ON_SCREEN 40

struct ParticleGLDetails
{
	GLuint particleVBO;
	void* particleBufferData;
	// Absolute max particles = 1024
};

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	// const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	// {
	// 	  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SALMON, mesh_path("salmon.obj"))
	// 	  // specify meshes of other assets here
	// };

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("spritesheet.png"),
			textures_path("vertical_wall_door.png"),
			textures_path("vertical_wall_long.png"),
			textures_path("horizontal_wall_door.png"),
			textures_path("horizontal_wall_long.png"),
			textures_path("floor.png"),
			textures_path("bullet-base.png"),
			textures_path("bullet-glow.png"),
			textures_path("heart.png"),
			textures_path("ice-shard-base.png"),
			textures_path("ice-shard-glow.png"),
			textures_path("open_door.png"),
			textures_path("closed_door.png"),
			textures_path("death-message.png"),
			textures_path("not_enough_coins_message.png"),
			textures_path("coin.png"),
			textures_path("coin-glow.png"),
			textures_path("tutorial.png"),
			textures_path("tutorial1.png"),
			textures_path("tutorial2.png"),
			textures_path("tutorial3.png"),
			textures_path("tutorial4.png"),
			textures_path("dodge.png"),
			textures_path("ice-shard-base-spline.png"),
			textures_path("ice-shard-glow-spline.png"),
			textures_path("exit_button.png"),
			textures_path("menu_button.png"),
			textures_path("menu_button_pressed.png"),
			textures_path("menu_panel.png"),
			textures_path("hotbar_panel.png"),
			textures_path("hotbar_panel_selected.png"),
			textures_path("hotbar_target.png"),
			textures_path("ui_panel.png"),
			textures_path("dialogue_panel.png"),
			textures_path("dialogue_arrow.png"),
			textures_path("alert_panel.png"),
			textures_path("title_screen.png"),
			textures_path("title_screen_panel.png"),
			textures_path("boss_health_bar.png"),
			textures_path("straight_shot.png"),
			textures_path("bounce_shot.png"),
			textures_path("split_shot.png"),
			textures_path("long_shot.png"),
			textures_path("rapid_shot.png"),
			textures_path("energy-blast-base.png"),
			textures_path("energy-blast-glow.png"),
			textures_path("bolt-base.png"),
			textures_path("bolt-glow.png"),
			textures_path("fire-base.png"),
			textures_path("fire-glow.png"),
			textures_path("boss.png"),
			textures_path("deathParticle.png"),
			textures_path("coin.png"),
			textures_path("deathParticleGreen.png"),
			textures_path("deathParticleBlue.png"),
			textures_path("deathParticleBoss.png"),
			textures_path("bullet-dead-base.png"),
			textures_path("bullet-dead-glow.png"),
			textures_path("enemy-bullet-dead.png"),
			textures_path("spline-dead.png"),
			textures_path("spark-base.png"),
			textures_path("spark-glow.png"),
			textures_path("explosion-base.png"),
			textures_path("explosion-glow.png"),
			textures_path("coin.png"),
			textures_path("deathParticleBomb.png"),
			textures_path("deathParticleGarbage.png"),
			textures_path("deathParticleMine.png"),
			textures_path("zapper-bullet-dead.png"),
			textures_path("range_icon.png"),
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("particle"),
		shader_path("textured"),
		shader_path("water"),
		shader_path("blur"),
		shader_path("textured-glow"),
		shader_path("animated"),
		shader_path("boss")};

	const std::vector< std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
		// There are none
	};

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:
	// Initialize the window
	bool init(GLFWwindow* window, UISystem* ui_system);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();
	
	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlMeshes();

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the water
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw();

	static mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(RenderRequest& render, Motion& motion, Entity entity,
		const mat3& projection);

	void drawParticleSystem(const mat3& projection, ParticleSystem& particleSystem);
	void blurBloom();
	void drawToScreen();

	// Window handle
	GLFWwindow* window;
	UISystem *ui_system;
	// Screen texture handles
	GLuint frame_buffer;

	GLuint off_screen_render_buffer_color;
	GLuint off_screen_emissive_buffer;

	// pingpong buffer handles
	GLuint pingpongFBO[2];
	GLuint pingpongBuffer[2];

	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	// Absolute max particles = 512
	const int maxParticles = MAX_PARTICLES;
	const int particleBufferSize = PARTICLE_BUFFER_SIZE;
	const int maxParticlesOnScreen = MAX_PARTICLES_ON_SCREEN;

	std::vector<ParticleGLDetails> particleBufferPool;
	int particleBufferIndex;

	// GLOBAL VAO
	GLuint globalVAO;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
