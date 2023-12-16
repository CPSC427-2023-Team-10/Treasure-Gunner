#include "room_generation.hpp"
#include <world_init.hpp>
#include "tiny_ecs_registry.hpp"
#include "collisions.hpp"

void createBoundaryWalls(RenderSystem* renderer)
{
	createBoundaryWall(renderer, 0, 1);
	createBoundaryWall(renderer, 1, 1);
	createBoundaryWall(renderer, 2, 1);
	createBoundaryWall(renderer, 3, 1);
}

void createFloor(RenderSystem* renderer, vec2 pos, float width, float height)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(width, height);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FLOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		(unsigned int)RENDER_LAYER_ID::BACKGROUND_FAR);
}

Entity createDoor(RenderSystem* renderer, int direction, int type)
{
	vec2 pos = { 0,0 };
	float width = 100;
	float height = 100;

	switch (direction) {
		case 0:
			pos = { (float)window_width_px / 2, 50 };
			break;
		case 1:
			pos = { (float)window_width_px - 50, (float)window_height_px / 2 };
			break;
		case 2:
			pos = { (float)window_width_px / 2, (float)window_height_px - 50 };
			break;
		case 3:
			pos = { 50, (float)window_height_px / 2 };
			break;
	}
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ANIMATED_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = M_PI;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(width, height);
	motion.scale.x *= -1;

	Door& door = registry.doors.emplace(entity);
	door.direction = direction;
	door.type = type;

	CollisionMesh collision_mesh = createBoxCollisionMesh(entity, vec2(1, 1));
	collision_mesh.is_solid = true;
	collision_mesh.is_static = true;
	registry.collisionMeshes.insert(entity, collision_mesh);
	Animation& animation = registry.animations.emplace(entity);
	animation.frame = 0;
	animation.num_frames = 4;
	animation.speed = 300.f;
	animation.row = 13;

	switch (type) {
	case 0:
		animation.row = 14;
		break;
	case 1:
		animation.row = 12;
		break;
	case 3:
		animation.row = 12;
		break;
	case 5:
		animation.row = 22; //replace with boss door texture
		break;
	default:
		animation.row = 14;
		break;
	}

	registry.renderRequests.insert(
		entity,
		{   TEXTURE_ASSET_ID::SPRITESHEET,
			EFFECT_ASSET_ID::ANIMATED,
			GEOMETRY_BUFFER_ID::ANIMATED_SPRITE
		},
		(unsigned int)RENDER_LAYER_ID::BACKGROUND_NEAR);

	return entity;
}

Entity createRubble(RenderSystem* renderer, int direction)
{
	vec2 pos = { 0,0 };
	float width = 100;
	float height = 100;

	switch (direction) {
	case 0:
		pos = { (float)window_width_px / 2, 50 };
		break;
	case 1:
		pos = { (float)window_width_px - 50, (float)window_height_px / 2 };
		break;
	case 2:
		pos = { (float)window_width_px / 2, (float)window_height_px - 50 };
		break;
	case 3:
		pos = { 50, (float)window_height_px / 2 };
		break;
	}
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(width, height);

	registry.walls.emplace(entity);

	CollisionMesh collision_mesh = createBoxCollisionMesh(entity, vec2(1, 1));
	collision_mesh.is_solid = true;
	collision_mesh.is_static = true;
	registry.collisionMeshes.insert(entity, collision_mesh);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::OPEN_DOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		(unsigned int)RENDER_LAYER_ID::BACKGROUND_NEAR);

	return entity;
}

void openDoor(Entity& entity) {
		registry.collisionMeshes.get(entity).is_solid = false;
		Animation& animation = registry.animations.get(entity);
		switch (registry.doors.get(entity).type) {
		case 0:
			animation.row = 13;
			break;
		case 1:
			animation.row = 11;
			break;
		case 2:
			animation.row = 13;
			break;
		case 3:
			animation.row = 11;
			break;
		case 4:
			animation.row = 13;
			break;
		case 5:
			animation.row = 21; //replace with boss room
			break;
		}
}

Entity createWall(RenderSystem* renderer, vec2 pos, float width, float height, WALL_ID id)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(width, height);

	// Create a collision mesh for the entity
	// Because the wall is a perfect rectangle we can use a scale of 1
	CollisionMesh collision_mesh = createBoxCollisionMesh(entity, vec2(1, 1));
	collision_mesh.is_solid = true;
	collision_mesh.is_static = true;
	registry.collisionMeshes.insert(entity, collision_mesh);

	TEXTURE_ASSET_ID texture;
	if (id == WALL_ID::HORIZONTAL_DOOR) {
		texture = TEXTURE_ASSET_ID::HORIZONTAL_WALL_DOOR;
	}
	else if (id == WALL_ID::HORIZONTAL_LONG) {
		texture = TEXTURE_ASSET_ID::HORIZONTAL_WALL_LONG;
	}
	else if (id == WALL_ID::VERTICAL_DOOR) {
		texture = TEXTURE_ASSET_ID::VERTICAL_WALL_DOOR;
	}
	else if (id == WALL_ID::VERTICAL_LONG) {
		texture = TEXTURE_ASSET_ID::VERTICAL_WALL_LONG;
	}

	registry.walls.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		},
		(unsigned int)RENDER_LAYER_ID::FOREGROUND);

	return entity;
}

void createBoundaryWall(RenderSystem* renderer, int direction, bool hasDoor) {
	vec2 pos = {0,0};
	float width = 0;
	float height = 0;
	float newWidth = 0;
	float newHeight = 0;
	vec2 newPos1 = { 0,0 };
	vec2 newPos2 = { 0,0 };
	WALL_ID id;

	switch (direction) {
		case 0:
			// South wallS
			pos = { (float)window_width_px / 2, 50 };
			width = (float)window_width_px;
			height = 100;
			newWidth = width/2 -50;
			newHeight = 100;
			newPos1 = { pos.x / 2 - 25, pos.y };
			newPos2 = { pos.x + pos.x / 2 + 25, pos.y };
			id = WALL_ID::HORIZONTAL_LONG;
			break;
		case 1: 
			// East wall
			pos = { (float)window_width_px - 50, (float)window_height_px / 2 };
			width = 100;
			height = (float)window_height_px;
			newWidth = 100;
			newHeight = height/2 -50;
			newPos1 = { pos.x, pos.y / 2 - 25};
			newPos2 = { pos.x, pos.y + pos.y /2 +25};
			id = WALL_ID::VERTICAL_LONG;
			break;
		case 2: 
			// North wall
			pos = { (float)window_width_px / 2, (float)window_height_px - 50 };
			width = (float)window_width_px;
			height = 100;
			newWidth = width / 2 - 50;
			newHeight = 100;
			newPos1 = { pos.x / 2 - 25, pos.y };
			newPos2 = { pos.x + pos.x / 2 + 25, pos.y };
			id = WALL_ID::HORIZONTAL_LONG;
			break;
		case 3:
			// West wall
			pos = { 50, (float)window_height_px / 2 };
			width = 100;
			height = (float)window_height_px;
			newWidth = 100;
			newHeight = height / 2 - 50;
			newPos1 = { pos.x, pos.y / 2 -25 };
			newPos2 = { pos.x, pos.y + pos.y / 2 + 25};
			id = WALL_ID::VERTICAL_LONG;
			break;
	}

	if (hasDoor) {
		if (direction == 1 || direction == 3) {
			id = WALL_ID::VERTICAL_DOOR;
		}
		else {
			id = WALL_ID::HORIZONTAL_DOOR;
		}
		createWall(renderer, newPos1, newWidth, newHeight, id);
		createWall(renderer, newPos2, newWidth, newHeight, id);
	}
	else {
	// Create a collision mesh for the entity
	// Because the wall is a perfect rectangle we can use a scale of 1
		createWall(renderer, pos, width, height, id);
	}
}

Entity createMovingWall(RenderSystem* renderer, vec2 start_pos, vec2 end_pos, float period_ms, float width, float height) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::ANIMATED_SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.position = start_pos;
	motion.angle = 0.f;
	motion.velocity = { 0.f, 0.f };
	motion.scale = vec2(width, height);

	// Set animation values
	Animation& animation = registry.animations.emplace(entity);
	animation.frame = 0;
	animation.num_frames = 6;
	animation.row = 38;
	animation.speed = 250.f;

	// Create a collision mesh for the entity
	// Because the wall is a perfect rectangle we can use a scale of 1
	CollisionMesh collision_mesh = createBoxCollisionMesh(entity, vec2(1, 1));
	collision_mesh.is_solid = true;
	collision_mesh.is_static = true;
	registry.collisionMeshes.insert(entity, collision_mesh);

	registry.walls.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::SPRITESHEET,
			EFFECT_ASSET_ID::ANIMATED,
			GEOMETRY_BUFFER_ID::ANIMATED_SPRITE
		},
		(unsigned int)RENDER_LAYER_ID::FOREGROUND);

	Interpolation& interpolation = registry.interpolations.emplace(entity);
	interpolation.start_position = start_pos;
	interpolation.end_position = end_pos;
	interpolation.period_ms = period_ms;
	return entity;
}

void createEnemyRoom(RenderSystem* renderer, Entity player, int from) {
	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);

	createBoundaryWalls(renderer);

	int min = 4;
	int max = 8;
	
	Player& playerRef = registry.players.get(player);
	int coins = playerRef.coins;
	int roomsCleared = playerRef.roomsCleared;

	printf("Rooms cleared %d\n", roomsCleared);

	int count = coins / 70;
	if (count < min)
		count = min;
	if (count > max)
		count = max;
	int maxElite = count - 3;

	vec2 spawnMin = {500,500};
	vec2 spawnMax = {700,700 };

	switch (from) {
		case 0:
			spawnMin = {350, 350};
			spawnMax = { window_width_px - 350, window_height_px/2};
			break;
		case 1:
			spawnMin = { window_width_px / 2, 350 };
			spawnMax = { window_width_px -350 , window_height_px-350 };
			break;
		case 2:
			spawnMin = { 350, window_height_px / 2 };
			spawnMax = { window_width_px - 350,  window_height_px - 350 };
			break;
		case 3:
			spawnMin = { 350, 350 };
			spawnMax = { window_width_px / 2 , window_height_px - 350 };
			break;
	}

	if (roomsCleared == 0)
	{
		count = rand() % 2 + 3;
		createEasySwarm(renderer, player, spawnMin, spawnMax, count, false);
	}
	else if (roomsCleared == 1 || (coins < 50 && roomsCleared > 2)) // Guarentees the first room is easy
	{
		//creating normal enemies
		count = 4;
		if (roomsCleared > 1)
		{
			count += rand() % 4;
		}
		maxElite = 2;
		createSwarm(renderer, player, spawnMin, spawnMax, count, maxElite, false);
	}
	else if (roomsCleared == 2) // The second room will have some spice
	{
		//creating normal enemies
		count = 3;
		maxElite = 2;
		createSwarm(renderer, player, spawnMin, spawnMax, count, maxElite, false);
		int enemyType = rand() % 2;
		if (enemyType == 0)
		{
			createFlameThrowerSwarm(renderer, player, spawnMin, spawnMax, 2, false);
		}
		else if (enemyType == 1)
		{
			createBombers(renderer, player, spawnMin, spawnMax, count, false);
		}
	}
	else // Base generation for all other rooms
	{
		// zappers are guarenteed for the 5th room and can appear any time after that
		/*if (roomsCleared == 5)
		{
			createZapperSwarm(renderer, player, spawnMin, spawnMax, 2, false);
		}*/
		//creating all enemies
		/*min = 5;
		max = 8;
		count = coins / 70;
		if (count < min)
			count = min;
		if (count > max)
			count = max;

		maxElite = count - (rand() % 3 + 1);
		if (maxElite > 4) {
			maxElite = 4;
		}*/
		printf("Mega Swarm\n");
		createMegaSwarm(renderer, player, spawnMin, spawnMax, coins, roomsCleared, false);
	}

	vec2 wallStartPositions[5] = { 
		{ (float)window_width_px / 2, 300 } ,
		{(float)window_width_px / 4, 300 },
		{ (float)window_width_px / 4, 700 },
		{ (float)window_width_px / 4, 700 },
		{ (float)window_width_px - (float)window_width_px / 4, 700 }
	};
	vec2 wallEndPositions[5] = { 
		{ (float)window_width_px / 2, 500 } 
	, {(float)window_width_px - (float)window_width_px / 4, 300 },
	{ (float)window_width_px - (float)window_width_px / 4, 700 },
		{ (float)window_width_px / 4, 300 },
		{ (float)window_width_px - (float)window_width_px / 4, 300 }
	};
	float wallTime[5] = {
		4000,
		8000,
		8000,
		6000,
		6000
	};

	if (roomsCleared > 2)
	{
		if (rand() % 3 == 1) {
			int pos = rand() % 5;
			createMovingWall(renderer, wallStartPositions[pos], wallEndPositions[pos], wallTime[pos], 100, 100);
		}
	}

	bool createBossDoor = false;
	Player& playerComponent = registry.players.components[0];

	if (playerComponent.roomsCleared >= 10) {
		createBossDoor = true;
	}

	for (int i = 0; i < 4; i++) {
		if (i != (from + 2) % 4)
			if (createBossDoor) {
				createDoor(renderer, i, 5);
				createBossDoor = false;
			}
			else {
				createDoor(renderer, i, rand() % 3 % 2);
			}
	}
}

void createLastRoom(RenderSystem* renderer, Entity player, int from, int* doors, int num_items, vec2* positions, ITEM_ID* item_ids, GUN_ID* gun_ids, bool* shatters) {
	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);
	createBoundaryWalls(renderer);

	for (int i = 0; i < 6; i=i+2) {
		Entity door = createDoor(renderer, doors[i], doors[i+1]);
		openDoor(door);
	}
	createRubble(renderer, (from + 2) % 4);
	for (auto& entity : registry.walls.entities)
	{
		if (registry.renderRequests.has(entity)) {
			if (registry.renderRequests.get(entity).used_texture == TEXTURE_ASSET_ID::OPEN_DOOR)
				registry.renderRequests.get(entity).used_texture = TEXTURE_ASSET_ID::CLOSED_DOOR;
		}

	}

	for (int i = 0; i < num_items; i++) {
		createItem(renderer, positions[i], item_ids[i], gun_ids[i], shatters[i]);
	}
}

void createShopRoom(RenderSystem* renderer, Entity player, int from) {

	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);

	createBoundaryWalls(renderer);
	bool createBossDoor = false;
	Player& playerComponent = registry.players.components[0];

	if (playerComponent.roomsCleared >= 10) {
		createBossDoor = true;
	}

	for (int i = 0; i < 4; i++) {
		if (i != (from + 2) % 4) {
			if (createBossDoor) {
				createDoor(renderer, i, 5);
				createBossDoor = false;
			}
			else {
				Entity door = createDoor(renderer, i, 0);
			}
		}
	}

	createItem(renderer, { (float)window_width_px / 2 - 350.f, (float)window_height_px / 2 }, ITEM_ID::HEALTH, GUN_ID::NO_GUN, false);
	createItem(renderer, { (float)window_width_px / 2, (float)window_height_px / 2 }, ITEM_ID::RANGE, GUN_ID::NO_GUN, false);

	std::vector<GUN_ID> ids;

	if (!registry.hasBounces.has(player)) {
		ids.push_back(GUN_ID::BOUNCING_SHOT);
	}
	if (!registry.hasSplits.has(player)) {
		ids.push_back(GUN_ID::SPLIT_SHOT);
	}
	if (!registry.hasLongs.has(player)) {
		ids.push_back(GUN_ID::LONG_SHOT);
	}
	if (!registry.hasRapids.has(player)) {
		ids.push_back(GUN_ID::RAPID_SHOT);
	}

	ITEM_ID item_id;
	GUN_ID gun_id;

	if (ids.size() == 0) {
		item_id = ITEM_ID::RANGE;
		gun_id = GUN_ID::NO_GUN;
	}
	else {
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<int> distribution(0, ids.size() - 1);
		int random_number = distribution(gen);

		item_id = ITEM_ID::GUN;
		gun_id = ids[random_number];
	}

	createItem(renderer, { (float)window_width_px / 2 + 350.f, (float)window_height_px / 2 }, item_id, gun_id, false);
}

void createTutorialRoomOne(RenderSystem* renderer, Entity player) {
	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);
	createBoundaryWall(renderer, 0, 0);
	createBoundaryWall(renderer, 1, 1);
	createBoundaryWall(renderer, 2, 0);
	createBoundaryWall(renderer, 3, 0);
	Entity message = Entity();
	Motion& motion = registry.motions.emplace(message);
	motion.position = vec2(window_width_px / 2, window_height_px / 6);
	motion.scale = { 436, 64 };
	Entity door = createDoor(renderer, 1, 2);
	openDoor(door);
	Entity dialogue = Entity();
	registry.dialogues.insert(dialogue, 
		{ {"Use W,A,S,D to move around.\nUse your mouse to aim.\nUse left-click to shoot and right-click while moving to dodge.\nProceed to the next room by going through the door.", }, "Adam", 0 });
}

void createTutorialRoomTwo(RenderSystem* renderer, Entity player) {
	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);
	createBoundaryWall(renderer, 0, 0);
	createBoundaryWall(renderer, 1, 1);
	createBoundaryWall(renderer, 2, 0);
	createBoundaryWall(renderer, 3, 1);
	Entity message = Entity();
	Motion& motion = registry.motions.emplace(message);
	motion.position = vec2(window_width_px / 2, window_height_px / 6);
	motion.scale = { 436, 64 };
	createEnemy(renderer, { window_width_px / 2, window_height_px / 2 }, { 0,0 }, ENEMY_ID::DUMMY, false);
	createDoor(renderer, 1, 3);
	Entity dialogue = Entity();
	registry.dialogues.insert(dialogue,
		{ {"Enemies drop coins upon defeat.\nClear all enemies in the room to open the door.\nCoins are automatically picked up.\nHaving more coins attracts more and stronger enemies.", }, "Adam", 0 });
}

void createTutorialRoomThree(RenderSystem* renderer, Entity player) {
	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);
	createBoundaryWall(renderer, 0, 0);
	createBoundaryWall(renderer, 1, 1);
	createBoundaryWall(renderer, 2, 0);
	createBoundaryWall(renderer, 3, 1);
	Entity message = Entity();
	Motion& motion = registry.motions.emplace(message);
	motion.position = vec2(window_width_px / 2, window_height_px / 6);
	motion.scale = { 436, 64 };
	createItem(renderer, { (float)window_width_px / 2, (float)window_height_px / 2 }, ITEM_ID::HEALTH, GUN_ID::NO_GUN, false);
	Entity door = createDoor(renderer, 1, 4);
	openDoor(door);
	Entity dialogue = Entity();
	registry.dialogues.insert(dialogue,
		{ {"This is a shop.\nShoot at jars to buy items.\nWalk to the item to pick it up.\nUse 1-6 or mouse scroll to navigate your inventory.", }, "Adam", 0 });

}

void createTutorialRoomFour(RenderSystem* renderer, Entity player) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(1, 5);
	int random_number = distribution(gen);

	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);
	Entity message = Entity();
	Motion& motion = registry.motions.emplace(message);
	motion.position = vec2(window_width_px / 2, window_height_px / 6);
	motion.scale = { 436, 64 };
	createBoundaryWalls(renderer);
	for (int i = 0; i < 4; i++) {
		if (i != (1 + 2) % 4)
			createDoor(renderer, i, 0);
	};
	Entity dialogue = Entity();
	registry.dialogues.insert(dialogue,
		{ {"Go through a door to start your exploration.\nPress ESC at any time to pause the game.\nGood Luck!", }, "Adam", 0 });

}

void createBossRoom(RenderSystem* renderer, Entity player, int from) {
	createFloor(renderer, { window_width_px / 2, window_height_px / 2 }, window_width_px, window_height_px);

	for (int i = 0; i < 4; i++) {
		if (i==(from+2)%4)
			createBoundaryWall(renderer, i, 1);
		else
			createBoundaryWall(renderer, i, 0);
	}

	auto entity = createBoss(renderer, vec2(window_width_px - 500.f, window_height_px / 2), vec2(0.f, 0.f), registry.players.get(player).coins);
	
	
}