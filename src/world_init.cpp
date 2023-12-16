#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"
#include "collisions.hpp"
#include <numeric>


Gun createGun(GUN_ID gun_id) {
	Gun gun;
	gun.gun_id = gun_id;
	if (gun_id == GUN_ID::STRAIGHT_SHOT) {
		gun.cooldown_ms = 2000.f;
	}
	else if (gun_id == GUN_ID::SPLIT_SHOT) {
		gun.cooldown_ms = 400.f;
	}
	else if (gun_id == GUN_ID::SPLINE_SHOT) {
		gun.cooldown_ms = 2000.f;
	}
	else if (gun_id == GUN_ID::SPLIT_SPLINE_SHOT) {
		gun.cooldown_ms = 2000.f;
	}
	else if (gun_id == GUN_ID::RAPID_SHOT) {
		gun.cooldown_ms = 100.f;
	}
	else if (gun_id == GUN_ID::LONG_SHOT) {
		gun.cooldown_ms = 1500.f;
	}
	else if (gun_id == GUN_ID::OMNI_SHOT) {
		gun.cooldown_ms = 2000.f;
	}
	else if (gun_id == GUN_ID::ZAP_SHOT) {
		gun.cooldown_ms = 1500.f;
	}
	else if (gun_id == GUN_ID::FIRE_SHOT) {
		gun.cooldown_ms = 200.f;
	}
	else {
		gun.cooldown_ms = 300.f;
	}
	return gun;
}

Gun& equipGun(Entity entity, GUN_ID gun_id) {
	Gun gun = createGun(gun_id);
	if (gun_id == GUN_ID::STRAIGHT_SHOT) {
		if (registry.players.has(entity)) {
			gun.cooldown_ms = 300.f;
		}
		else {
			gun.cooldown_ms = 2000.f;
		}
	}
	return registry.guns.insert(entity, gun);
}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// // Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ANIMATED_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = mesh.original_size * 150.f;
	printf("Mesh original size is %f, %f\n", mesh.original_size.x, mesh.original_size.y);
	// motion.scale.y *= -1; // point front to the right

	Animation& animation = registry.animations.emplace(entity);
	animation.frame = 0;
	animation.row = 0;
	animation.num_frames = 5;
	animation.speed = 200.f;

	Acceleration& acceleration = registry.accelerations.emplace(entity);

	// Create a collision mesh for the entity
	// CollisionMesh collision_mesh = createEllipseCollisionMesh(entity, 10);
	CollisionMesh collision_mesh = createMeshCollider(entity, "CharacterCollider-Scaled.obj");
	//CollisionMesh collision_mesh = createCollisionMeshFromMesh(entity, )
	collision_mesh.is_solid = true;
	collision_mesh.is_static = false;
	registry.collisionMeshes.insert(entity, collision_mesh);

	// Create and (empty) Salmon component to be able to refer to all turtles
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{  TEXTURE_ASSET_ID::SPRITESHEET,
			EFFECT_ASSET_ID::ANIMATED,
			GEOMETRY_BUFFER_ID::ANIMATED_SPRITE}, // TEXTURE_COUNT indicates that no txture is needed
		(unsigned int)RENDER_LAYER_ID::MIDGROUND_NEAR);

	equipGun(entity, GUN_ID::STRAIGHT_SHOT);
	Hotbar& hotbar = registry.hotbars.emplace(entity);
	hotbar.selected = 0;
	hotbar.guns[0] = GUN_ID::STRAIGHT_SHOT;
	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position, vec2 velocity, ENEMY_ID id, bool summoned)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ANIMATED_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = velocity;
	motion.position = position;
	motion.angle = M_PI;

	Animation& animation = registry.animations.emplace(entity);
	animation.frame = 0;

	Enemy& enemy = registry.enemies.emplace(entity);
	enemy.id = id;

	// This doesn't have entity specific scaling, it's weird but I'm leaving it as an error catcher.
	CollisionMesh collision_mesh = createEllipseCollisionMesh(entity, 10);

	if (id == ENEMY_ID::NORMAL) {
		motion.scale = mesh.original_size * 70.f;
		animation.row = 4;
		animation.num_frames = 4;
		animation.speed = 100.f;

		enemy.health = 30.f;
		enemy.max_health = 30.f;
		enemy.coin_drop = 25.f;

		collision_mesh = createMeshCollider(entity, "EnemyNeutral-Scaled.obj");
		collision_mesh.is_solid = true;
		collision_mesh.is_static = false;
	} else if (id == ENEMY_ID::ELITE) {
		motion.scale = mesh.original_size * 120.f;
		animation.row = 15;
		animation.num_frames = 4;
		animation.speed = 100.f;

		enemy.health = 90.f;
		enemy.max_health = 90.f;
		enemy.coin_drop = 50.f;

		collision_mesh = createMeshCollider(entity, "EnemyElite.obj");
		collision_mesh.is_solid = true;
		collision_mesh.is_static = false;
	} else if (id == ENEMY_ID::BOMBER) {
		motion.scale = mesh.original_size * 120.f;
		motion.angle = M_PI/2;
		animation.row = 32;
		animation.num_frames = 4;
		animation.speed = 100.f;

		enemy.health = 70.f;
		enemy.max_health = 70.f;
		enemy.coin_drop = 30.f;

		collision_mesh = createMeshCollider(entity, "BombMesh.obj");
		collision_mesh.is_solid = true;
		collision_mesh.is_static = false;
	}
	else if (id == ENEMY_ID::DUMMY) {
		motion.position = position;
		//motion.angle = M_PI / 2;
		motion.angle = -M_PI / 2;
		motion.scale = mesh.original_size * 110.f;
		//motion.scale.y *= -1;
		animation.row = 19;
		animation.num_frames = 1;
		animation.speed = 100.f;

		enemy.health = 100.f;
		enemy.max_health = 100.f;
		enemy.coin_drop = 100.f;

		collision_mesh = createMeshCollider(entity, "DummyCollider-Rotated.obj");
		collision_mesh.is_solid = true;
		collision_mesh.is_static = true;
	}
	else if (id == ENEMY_ID::ZAPPER) {
		motion.scale = mesh.original_size * 90.f;

		float rotation = rand() % 6283; // 2PI * 1000 (because integer)
		motion.angle = rotation / 1000.0f;
		Rotate& rotate = registry.rotates.emplace(entity);
		if (rand() % 100 > 50) {
			rotate.direction = true;
		}
		else {
			rotate.direction = false;
		}

		animation.row = 34;
		animation.num_frames = 6;
		animation.speed = 100.f;

		enemy.health = 100.f;
		enemy.max_health = 100.f;
		enemy.coin_drop = 75.f;

		collision_mesh = createMeshCollider(entity, "ZapperMesh.obj");
		collision_mesh.is_solid = true;
		collision_mesh.is_static = false;
	}
	else if (id == ENEMY_ID::FLAMETHROWER) {
		motion.scale = mesh.original_size * 100.f;
		animation.row = 36;
		animation.num_frames = 2;
		animation.speed = 200.f;

		enemy.health = 40.f;
		enemy.max_health = 40.f;
		enemy.coin_drop = 50.f;

		collision_mesh = createMeshCollider(entity, "FlamethrowerMesh.obj");
		collision_mesh.is_solid = true;
		collision_mesh.is_static = false;
	}

	registry.collisionMeshes.insert(entity, collision_mesh);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPRITESHEET,
		 EFFECT_ASSET_ID::ANIMATED,
		 GEOMETRY_BUFFER_ID::ANIMATED_SPRITE},
		(unsigned int)RENDER_LAYER_ID::MIDGROUND_FAR);

	if (id == ENEMY_ID::NORMAL) {
		equipGun(entity, GUN_ID::STRAIGHT_SHOT).is_firing = true;
	}
	else if (id == ENEMY_ID::ELITE) {
		equipGun(entity, GUN_ID::SPLINE_SHOT).is_firing = true;
	}
	else if (id == ENEMY_ID::ZAPPER) {
		equipGun(entity, GUN_ID::ZAP_SHOT).is_firing = true;
	}
	else if (id == ENEMY_ID::FLAMETHROWER) {
		equipGun(entity, GUN_ID::FIRE_SHOT).is_firing = true;
	}
	else if (id == ENEMY_ID::BOMBER) {
		equipGun(entity, GUN_ID::EXPLOSION_SHOT).is_firing = false;
	}

	if (summoned) {
		animation.row = 31;
		animation.num_frames = 6;
		animation.speed = 200.f;
		registry.guns.get(entity).is_firing = false;
		registry.summonTimers.emplace(entity);
	}
	return entity;
}

Entity createBoss(RenderSystem* renderer, vec2 position, vec2 velocity, int coins)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BOSS_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.velocity = velocity;
	motion.position = position;
	motion.angle = M_PI;
	motion.scale = mesh.original_size * 500.f;

	Animation& animation = registry.animations.emplace(entity);
	animation.frame = 0;
	animation.row = 4;
	animation.num_frames = 4;
	animation.speed = 100.f;

	Boss& boss = registry.bosses.emplace(entity);
	float new_health;
	float new_coins;
	if (coins < 100) {
		new_health = boss.health * (1 + coins / 100);
		new_coins = boss.coin_drop * (1 + coins / 100);
	} else {
		new_health = boss.health * (coins / 100);
		new_coins = boss.coin_drop * (coins / 100);
	}

	boss.health = new_health;
	
	if (boss.health >= boss.max_health) {
		boss.health = boss.max_health;
	}
	if (new_coins <= boss.coin_drop) {
		boss.coin_drop = new_coins;
	}

	// Create a collision mesh for the entity
	// CollisionMesh collision_mesh = createEllipseCollisionMesh(entity, 10);
	CollisionMesh collision_mesh = createMeshCollider(entity, "BossCollider-Triangulated.obj");
	collision_mesh.is_solid = true;
	collision_mesh.is_static = true;

	registry.collisionMeshes.insert(entity, collision_mesh);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOSS,
		 EFFECT_ASSET_ID::BOSS_ANIMATE,
		 GEOMETRY_BUFFER_ID::BOSS_SPRITE },
		(unsigned int)RENDER_LAYER_ID::MIDGROUND_FAR);

	equipGun(entity, GUN_ID::SPLIT_SPLINE_SHOT).is_firing = false;
	boss.max_health = boss.health;

	registry.introTimers.emplace(entity);

	return entity;
}


Entity createSplineBullet(vec2 start_pos, vec2 end_pos, float angle, float speed, bool can_richochet, float max_dist, bool shot_by_player, bool shot_by_boss, float damage)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = start_pos;
	motion.angle = angle;
	motion.velocity = vec2(speed * cos(angle), speed * sin(angle));
	motion.scale = vec2({ 32.f, 32.f });

	// Create a collision mesh for the entity
	CollisionMesh collision_mesh = createEllipseCollisionMesh(entity, 8);
	collision_mesh.is_solid = true;
	collision_mesh.is_static = false;
	registry.collisionMeshes.insert(entity, collision_mesh);

	Projectile& proj = registry.projectiles.emplace(entity);
	proj.can_richochet = false;
	
	proj.shot_by_player = shot_by_player;
	proj.damage = damage;
	TEXTURE_ASSET_ID texture_base_id;
	TEXTURE_ASSET_ID texture_glow_id;

	if (shot_by_player) {
		texture_base_id = TEXTURE_ASSET_ID::BULLET_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::BULLET_GLOW;
	}
	else if (shot_by_boss) {
		texture_base_id = TEXTURE_ASSET_ID::ENERGY_BLAST_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::ENERGY_BLAST_GLOW;
		motion.scale = vec2({ 48.f, 48.f });
	} else {
		texture_base_id = TEXTURE_ASSET_ID::ICE_SHARD_BASE_SPLINE;
		texture_glow_id = TEXTURE_ASSET_ID::ICE_SHARD_GLOW_SPLINE;
	}
	// Create and (empty) Salmon component to be able to refer to all turtles
	registry.renderRequests.insert(
		entity,
		{ texture_base_id,
			EFFECT_ASSET_ID::TEXTURED_GLOW,
			GEOMETRY_BUFFER_ID::SPRITE },
		(unsigned int)RENDER_LAYER_ID::FOREGROUND);

	registry.glows.insert(entity,
		{
			texture_glow_id
		});

	// Create spline movement from start pos to end pos
	SplineBullet& spline = registry.splineBullets.emplace(entity);
	float distance_bw = glm::distance(start_pos, end_pos);
	vec2 direction = glm::normalize(end_pos - start_pos);
	vec2 normal = { -direction.y, direction.x };
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> direction_distribution(-1, 1);
	std::uniform_real_distribution<float> distance_distribution(0.2, 1);
	float random_direction = direction_distribution(gen);
	float random_distance = distance_distribution(gen);
	vec2 control_point_1 = start_pos + (random_distance * distance_bw / 2.f * direction) + (random_direction * distance_bw * normal);
	

	int num_points = 20;
	float t_step = 1.f / (num_points - 1);
	float t = 0.f;
	float distance = 0.f;
	int point_number = 0;
	while (t <= 1.f) {
		vec2 p = catmullrom(start_pos, control_point_1, end_pos, end_pos, t);
		spline.points_on_line.push_back(p);
		t += t_step;
		if (point_number > 0) {
			distance += glm::distance(spline.points_on_line[point_number - 1], spline.points_on_line[point_number]);
		}
		point_number++;
	}

	spline.total_distance = distance;

	return entity;
}

vec2 catmullrom(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
	float t2 = t * t;
	float t3 = t2 * t;

	vec2 v0 = (p2 - p0) * 0.5f;
	vec2 v1 = (p3 - p1) * 0.5f;

	float a1 = 2 * t3 - 3 * t2 + 1;
	float a2 = t3 - 2 * t2 + t;
	float a3 = -2 * t3 + 3 * t2;
	float a4 = t3 - t2;

	return a1 * p1 + a2 * v0 + a3 * p2 + a4 * v1;
}

Entity createEasySwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> xDistribution(spawnMin.x, spawnMax.x);
	std::uniform_real_distribution<float> yDistribution(spawnMin.y, spawnMax.y);
	std::uniform_real_distribution<float> velocityDistribution(-5.f, 5.f);
	auto flock = Entity();
	registry.flocks.emplace(flock);
	auto& flockComponent = registry.flocks.get(flock);
	flockComponent.target = target;

	for (int i = 0; i < count; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, 3);
		int enemy_id = distribution(gen);

		ENEMY_ID id = ENEMY_ID::NORMAL;

		auto entity = createEnemy(renderer, vec2(xDistribution(gen), yDistribution(gen)), vec2(velocityDistribution(gen), velocityDistribution(gen)), id, summoned);
		flockComponent.boids.push_back(entity);
		Gun& gun = registry.guns.get(entity);
		std::uniform_real_distribution<float> gunDistribution(0, gun.cooldown_ms);
		gun.timer_ms = gunDistribution(gen);
	}
	return flock;
}

Entity createSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, int maxElite, bool summoned) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> xDistribution(spawnMin.x, spawnMax.x);
	std::uniform_real_distribution<float> yDistribution(spawnMin.y, spawnMax.y);
	std::uniform_real_distribution<float> velocityDistribution(-5.f, 5.f);
	auto flock = Entity();
	registry.flocks.emplace(flock);
	auto& flockComponent = registry.flocks.get(flock);
	flockComponent.target = target;
	int eliteCount = 0;
	for (int i = 0; i < count; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, 3);
		int enemy_id = distribution(gen);

		ENEMY_ID id = ENEMY_ID::NORMAL;

		if ((enemy_id == 0 && eliteCount < maxElite) || eliteCount == 0) {
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<int> distribution(0, 2);
			int type = distribution(gen);
			id = ENEMY_ID::ELITE;
			eliteCount++;
		}

		auto entity = createEnemy(renderer, vec2(xDistribution(gen), yDistribution(gen)), vec2(velocityDistribution(gen), velocityDistribution(gen)), id, summoned);
		flockComponent.boids.push_back(entity);
		Gun& gun = registry.guns.get(entity);
		std::uniform_real_distribution<float> gunDistribution(0, gun.cooldown_ms);
		gun.timer_ms = gunDistribution(gen);
	}
	return flock;
}

// ChatGPT Weighted Selection
std::size_t weightedSelection(const std::vector<double>& weights) {
	// Calculate the cumulative distribution function (CDF)
	std::vector<double> cdf(weights.size());
	std::partial_sum(weights.begin(), weights.end(), cdf.begin());

	// Generate a random value between 0 and the total weight
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<double> dis(0.0, cdf.back());

	// Perform binary search on the CDF to find the selected index
	auto it = std::upper_bound(cdf.begin(), cdf.end(), dis(gen));
	return std::distance(cdf.begin(), it);
}

// ChatGPT
// Function to normalize a vector of weights
void normalizeWeights(std::vector<double>& weights) {
	double sum = std::accumulate(weights.begin(), weights.end(), 0.0);

	if (sum != 0.0) {
		std::transform(weights.begin(), weights.end(), weights.begin(),
			[sum](double w) { return w / sum; });
	}
}

Entity createMegaSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int coins, int roomsCleared, bool summoned) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> xDistribution(spawnMin.x, spawnMax.x);
	std::uniform_real_distribution<float> yDistribution(spawnMin.y, spawnMax.y);
	std::uniform_real_distribution<float> velocityDistribution(-5.f, 5.f);
	Entity flock = Entity();
	registry.flocks.emplace(flock);
	Flock& flockComponent = registry.flocks.get(flock);
	flockComponent.target = target;

	int denominator = (rand() % 5) + 70;
	int count = coins / denominator;
	if (count < 4) count = 4;
	if (count > 9) count = 9;

	// setup weights here
	// ORDER IS: NORMAL, SPLINE, DUMMY, ZAPPER, FLAMETHROWER, BOMBER
	std::vector<double> weights = std::vector<double>();

	weights.push_back(max(25, 750 - coins)); // NORMAL
	weights.push_back(max(20, 500 - coins)); // SPLINE
	weights.push_back(0.0); // DUMMY

	double zapperWeight = 0;
	if (roomsCleared < 5) zapperWeight = 0;
	if (roomsCleared == 5) zapperWeight = 300;
	if (roomsCleared > 5) zapperWeight = 150;
	weights.push_back(zapperWeight); // ZAPPER

	double flamethrowers = rand() % 3 <= 1 ? 1 : 0;
	double flamethrowerWeight = 200 * flamethrowers;

	weights.push_back(flamethrowerWeight); // FLAMETHROWER

	weights.push_back(min(max(15, coins / 3), 150)); // BOMBER

	normalizeWeights(weights);

	int difficultyBudget = 25 * roomsCleared;
	// ORDER IS: NORMAL, SPLINE, DUMMY, ZAPPER, FLAMETHROWER, BOMBER
	int costs[] = {0,	 5,		 10000, 75,		30,			  35};

	printf("Spawn Weights are:\n");
	for (int i = 0; i < weights.size(); i++)
	{
		printf("  %d: %f\n", i, weights[i]);
	}
	printf("Difficulty Budget: %d\n", difficultyBudget);
	printf("Rooms cleared: %d, coins: %d\n", roomsCleared, coins);

	for (int i = 0; i < count; i++) {
		ENEMY_ID id = ENEMY_ID::NORMAL;

		if (difficultyBudget > 0)
		{
			id = (ENEMY_ID)weightedSelection(weights);

			difficultyBudget -= costs[(int)id];
		}

		Entity entity = createEnemy(renderer, vec2(xDistribution(gen), yDistribution(gen)), vec2(velocityDistribution(gen), velocityDistribution(gen)), id, summoned);
		if (id != ENEMY_ID::BOMBER)
		{
			flockComponent.boids.push_back(entity);
		}
		Gun& gun = registry.guns.get(entity);
		std::uniform_real_distribution<float> gunDistribution(0, gun.cooldown_ms);
		gun.timer_ms = gunDistribution(gen);
	}
	return flock;
}

Entity createFlameThrowerSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> xDistribution(spawnMin.x, spawnMax.x);
	std::uniform_real_distribution<float> yDistribution(spawnMin.y, spawnMax.y);
	std::uniform_real_distribution<float> velocityDistribution(-5.f, 5.f);
	auto flock = Entity();
	registry.flocks.emplace(flock);
	auto& flockComponent = registry.flocks.get(flock);
	flockComponent.target = target;
	Entity output;

	for (int i = 0; i < count; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, 3);
		int enemy_id = distribution(gen);
		ENEMY_ID id = ENEMY_ID::FLAMETHROWER;

		auto entity = createEnemy(renderer, vec2(xDistribution(gen), yDistribution(gen)), vec2(velocityDistribution(gen), velocityDistribution(gen)), id, summoned);
		flockComponent.boids.push_back(entity);
		Gun& gun = registry.guns.get(entity);
		std::uniform_real_distribution<float> gunDistribution(0, gun.cooldown_ms);
		gun.timer_ms = gunDistribution(gen);

		output = entity;
	}

	return flock;
}

Entity createZapperSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> xDistribution(spawnMin.x, spawnMax.x);
	std::uniform_real_distribution<float> yDistribution(spawnMin.y, spawnMax.y);
	std::uniform_real_distribution<float> velocityDistribution(-5.f, 5.f);
	auto flock = Entity();
	registry.flocks.emplace(flock);
	auto& flockComponent = registry.flocks.get(flock);
	flockComponent.target = target;
	for (int i = 0; i < count; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, 3);
		int enemy_id = distribution(gen);
		ENEMY_ID id = ENEMY_ID::ZAPPER;

		auto entity = createEnemy(renderer, vec2(xDistribution(gen), yDistribution(gen)), vec2(velocityDistribution(gen), velocityDistribution(gen)), id, summoned);
		flockComponent.boids.push_back(entity);
		Gun& gun = registry.guns.get(entity);
		std::uniform_real_distribution<float> gunDistribution(0, gun.cooldown_ms);
		gun.timer_ms = gunDistribution(gen);
	}
	return flock;
}

void createBombers(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> xDistribution(spawnMin.x, spawnMax.x);
	std::uniform_real_distribution<float> yDistribution(spawnMin.y, spawnMax.y);
	std::uniform_real_distribution<float> velocityDistribution(-5.f, 5.f);

	for (int i = 0; i < count; i++) {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, 3);
		int enemy_id = distribution(gen);
		ENEMY_ID id = ENEMY_ID::BOMBER;

		auto entity = createEnemy(renderer, vec2(xDistribution(gen), yDistribution(gen)), vec2(velocityDistribution(gen), velocityDistribution(gen)), id, summoned);
	}
}

Entity createBullet(vec2 pos, float angle, float speed, bool can_richochet, float max_dist, bool shot_by_player, bool piercing, float damage, float scale, GUN_ID id)
{
	auto entity = Entity();

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = angle;
	motion.velocity = vec2(speed * cos(angle), speed* sin(angle));
	motion.scale = vec2({32.f, 32.f}) * scale;

	// Create a collision mesh for the entity
	CollisionMesh collision_mesh = createCapsuleCollisionMesh(entity, 0.5, 5, AXIS::X);//createEllipseCollisionMesh(entity, 8);
	collision_mesh.is_solid = true;
	collision_mesh.is_static = false;
	registry.collisionMeshes.insert(entity, collision_mesh);

	Projectile& proj = registry.projectiles.emplace(entity);
	proj.can_richochet = can_richochet;
	proj.max_distance = max_dist;
	proj.shot_by_player = shot_by_player;
	proj.damage = damage;
	proj.piercing = piercing;
	TEXTURE_ASSET_ID texture_base_id;
	TEXTURE_ASSET_ID texture_glow_id;

	if (shot_by_player) {
		texture_base_id = TEXTURE_ASSET_ID::BULLET_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::BULLET_GLOW;
	}
	else if (id == GUN_ID::OMNI_SHOT) {
		texture_base_id = TEXTURE_ASSET_ID::ENERGY_BLAST_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::ENERGY_BLAST_GLOW;
		motion.scale = vec2{ 48.f, 48.f };
	}
	else if (id == GUN_ID::ZAP_SHOT) {
		texture_base_id = TEXTURE_ASSET_ID::BOLT_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::BOLT_GLOW;
		motion.scale = vec2{ 48.f, 48.f };
	}
	else if (id == GUN_ID::FIRE_SHOT) {
		texture_base_id = TEXTURE_ASSET_ID::FIRE_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::FIRE_GLOW;
		motion.scale = vec2{ 48.f, 48.f };
	}
	else if (id == GUN_ID::EXPLOSION_SHOT) {
		texture_base_id = TEXTURE_ASSET_ID::EXPLOSION_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::EXPLOSION_GLOW;
		motion.scale = vec2{ 48.f, 48.f };
		proj.explodes = true;
	}
	else {
		texture_base_id = TEXTURE_ASSET_ID::ICE_SHARD_BASE;
		texture_glow_id = TEXTURE_ASSET_ID::ICE_SHARD_GLOW;
	}
	// Create and (empty) Salmon component to be able to refer to all turtles
	registry.renderRequests.insert(
		entity,
		{ texture_base_id,
			EFFECT_ASSET_ID::TEXTURED_GLOW,
			GEOMETRY_BUFFER_ID::SPRITE},
		(unsigned int)RENDER_LAYER_ID::FOREGROUND);

	registry.glows.insert(entity,
		{
			texture_glow_id
		});

	return entity;

}
Entity createParticleSystem(vec2 position, vec2 velocity, int num, float lifetime, float angle, int size, TEXTURE_ASSET_ID tex) {
	Entity entity = Entity();

	// Setting initial motion values

	ParticleSystem& particleSystem = registry.particleSystems.emplace(entity);
	particleSystem.velocity = velocity;
	particleSystem.init_position = position;
	particleSystem.position = position;
	particleSystem.texture = tex;
	particleSystem.lifetime = lifetime;
	particleSystem.spawn_timeout = 5000.f;
	particleSystem.spawn_rate = particleSystem.spawn_timeout / 10;
	particleSystem.num_particles = num;
	particleSystem.angle = angle;
	particleSystem.particlePointSize = size;
	particleSystem.particles = std::vector<Particle>(num);
	for (int i = 0; i < num; i++) {
		particleSystem.particles[i].position = position;
		particleSystem.particles[i].velocity = velocity;
		particleSystem.particles[i].lifetime = lifetime;
		particleSystem.particles[i].size = 10.f;
		particleSystem.particles[i].angle = 0.f;
		particleSystem.particles[i].color = { 1.f, 1.f, 1.f, 1.f };
	}

	return entity;
}

void createItem(RenderSystem* renderer, vec2 pos, ITEM_ID item_id, GUN_ID gun_id, bool shattered) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ANIMATED_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& motion = registry.motions.emplace(entity);
	motion.angle = M_PI;
	motion.velocity = { 0.f, 0.f };
	motion.position = pos;
	motion.scale = mesh.original_size * 150.f;
	motion.scale.x *= -1;

	auto& item = registry.items.emplace(entity);
	item.item_id = item_id;
	item.gun_id = gun_id;
	item.position = pos;
	item.shattered = shattered;

	auto& animation = registry.animations.emplace(entity);
	animation.frame = 0;
	animation.num_frames = 4;
	animation.speed = 300.f;

	// Initialize animation and item parameters depending on ID
	if (item_id == ITEM_ID::HEALTH) {
		if (shattered) {
			animation.row = 8;
		}
		else {
			animation.row = 7;
			auto& container = registry.itemContainers.emplace(entity);
			container.cost = 100.f;
		}
		registry.healthItems.emplace(entity);
	}
	else if (item_id == ITEM_ID::GUN) {
		if (shattered) {
			if (gun_id == GUN_ID::BOUNCING_SHOT) {
				animation.row = 24;
			}
			else if (gun_id == GUN_ID::SPLIT_SHOT) {
				animation.row = 10;
			}
			else if (gun_id == GUN_ID::LONG_SHOT) {
				animation.row = 28;
			}
			else if (gun_id == GUN_ID::RAPID_SHOT) {
				animation.row = 30;
			}
		}
		else {
			if (gun_id == GUN_ID::BOUNCING_SHOT) {
				animation.row = 23;
			}
			else if (gun_id == GUN_ID::SPLIT_SHOT) {
				animation.row = 9;
			}
			else if (gun_id == GUN_ID::LONG_SHOT) {
				animation.row = 27;
			}
			else if (gun_id == GUN_ID::RAPID_SHOT) {
				animation.row = 29;
			}
			auto& container = registry.itemContainers.emplace(entity);
			container.cost = 500.f;
		}
		
		auto& gun_item = registry.gunItems.emplace(entity);
		gun_item.id = gun_id;
	}
	else if (item_id == ITEM_ID::RANGE) {
		if (shattered) {
			animation.row = 26;
		}
		else {
			animation.row = 25;
			auto& container = registry.itemContainers.emplace(entity);
			container.cost = 100.f;
		}
		registry.rangeItems.emplace(entity);
	}
	else {
		animation.row = 6;
		animation.num_frames = 1;
	}

	// Create a collision mesh for the entity
	CollisionMesh collision_mesh = createEllipseCollisionMesh(entity, 3);
	if (item_id != ITEM_ID::NO_ITEM) {
		collision_mesh.is_solid = true;
		collision_mesh.is_static = true;
		registry.collisionMeshes.insert(entity, collision_mesh);
	}

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPRITESHEET,
		 EFFECT_ASSET_ID::ANIMATED,
		 GEOMETRY_BUFFER_ID::ANIMATED_SPRITE },
		(unsigned int)RENDER_LAYER_ID::MIDGROUND_FAR);
}

void swapGun(Entity player, int hotbar_index) {
	Hotbar& hotbar = registry.hotbars.get(player);
	GUN_ID gun_id = hotbar.guns[hotbar_index];
	int timer_ms = 0;
	bool is_firing = false;

	if (registry.guns.has(player)) {
		// Cooldown is preserved when swapping guns
		timer_ms = registry.guns.get(player).timer_ms;
		is_firing = registry.guns.get(player).is_firing;
	}
	else if (registry.gunStatuses.has(player)) {
		timer_ms = registry.gunStatuses.get(player).timer_ms;
		is_firing = registry.gunStatuses.get(player).is_firing;
		registry.gunStatuses.remove(player);
	}

	// save the status of the current gun
	if (gun_id == GUN_ID::NO_GUN) {
		GunStatus& gun_status = registry.gunStatuses.emplace(player);
		gun_status.is_firing = is_firing;
		gun_status.timer_ms = timer_ms;
	}
	registry.guns.remove(player);
	if (gun_id != GUN_ID::NO_GUN) {
		Gun& gun = equipGun(player, gun_id);
		gun.timer_ms = timer_ms;
		gun.is_firing = is_firing;
		hotbar.selected = hotbar_index;
	}
}

