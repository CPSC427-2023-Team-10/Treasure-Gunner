#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include <random>

// These are hard coded to the dimensions of the entity texture
const float ENEMY_BB_WIDTH = 0.4f * 300.f;
const float ENEMY_BB_HEIGHT = 0.4f * 202.f;

// the gun
Gun createGun(GUN_ID gun_id);
// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);

Entity createEnemy(RenderSystem* renderer, vec2 position, vec2 velocity, ENEMY_ID id, bool summoned);

Entity createBoss(RenderSystem* renderer, vec2 position, vec2 velocity, int coins);

Entity createEasySwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned);

Entity createSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, int maxElite, bool summoned);

//Entity createMegaSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, int maxElite, bool summoned);
Entity createMegaSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int coins, int roomsCleared, bool summoned);

Entity createFlameThrowerSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned);

Entity createZapperSwarm(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned);

void createBombers(RenderSystem* renderer, Entity target, vec2 spawnMin, vec2 spawnMax, int count, bool summoned);

Entity createBullet(vec2 pos, float angle, float speed, bool can_richochet, float max_dist, bool shot_by_player, bool piercing, float damage, float scale, GUN_ID id);

Entity createMovingWall(RenderSystem* renderer, vec2 start_pos, vec2 end_pos, float period_ms, float width, float height);

Entity createParticleSystem(vec2 position, vec2 velocity, int num, float lifetime, float angle, int size, TEXTURE_ASSET_ID tex);

vec2 catmullrom(vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t);

Entity createSplineBullet(vec2 start_pos, vec2 end_pos, float angle, float speed, bool can_richochet, float max_dist, bool shot_by_player, bool shot_by_boss, float damage);

void createItem(RenderSystem* renderer, vec2 pos, ITEM_ID item_id, GUN_ID gun_id, bool shattered);

Gun& equipGun(Entity entity, GUN_ID gun_id);

void swapGun(Entity player, int hotbar_index);
