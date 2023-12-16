// internal
#include "physics_system.hpp"
#include "world_init.hpp"

#include "world_system.hpp"

// helper function
void add_entity_no_duplicates(std::vector<Entity>& entities, Entity entity) {
	bool already_added = false;
	for (Entity e : entities) {
		if (e == entity) {
			already_added = true;
			break;
		}
	}
	if (!already_added) entities.push_back(entity);
}

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

bool obstacleCollides(const Motion& motion1, const Motion& motion2)
{
	vec2 box1_half_size = get_bounding_box(motion1) / 2.f;
	vec2 box2_half_size = get_bounding_box(motion2) / 2.f;

	bool overlap_x = (motion1.position.x - box1_half_size.x < motion2.position.x + box2_half_size.x) &&
		(motion1.position.x + box1_half_size.x > motion2.position.x - box2_half_size.x);

	bool overlap_y = (motion1.position.y - box1_half_size.y < motion2.position.y + box2_half_size.y) &&
		(motion1.position.y + box1_half_size.y > motion2.position.y - box2_half_size.y);

	return overlap_x && overlap_y;
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_container = registry.motions;
	for(uint i = 0; i < motion_container.size(); i++)
	{
		// update motion.position based on step_seconds and motion.velocity
		Motion& motion = motion_container.components[i];
		Entity entity = motion_container.entities[i];
		float step_seconds = elapsed_ms / 1000.f;
		if (registry.splineBullets.has(entity)) {
			SplineBullet& spline = registry.splineBullets.get(entity);
			if (spline.distance_covered >= spline.total_distance && !spline.last_point_crossed) {
				int last = spline.points_on_line.size() - 1;
				float angle = std::atan2(spline.points_on_line[last].y - spline.points_on_line[last - 1].y, spline.points_on_line[last].x - spline.points_on_line[last - 1].x);
				motion.angle = angle;
				motion.velocity = vec2(std::cos(angle), std::sin(angle)) * spline.speed;
				spline.last_point_crossed = true;
			}
			if (spline.distance_covered < spline.total_distance) {
				int point_to_use = floor((spline.distance_covered / spline.total_distance) * (spline.points_on_line.size()));
				float angle = std::atan2(spline.points_on_line[point_to_use].y - motion.position.y, spline.points_on_line[point_to_use].x - motion.position.x);
				motion.angle = angle;
				motion.velocity = vec2(std::cos(angle), std::sin(angle)) * spline.speed;	
			}
			vec2 dist = motion.velocity * step_seconds;
			motion.position += dist;
			Projectile& projectile = registry.projectiles.get(entity);
			projectile.distance_travelled += sqrt(pow(dist.x, 2) + pow(dist.y, 2));
			spline.distance_covered += sqrt(pow(dist.x, 2) + pow(dist.y, 2));

			if (projectile.distance_travelled > projectile.max_distance) {
				// Spawn spline shard particle
				Entity particles = createParticleSystem(motion.position, motion.velocity, 10, 250.f, 0.f, 8, TEXTURE_ASSET_ID::SPLINE_BULLET_DEAD);
				ParticleSystem& particleSys = registry.particleSystems.get(particles);
				particleSys.texture_glow = TEXTURE_ASSET_ID::BULLET_DEAD_GLOW;
				particleSys.particlePointSize = 8;

				registry.remove_all_components_of(entity);
			}
		} else if (registry.dodgeTimers.has(entity)) {
			DodgeTimer& dodge = registry.dodgeTimers.get(entity);
			if (dodge.time_left > 0) {
				dodge.time_left -= elapsed_ms;
				motion.position.x = (dodge.finalPosition.x - dodge.initialPosition.x) * (1 - pow(dodge.time_left/dodge.timer_ms, 3)) + dodge.initialPosition.x;
				motion.position.y = (dodge.finalPosition.y - dodge.initialPosition.y) * (1 - pow(dodge.time_left/dodge.timer_ms, 3)) + dodge.initialPosition.y;
			}
			else {
				Player& player = registry.players.get(entity);
				Animation& animation = registry.animations.get(entity);
				if (!player.right && !player.left && !player.up && !player.down) {
					animation.frame = 0;
					animation.row = 0;
					animation.num_frames = 5;
					animation.speed = 200.f;
				}
				else {
					animation.frame = 0;
					animation.row = 1;
					animation.num_frames = 4;
					animation.speed = 200.f;
				}
				registry.dodgeTimers.remove(entity);
			}
		} else if (registry.interpolations.has(entity)) {
			Interpolation& interpolation = registry.interpolations.get(entity);
			float t = (std::sin((2 * M_PI)/interpolation.period_ms * interpolation.timer_ms - M_PI/2) + 1)/2;
			motion.position = interpolation.start_position + t * (interpolation.end_position - interpolation.start_position);
			interpolation.timer_ms += elapsed_ms;

		} else if (registry.players.has(entity)) {
			Player& player = registry.players.get(entity);
			Acceleration& acceleration = registry.accelerations.get(entity);
			// accelerate horizontally
			if (player.left) {
        		motion.velocity.x -= acceleration.acceleration * step_seconds;
    		} 
			if (player.right) {
        		motion.velocity.x += acceleration.acceleration * step_seconds;
    		}
			// decelerate horizontally
        	if (!player.left && !player.right) {
				float sign = motion.velocity.x > 0 ? 1.f : -1.f;
        		motion.velocity.x -= sign * acceleration.deceleration * step_seconds;
        		if (sign * motion.velocity.x < 0) {
            		motion.velocity.x = 0.f;
        		}
			} 
    		// accelerate vertically
			if (player.up) {
				motion.velocity.y -= acceleration.acceleration * step_seconds;
			} 
			if (player.down) {
				motion.velocity.y += acceleration.acceleration * step_seconds;
			}
			// decelerate vertically
			if (!player.up && !player.down) {
				float sign = motion.velocity.y > 0 ? 1.f : -1.f;
				motion.velocity.y -= sign * acceleration.deceleration * step_seconds;
				if (sign * motion.velocity.y < 0) {
					motion.velocity.y = 0.f;
				}
			}
			// limit speed
			float speed = sqrt(motion.velocity.x * motion.velocity.x + motion.velocity.y * motion.velocity.y);
    		if (speed > acceleration.max_speed_acc) {
        		motion.velocity.x *= acceleration.max_speed_acc / speed;
        		motion.velocity.y *= acceleration.max_speed_acc / speed;
    		}
			vec2 dist = motion.velocity * step_seconds;
			motion.position += dist;
	
			// Rotate player to face mouse
			vec2 player_position = registry.motions.get(entity).position;
			vec2 mouse_position = registry.players.get(entity).mouse_position;
			float angle = std::atan2(mouse_position[1]-player_position[1], mouse_position[0]-player_position[0]);
			registry.motions.get(entity).angle = angle;
		} else {
			vec2 dist = motion.velocity * step_seconds;
			motion.position += dist;
			if (registry.projectiles.has(entity)) {
				Projectile& projectile = registry.projectiles.get(entity);
				projectile.distance_travelled += sqrt(pow(dist.x, 2) + pow(dist.y, 2));
				if (projectile.distance_travelled > projectile.max_distance) {
					if (projectile.shot_by_player)
					{
						Entity particles = createParticleSystem(motion.position, motion.velocity, 10, 150.f, 0.f, 8, TEXTURE_ASSET_ID::BULLET_DEAD_BASE);
						ParticleSystem& particleSys = registry.particleSystems.get(particles);
						particleSys.texture_glow = TEXTURE_ASSET_ID::BULLET_DEAD_GLOW;
					}
					else
					{
						if (registry.renderRequests.has(entity))
						{
							RenderRequest& renderRequest = registry.renderRequests.get(entity);
							if (renderRequest.used_texture == TEXTURE_ASSET_ID::ICE_SHARD_BASE)
							{
								// ice fragments
								Entity particles = createParticleSystem(motion.position, motion.velocity, 10, 150.f, 0.f, 8, TEXTURE_ASSET_ID::ENEMY_BULLET_DEAD);
								ParticleSystem& particleSys = registry.particleSystems.get(particles);
								particleSys.texture_glow = TEXTURE_ASSET_ID::BULLET_DEAD_GLOW;
							}
							else if (renderRequest.used_texture == TEXTURE_ASSET_ID::BOLT_BASE)
							{
								// ice fragments
								Entity particles = createParticleSystem(motion.position, motion.velocity, 10, 150.f, 0.f, 8, TEXTURE_ASSET_ID::BULLET_DEAD_ZAPPER);
								ParticleSystem& particleSys = registry.particleSystems.get(particles);
								particleSys.texture_glow = TEXTURE_ASSET_ID::BULLET_DEAD_GLOW;
							}
						}
					}

					registry.remove_all_components_of(entity);
				}
			}
		}
	}
	check_collisions();
	return;
}


// Checks for collisions between all collidable entities
void PhysicsSystem::check_collisions()
{
	auto& collision_mesh_container = registry.collisionMeshes;
	float min_overlap;
	vec2 overlap_normal;
	std::vector<Entity> collided_entities;
	for (int i = 0; i < collision_mesh_container.size(); i++) {
		Entity entity_i = collision_mesh_container.entities[i];

		for (int j = i+1; j < collision_mesh_container.size(); j++) {
			Entity entity_j = collision_mesh_container.entities[j];

			if (!valid_collision(entity_i, entity_j)) continue;

			// Broad phase collision check
			// We can discard 95% of the collision checks by checking a simple bounding box
			if (collides_AABB(entity_i, entity_j, min_overlap, overlap_normal)) {
				// Narrow phase collision check
				if (collides_SAT(entity_i, entity_j, min_overlap, overlap_normal)) {
					// Collision detected
					// Create a collision component and add it to the registry
					Collision collision = Collision(entity_j);
					collision.min_overlap = min_overlap;
					collision.overlap_normal = overlap_normal;
					registry.collisions.insert(entity_i, collision, false);

					resolve_collision(entity_i, collision);

					add_entity_no_duplicates(collided_entities, entity_i);
					add_entity_no_duplicates(collided_entities, entity_j);
				}
			}
		}
	}
	substep_collisions(collided_entities);

	float max_area = 0.f;
}

void PhysicsSystem::substep_collisions(std::vector<Entity>& candidates)
{
	float min_overlap;
	vec2 overlap_normal;

	for (int substep = 0; substep < COLLISION_SUBSTEPS; substep++) {
		std::vector<Entity> new_candidates;
		for (int i = 0; i < candidates.size(); i++) {
			Entity candidate = candidates[i];

			for (int j = 0; j < registry.collisionMeshes.size(); j++) {
				Entity collidable = registry.collisionMeshes.entities[j];
				if (candidate == collidable) continue;

				if (!valid_collision(candidate, collidable)) continue;

				if (collides_AABB(candidate, collidable, min_overlap, overlap_normal)) {
					if (collides_SAT(candidate, collidable, min_overlap, overlap_normal)) {
						// Collision detected
						// Create a collision component and add it to the registry
						Collision collision = Collision(collidable);
						collision.min_overlap = min_overlap;
						collision.overlap_normal = overlap_normal;
						resolve_collision(candidate, collision);

						// Add colliding entites to the new_candidates vector for next iteration
						// Only add the entity if it is not already in the vector
						add_entity_no_duplicates(new_candidates, candidate);
						add_entity_no_duplicates(new_candidates, collidable);
					}
				}
			}
		}
		candidates = new_candidates;
	}
}

void PhysicsSystem::resolve_collision(Entity entity1, Collision& collision) {
	Entity entity2 = collision.other_entity;

	CollisionMesh& mesh1 = registry.collisionMeshes.get(entity1);
	CollisionMesh& mesh2 = registry.collisionMeshes.get(entity2);

	// Handle collisions with solid surfaces
	if (mesh1.is_solid && mesh2.is_solid) {
		if (mesh1.is_static) {
			// Entity 1 is immovable, so we only move entity 2
			Motion& motion = registry.motions.get(entity2);
			motion.position += collision.min_overlap * collision.overlap_normal;
		}
		else if (mesh2.is_static) {
			// Entity 2 is immovable, so we only move entity 1
			Motion& motion = registry.motions.get(entity1);
			motion.position -= collision.min_overlap * collision.overlap_normal;
		}
		else {
			// Both entities are movable so each get moved by one half of the overlap
			Motion& motion1 = registry.motions.get(entity1);
			Motion& motion2 = registry.motions.get(entity2);
			motion1.position -= collision.min_overlap / 2 * collision.overlap_normal;
			motion2.position += collision.min_overlap / 2 * collision.overlap_normal;
		}
	}
}

bool PhysicsSystem::valid_collision(Entity entity1, Entity entity2) {

	// Don't check for collisions between two projectiles
	if (registry.projectiles.has(entity1) && registry.projectiles.has(entity2)) return false;

	CollisionMesh& mesh1 = registry.collisionMeshes.get(entity1);
	CollisionMesh& mesh2 = registry.collisionMeshes.get(entity2);

	// A collision between two static objects does not need to be checked
	// since they will never move and thus a collision would be generated every frame
	if (mesh1.is_static && mesh2.is_static) return false;

	if (registry.projectiles.has(entity1) || registry.projectiles.has(entity2)) {
		Projectile& projectile = registry.projectiles.has(entity1) ? registry.projectiles.get(entity1) : registry.projectiles.get(entity2);
		Entity other_entity = registry.projectiles.has(entity1) ? entity2 : entity1;
		if (registry.players.has(other_entity)) {
			// Discard Projectile Player Collisions between players and their own projectiles or if player dodges
			if (projectile.shot_by_player || registry.dodgeTimers.has(other_entity)) return false;
		}
		else if (registry.enemies.has(other_entity)) {
			// Discard Projectile Enemy Collisions between enemies and projectiles shot by enemies or if enemy is being summoned
			if ((!projectile.shot_by_player && !projectile.explodes) || registry.summonTimers.has(other_entity)) return false;
		}
		else if (registry.bosses.has(other_entity)) {
			// Discard Projectile Boss Collisions between boss and projectiles shot by boss
			if (!projectile.shot_by_player && !projectile.explodes) return false;
		}

		bool hit_enemy = false;
		for (Entity entity : projectile.hit_entities) {
			// Discard Projectile Collisions between projectiles and entities they have already hit
			if (entity == other_entity) {
				hit_enemy = true;
				break;
			}
		}
		if (hit_enemy) return false;
	}

	return true;
}