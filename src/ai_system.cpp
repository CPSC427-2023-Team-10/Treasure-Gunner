// internal
#include "ai_system.hpp"

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	auto& boid_container = registry.flock;
	for (int i = 0; i < registry.flocks.components.size(); i++) {
		updateBoids(registry.flocks.entities[i], elapsed_ms);
	}
}

//Helper functions
float magnitude(vec2 in) {
	return std::sqrt(in.x * in.x + in.y * in.y);
}

float calculateDistance(vec2 pos1, vec2 pos2) {
	return magnitude(pos1 - pos2);
}

vec2 normalized(vec2 in) {
	float mag = magnitude(in);
	return vec2(in.x / mag, in.y / mag);
}

void AISystem::updateBoids(Entity flock, float elapsed_ms) {
	auto& boids = registry.flocks.get(flock).boids;
	// Remove boids that have been destroyed
	for (int i = 0; i < boids.size(); i++) {
		while (!registry.motions.has(boids[i])) {
			boids.erase(boids.begin() + i);
			if (i >= boids.size()) break;
		}
	}
	// Remove flock if there are no boids left
	if (boids.size() == 0) {
		registry.flocks.remove(flock);
		return;
	}

	for (int i = 0; i < boids.size(); i++) {
		updateBoid(boids[i], boids, registry.flocks.get(flock).target, elapsed_ms);
	}
}

void AISystem::updateBoid(Entity entity, std::vector<Entity>& flock, Entity target, float elapsed_ms) {
	vec2 close = vec2(0, 0);
	vec2 avgVel = vec2(0, 0);
	vec2 avgPos = vec2(0, 0);
	int visualCount = 0;
	Motion& boidMotion = registry.motions.get(entity);
	for (int i = 0; i < flock.size(); i++) {
		if (flock[i] == entity) continue;
		Motion& otherMotion = registry.motions.get(flock[i]);
		vec2 diff = boidMotion.position - otherMotion.position;
		if (magnitude(diff) < separationRadius) {
			// Separation Force
			close += diff;
		} else if (magnitude(diff) < visualRadius) {
			// Alignment Force
			avgVel += otherMotion.velocity;
			// Cohesion Force
			avgPos += otherMotion.position;
			visualCount++;
		}
	}
	if (visualCount > 0) {
		avgVel /= visualCount;
		avgPos /= visualCount;
		boidMotion.velocity += (avgVel - boidMotion.velocity) * alignmentWeight;
			+ (avgPos - boidMotion.position) * cohesionWeight;
	}
	boidMotion.velocity += close * separationWeight;

	// Edge Avoidance
	if (boidMotion.position.x < edgeMargin) {
		boidMotion.velocity.x += turnFactor;
	} else if (boidMotion.position.x > window_width_px - edgeMargin) {
		boidMotion.velocity.x -= turnFactor;
	} else if (boidMotion.position.y < edgeMargin) {
		boidMotion.velocity.y += turnFactor;
	} else if (boidMotion.position.y > window_height_px - edgeMargin) {
		boidMotion.velocity.y -= turnFactor;
	}

	boidMotion.velocity += calculateTargetForce(entity, target);

	float speed = magnitude(boidMotion.velocity);

	if (speed < minSpeed) {
		boidMotion.velocity = normalized(boidMotion.velocity) * minSpeed;
	} else if (speed > maxSpeed) {
		boidMotion.velocity = normalized(boidMotion.velocity) * maxSpeed;
	}

	Motion& targetMotion = registry.motions.get(target);

	if (registry.rotates.has(entity)) {
		// Apply spinning motion separately
		float spinSpeed = 0.0005f; // Set the speed of rotation in radians per millisecond
		float angleChange = spinSpeed * elapsed_ms; // Calculate the angle change in radians

		// Update the angle of the entity to simulate spinning in place
		if (registry.rotates.get(entity).direction) {
			boidMotion.angle += angleChange;
		}
		else {
			boidMotion.angle -= angleChange;
		}
	}
	else {
		boidMotion.angle = std::atan2(targetMotion.position.y - boidMotion.position.y, targetMotion.position.x - boidMotion.position.x);
	}
}

vec2 AISystem::calculateTargetForce(Entity boid, Entity target) {
	Motion& boidMotion = registry.motions.get(boid);
	Motion& targetMotion = registry.motions.get(target);
	vec2 diff = targetMotion.position - boidMotion.position;
	float dist = magnitude(diff);
	return normalized(diff) * (dist - targetRadius) * targetWeight;
}