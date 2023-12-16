#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "collisions.hpp"

// The number of substeps to take when checking for collisions
// A higher number results in more stable collisions, but is less performant
#define COLLISION_SUBSTEPS 5

// A simple physics system that moves rigid bodies and checks for collision
class PhysicsSystem
{
public:
	void step(float elapsed_ms);

	void check_collisions();

	void substep_collisions(std::vector<Entity>& candidates);

	// Resolves a collision by moving the colliding entities apart
	void resolve_collision(Entity entity1, Collision& collision);


	bool valid_collision(Entity entity1, Entity entity2);

	PhysicsSystem()
	{
	}
};