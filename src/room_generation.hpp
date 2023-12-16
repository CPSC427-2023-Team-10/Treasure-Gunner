#pragma once

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"
#include <render_system.hpp>


void createBoundaryWalls(RenderSystem* renderer);
void createFloor(RenderSystem* renderer, vec2 pos, float width, float height);
Entity createWall(RenderSystem* renderer, vec2 pos, float width, float height, WALL_ID id);
Entity createMovingWall(RenderSystem* renderer, vec2 start_pos, vec2 end_pos, float period_ms, float width, float height);
Entity createDoor(RenderSystem* renderer, int direction, int type);
Entity createRubble(RenderSystem* renderer, int direction);
void openDoor(Entity& entity);
void createBoundaryWall(RenderSystem* renderer, int direction, bool hasDoor);
void createEnemyRoom(RenderSystem* renderer, Entity player, int from);
void createShopRoom(RenderSystem* renderer, Entity player, int from);
void createTutorialRoomOne(RenderSystem* renderer, Entity player);
void createTutorialRoomTwo(RenderSystem* renderer, Entity player);
void createTutorialRoomThree(RenderSystem* renderer, Entity player);
void createTutorialRoomFour(RenderSystem* renderer, Entity player);
void createBossRoom(RenderSystem* renderer, Entity player, int from);
void createLastRoom(RenderSystem* renderer, Entity player, int from, int* doors, int num_items, vec2* positions, ITEM_ID* item_ids, GUN_ID* gun_ids, bool* shatters);