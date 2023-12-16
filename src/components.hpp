#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"


struct SplineBullet
{
	float total_distance;
	float distance_covered = 0.f;
	float speed = 600.f;
	bool last_point_crossed = false;
	int last_point = 0;
	std::vector<vec2> points_on_line;
};

// Player component
struct Player
{
	vec2 mouse_position = { 0.f, 0.f };
	float health = 100.f;
	float max_health = 100.f;
	float range = 500.f;
	float max_range = 1000.f;
	int coins = 0;
	bool immune = 0;
	bool right = false;
	bool left = false;
	bool up = false;
	bool down = false;
	int roomsCleared = 0;
	int last_door = 0;
};

enum class ENEMY_ID {
	NORMAL = 0,
	ELITE = NORMAL + 1,
	DUMMY = ELITE + 1,
	ZAPPER = DUMMY + 1,
	FLAMETHROWER = ZAPPER + 1,
	BOMBER = FLAMETHROWER + 1
};

struct Enemy
{
	float health;
	float max_health;
	float coin_drop;
	ENEMY_ID id;
};

enum class BOSS_ID
{
	IDLE = 0,
	ACTION = IDLE + 1
};

struct Boss
{
	float health = 500.f;
	float max_health = 5000.f;
	float coin_drop = 1000.f;
	float max_drop = 9000.f;
	float cooldown = 9000.f * 2;
	BOSS_ID id = BOSS_ID::IDLE;
};

enum class WALL_ID {
	HORIZONTAL_DOOR = 0,
	HORIZONTAL_LONG = HORIZONTAL_DOOR + 1,
	VERTICAL_DOOR = HORIZONTAL_LONG + 1,
	VERTICAL_LONG = VERTICAL_DOOR + 1,
};

// Wall component
struct Wall
{

};

struct Projectile
{	
	bool shot_by_player = true;
	bool can_richochet = true;
	bool piercing = false;
	bool explodes = false;
	float max_distance = 2000.f;
	float distance_travelled = 0.f;
	float damage = 10.f;
	std::vector<Entity> hit_entities;
};

enum class GUN_ID {
	STRAIGHT_SHOT = 0,
	SPLIT_SHOT = STRAIGHT_SHOT + 1,
	BOUNCING_SHOT = SPLIT_SHOT + 1,
	SPLINE_SHOT = BOUNCING_SHOT + 1,
	SPLIT_SPLINE_SHOT = SPLINE_SHOT + 1,
	RAPID_SHOT = SPLIT_SPLINE_SHOT + 1,
	LONG_SHOT = RAPID_SHOT + 1,
	OMNI_SHOT = LONG_SHOT + 1,
	ZAP_SHOT = OMNI_SHOT + 1,
	FIRE_SHOT = ZAP_SHOT + 1,
	EXPLOSION_SHOT = FIRE_SHOT + 1,
	GUN_COUNT = FIRE_SHOT + 1,
	NO_GUN = GUN_COUNT + 1,
};

const int gun_count = (int)GUN_ID::GUN_COUNT;

struct Gun
{
	float cooldown_ms;
	float timer_ms = 0.f;
	bool is_firing = false;
	GUN_ID gun_id;
};

// Used by the player to keep track of gun status when switching between guns
struct GunStatus
{
	bool is_firing = false;
	float timer_ms = 0.f;
};

struct Hotbar
{
	int selected = 0;
	const static int capacity = 6;
	GUN_ID guns[capacity] = { GUN_ID::NO_GUN, GUN_ID::NO_GUN, GUN_ID::NO_GUN, GUN_ID::NO_GUN, GUN_ID::NO_GUN, GUN_ID::NO_GUN };
};

// All data relevant to the shape and motion of entities
struct Motion {
	vec2 position = { 0.f, 0.f };
	float angle = 0.f;
	vec2 velocity = { 0.f, 0.f };
	vec2 scale = { 10.f, 10.f };
};

struct Interpolation {
	vec2 start_position = { 0.f, 0.f };
	vec2 end_position = { 0.f, 0.f };
	float period_ms = 1000.f;
	float timer_ms = 0.f;
};

struct Acceleration {
	float acceleration = 900.f;
	float deceleration = 1100.f;
	float max_speed_acc = 300.f;
};

struct Boid
{
	vec2 position;
	vec2 velocity;
};

struct Flock
{
	std::vector<Entity> boids;
	Entity target;
};


// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other_entity; // the second object involved in the collision
	Collision(Entity& other_entity) { this->other_entity = other_entity; };
	float min_overlap = 0.f;
	vec2 overlap_normal = { 0.f, 0.f };
};

// Defines the structure around an object which can collide with other objects
// The structure is defined by a set of vertices and a set of polygons
// Each individual polygon must be convex but the total shape can be non-convex
struct CollisionMesh
{
	std::vector<vec2> vertices;
	std::vector<std::vector<int>> polygons;
	bool is_solid = true;
	bool is_static = false;
};

// A cache entry for the collision mesh of an entity
// Calculated once per frame per entity
struct CollisionCacheEntry
{
	std::vector<vec2> vertices;
	vec2 AABB_min;
	vec2 AABB_max;
	vec2 position;
	float angle;
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float screen_darken_factor = 0;
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float timer_ms = 3000.f;
};

// A timer associated to dodging
struct DodgeTimer
{
	float timer_ms = 300.f;
	float time_left = 300.f;
	vec2 initialPosition;
	vec2 finalPosition;
};

// A timer associated to dodging
struct TransitionTimer
{
	float timer_ms = 600.f;
	int from = 1;
	int type = 0;
	bool entering = false;
};

struct Door
{
	int direction = 0;
	int type = 0;
};

// A timer that will be associated to a salmon lighting-up
struct LightUp
{
	float timer_ms = 500.f;
};

// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

struct AnimatedVertex
{
	vec3 position;
	vec2 texcoord;
};

struct BossVertex
{
	vec3 position;
	vec2 texcoord;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	static bool loadBlenderFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct Animation
{
	int frame;
	int row;
	int num_frames;
	float speed;
	float timer = 0.f;
};

struct DamageTimer
{
	float timer_ms = 100.f;
};

enum class ITEM_ID {
	HEALTH = 0,
	GUN = HEALTH + 1,
	RANGE = GUN + 1,
	NO_ITEM = RANGE + 1
};

struct Item {
	ITEM_ID item_id;
	GUN_ID gun_id;
	vec2 position;
	bool shattered;
};

struct ItemContainer
{
	float cost;
};

struct HealthItem {
	float restore = 30.f;
};

struct GunItem {
	GUN_ID id;
};

struct RangeItem {
	float increase = 50.f;
};

struct HasSplit {

};

struct HasBounce {

};

struct HasLong {

};

struct HasRapid {

};

struct NotEnoughCoinsTimer {
	float timer_ms = 1000.f;
};

struct IntroTimer
{
	float timer_ms = 2000.f;
};

struct RoarTimer
{
	float timer_ms = 9000.f;
};

struct SummonTimer {
	float timer_ms = 3000.f;
};

struct Victory
{

};

struct Rotate {
	bool direction;
};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID {
	SPRITESHEET = 0,
	VERTICAL_WALL_DOOR = SPRITESHEET + 1,
	VERTICAL_WALL_LONG = VERTICAL_WALL_DOOR + 1,
	HORIZONTAL_WALL_DOOR = VERTICAL_WALL_LONG + 1,
	HORIZONTAL_WALL_LONG = HORIZONTAL_WALL_DOOR + 1,
	FLOOR = HORIZONTAL_WALL_LONG + 1,
	BULLET_BASE = FLOOR + 1,
	BULLET_GLOW = BULLET_BASE + 1,
	HEART = BULLET_GLOW + 1,
	ICE_SHARD_BASE = HEART + 1,
	ICE_SHARD_GLOW = ICE_SHARD_BASE + 1,
	OPEN_DOOR = ICE_SHARD_GLOW + 1,
	CLOSED_DOOR = OPEN_DOOR + 1,
	DEATH_MESSAGE = CLOSED_DOOR + 1,
	NOT_ENOUGH_COINS_MESSAGE = DEATH_MESSAGE + 1,
	COIN = NOT_ENOUGH_COINS_MESSAGE + 1,
	COIN_GLOW = COIN + 1,
	TUTORIAL = COIN_GLOW + 1,
	TUTORIAL1 = TUTORIAL + 1,
	TUTORIAL2 = TUTORIAL1 + 1,
	TUTORIAL3 = TUTORIAL2 + 1,
	TUTORIAL4 = TUTORIAL3 + 1,
	SMOKE = TUTORIAL4 + 1,
	ICE_SHARD_BASE_SPLINE = SMOKE + 1,
	ICE_SHARD_GLOW_SPLINE = ICE_SHARD_BASE_SPLINE + 1,
	EXIT_BUTTON = ICE_SHARD_GLOW_SPLINE + 1,
	MENU_BUTTON = EXIT_BUTTON + 1,
	MENU_BUTTON_PRESSED = MENU_BUTTON + 1,
	MENU_PANEL = MENU_BUTTON_PRESSED + 1,
	HOTBAR_PANEL = MENU_PANEL + 1,
	HOTBAR_PANEL_SELECTED = HOTBAR_PANEL + 1,
	HOTBAR_TARGET = HOTBAR_PANEL_SELECTED + 1,
	UI_PANEL = HOTBAR_TARGET + 1,
	DIALOGUE_PANEL = UI_PANEL + 1,
	DIALOGUE_ARROW = DIALOGUE_PANEL + 1,
	ALERT_PANEL = DIALOGUE_ARROW + 1,
	TITLE_SCREEN = ALERT_PANEL + 1,
	TITLE_SCREEN_PANEL = TITLE_SCREEN + 1,
	BOSS_HEALTH_BAR = TITLE_SCREEN_PANEL + 1,
	STRAIGHT_SHOT = BOSS_HEALTH_BAR + 1,
	BOUNCE_SHOT = STRAIGHT_SHOT+ 1,
	SPLIT_SHOT = BOUNCE_SHOT + 1,
	LONG_SHOT = SPLIT_SHOT + 1,
	RAPID_SHOT = LONG_SHOT + 1,
	ENERGY_BLAST_BASE = RAPID_SHOT + 1,
	ENERGY_BLAST_GLOW = ENERGY_BLAST_BASE + 1,
	BOLT_BASE = ENERGY_BLAST_GLOW + 1,
	BOLT_GLOW = BOLT_BASE + 1,
	FIRE_BASE = BOLT_GLOW + 1,
	FIRE_GLOW = FIRE_BASE + 1,
	BOSS = FIRE_GLOW + 1,
	DEATH = BOSS + 1,
	HEART_PARTICLE = DEATH + 1,
	DEATH_GREEN = HEART_PARTICLE + 1,
	DEATH_BLUE = DEATH_GREEN + 1,
	DEATH_BOSS = DEATH_BLUE + 1,
	BULLET_DEAD_BASE = DEATH_BOSS + 1,
	BULLET_DEAD_GLOW = BULLET_DEAD_BASE + 1,
	ENEMY_BULLET_DEAD = BULLET_DEAD_GLOW + 1,
	SPLINE_BULLET_DEAD = ENEMY_BULLET_DEAD + 1,
	SPARK_BASE = SPLINE_BULLET_DEAD + 1,
	SPARK_GLOW = SPARK_BASE + 1,
	EXPLOSION_BASE = SPARK_GLOW + 1,
	EXPLOSION_GLOW = EXPLOSION_BASE + 1,
	PRIZE = EXPLOSION_GLOW + 1,
	DEATH_BOMB = PRIZE + 1,
	DEATH_GARBAGE = DEATH_BOMB + 1,
	DEATH_MINE = DEATH_GARBAGE + 1,
	BULLET_DEAD_ZAPPER = DEATH_MINE + 1,
	RANGE_ICON = BULLET_DEAD_ZAPPER + 1,
	TEXTURE_COUNT = RANGE_ICON + 1
};

const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	PARTICLE = COLOURED + 1,
	TEXTURED = PARTICLE + 1,
	WATER = TEXTURED + 1,
	BLOOM = WATER + 1,
	TEXTURED_GLOW = BLOOM + 1,
	ANIMATED = TEXTURED_GLOW + 1,
	BOSS_ANIMATE = ANIMATED + 1,
	EFFECT_COUNT = BOSS_ANIMATE + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	N_MESHES = 0,
	SPRITE = N_MESHES + 1,
	ANIMATED_SPRITE = SPRITE + 1,
	BOSS_SPRITE = ANIMATED_SPRITE + 1,
	SCREEN_TRIANGLE = BOSS_SPRITE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int n_meshes = (int)GEOMETRY_BUFFER_ID::N_MESHES;
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

enum class RENDER_LAYER_ID {
	BACKGROUND_FAR = 0,
	BACKGROUND_NEAR = BACKGROUND_FAR + 1,
	MIDGROUND_FAR = BACKGROUND_NEAR + 1,
	MIDGROUND_NEAR = MIDGROUND_FAR + 1,
	FOREGROUND = MIDGROUND_NEAR + 1,
	UI = FOREGROUND + 1,
	RENDER_LAYER_COUNT = UI + 1
};

const int render_layer_count = (int)RENDER_LAYER_ID::RENDER_LAYER_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
};

// Must have a RenderRequest with the glow effect to work
struct Glows {
	TEXTURE_ASSET_ID glowmap = TEXTURE_ASSET_ID::TEXTURE_COUNT;
};

enum class UI_STATE_ID
{
	MENU = 0b1,
	PLAYING = MENU << 1,
	DEATH = PLAYING << 1,
	PAUSE = DEATH << 1,
	CONTROLS = PAUSE << 1,
	SCORE = CONTROLS << 1,
};

enum class UI_BUTTON_ID
{
	NEW_GAME = 0,
	LOAD_GAME = NEW_GAME + 1,
	QUIT = LOAD_GAME + 1,
	RESUME = QUIT + 1,
	SAVE_GAME = RESUME + 1,
	MAIN_MENU = SAVE_GAME + 1,
	CONTROLS = MAIN_MENU + 1,
	NONE = CONTROLS + 1,
	UI_BUTTON_COUNT = NONE + 1
};

enum class FONT_SIZE_ID {
	EXTRA_SMALL = 0,
	SMALL = EXTRA_SMALL + 1,
	MEDIUM = SMALL + 1,
	LARGE = MEDIUM + 1,
	EXTRA_LARGE = LARGE + 1,
	TITLE = EXTRA_LARGE + 1,
	FONT_SIZE_COUNT = TITLE + 1
};
const int font_size_count = (int)FONT_SIZE_ID::FONT_SIZE_COUNT;

struct Message {
	std::string message;
	vec2 position = { 0.f, 0.f };
	FONT_SIZE_ID font_size = FONT_SIZE_ID::SMALL;
};

struct Dialogue {
	std::vector<std::string> dialogue_pages;
	std::string speaker;
	int current_page = 0;
	int current_character = 0;
	float timer_ms = 0.f;
	float timer_ms_per_character = 25.f;
};

struct Alert {
	std::string message;
};

struct Particle {
	vec2 position = { 0.f, 0.f };
	vec2 velocity = { 0.f, 0.f };
	float lifetime = 1000.f;
	float size = 10.f;
	float angle = 0.f;
	vec4 color = { 1.f, 1.f, 1.f, 1.f };
};

struct ParticleSystem {
	TEXTURE_ASSET_ID texture = TEXTURE_ASSET_ID::COIN;
	TEXTURE_ASSET_ID texture_glow = TEXTURE_ASSET_ID::COIN_GLOW;
	vec2 position = { 0.f, 0.f };
	vec2 init_position = { 0.f, 0.f };
	vec2 velocity = { 0.f, 0.f };


	float lifetime = 2000.f;
	float spawn_timeout = 500.f;
	float spawn_rate = 100.f;
	int num_particles = 1;
	float angle = 0.f;
	bool has_spawned = false;
	GLfloat particlePointSize = 32;

	std::vector<Particle> particles;
};
