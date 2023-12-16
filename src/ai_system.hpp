#pragma once

#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// DON'T WORRY ABOUT THIS CLASS UNTIL ASSIGNMENT 3
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class AISystem
{
public:
	void step(float elapsed_ms);

	const float targetRadius = 500.0f;
	const float visualRadius = 150.f;
	const float separationRadius = 75.f;
	const float targetWeight = 0.005f;
	const float alignmentWeight = 0.01f;
	const float cohesionWeight = 0.007f;
	const float separationWeight = 0.05f;
	const float edgeMargin = 200.f;
	const float turnFactor = 3.f;

	float maxSpeed = 300.0f;
	float minSpeed = 100.0f;

private:
	void updateBoids(Entity flock, float elapsed_ms);
	void updateBoid(Entity boid, std::vector<Entity>& flock, Entity Target, float elapsed_ms);
	vec2 calculateTargetForce(Entity boid, Entity target);
};