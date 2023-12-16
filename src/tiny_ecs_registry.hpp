#pragma once
#include <vector>

#include "tiny_ecs.hpp"
#include "components.hpp"

class ECSRegistry
{
	// Callbacks to remove a particular or all entities in the system
	std::vector<ContainerInterface*> registry_list;

public:
	// Manually created list of all components this game has
	ComponentContainer<Motion> motions;
	ComponentContainer<Collision> collisions;
	ComponentContainer<Player> players;
	ComponentContainer<Mesh*> meshPtrs;
	BucketedComponentContainer<RenderRequest, render_layer_count> renderRequests;
	ComponentContainer<Glows> glows;
	ComponentContainer<ScreenState> screenStates;
	ComponentContainer<DebugComponent> debugComponents;
	ComponentContainer<vec3> colors;
	ComponentContainer<Wall> walls;
	ComponentContainer<Boid> flock;
	ComponentContainer<Gun> guns;
	ComponentContainer<Projectile> projectiles;
	ComponentContainer<CollisionMesh> collisionMeshes;
	ComponentContainer<CollisionCacheEntry> collisionCache;
	ComponentContainer<Interpolation> interpolations;
	ComponentContainer<DodgeTimer> dodgeTimers;
	ComponentContainer<TransitionTimer> transitionTimers;
	ComponentContainer<DeathTimer> deathTimers;
	ComponentContainer<Door> doors;
	ComponentContainer<Acceleration> accelerations;
	ComponentContainer<Enemy> enemies;
	ComponentContainer<Flock> flocks;
	ComponentContainer<Animation> animations;
	ComponentContainer<DamageTimer> damageTimers;
	ComponentContainer<Item> items;
	ComponentContainer<ItemContainer> itemContainers;
	ComponentContainer<HealthItem> healthItems;
	ComponentContainer<GunItem> gunItems;
	ComponentContainer<RangeItem> rangeItems;
	ComponentContainer<HasSplit> hasSplits;
	ComponentContainer<HasBounce> hasBounces;
	ComponentContainer<HasLong> hasLongs;
	ComponentContainer<HasRapid> hasRapids;
	ComponentContainer<NotEnoughCoinsTimer> notEnoughCoinsTimer;
	ComponentContainer<ParticleSystem> particleSystems;
	ComponentContainer<SplineBullet> splineBullets;
	ComponentContainer<Message> messages;
	ComponentContainer<Dialogue> dialogues;
	ComponentContainer<Alert> alerts;
	ComponentContainer<Hotbar> hotbars;
	ComponentContainer<Boss> bosses;
	ComponentContainer<IntroTimer> introTimers;
	ComponentContainer<RoarTimer> roarTimers;
	ComponentContainer<SummonTimer> summonTimers;
	ComponentContainer<Victory> victories;
	ComponentContainer<GunStatus> gunStatuses;
	ComponentContainer<Rotate> rotates;

	// constructor that adds all containers for looping over them
	// IMPORTANT: Don't forget to add any newly added containers!
	ECSRegistry()
	{
		registry_list.push_back(&motions);
		registry_list.push_back(&collisions);
		registry_list.push_back(&players);
		registry_list.push_back(&meshPtrs);
		registry_list.push_back(&renderRequests);
		registry_list.push_back(&glows);
		registry_list.push_back(&screenStates);
		registry_list.push_back(&debugComponents);
		registry_list.push_back(&colors);
		registry_list.push_back(&walls);
		registry_list.push_back(&flock);
		registry_list.push_back(&guns);
		registry_list.push_back(&collisionMeshes);
		registry_list.push_back(&projectiles);
		registry_list.push_back(&interpolations);
		registry_list.push_back(&dodgeTimers);
		registry_list.push_back(&transitionTimers);
		registry_list.push_back(&accelerations);
		registry_list.push_back(&collisionCache);
		registry_list.push_back(&enemies);
		registry_list.push_back(&doors);
		registry_list.push_back(&flocks);
		registry_list.push_back(&particleSystems);
		registry_list.push_back(&animations);
		registry_list.push_back(&damageTimers);
		registry_list.push_back(&items);
		registry_list.push_back(&itemContainers);
		registry_list.push_back(&healthItems);
		registry_list.push_back(&gunItems);
		registry_list.push_back(&rangeItems);
		registry_list.push_back(&hasSplits);
		registry_list.push_back(&hasBounces);
		registry_list.push_back(&hasLongs);
		registry_list.push_back(&hasRapids);
		registry_list.push_back(&notEnoughCoinsTimer);
		registry_list.push_back(&splineBullets);
		registry_list.push_back(&messages);
		registry_list.push_back(&deathTimers);
		registry_list.push_back(&dialogues);
		registry_list.push_back(&hotbars);
		registry_list.push_back(&bosses);
		registry_list.push_back(&introTimers);
		registry_list.push_back(&roarTimers);
		registry_list.push_back(&summonTimers);
		registry_list.push_back(&victories);
		registry_list.push_back(&alerts);
		registry_list.push_back(&gunStatuses);
		registry_list.push_back(&rotates);
	}

	void clear_all_components() {
		for (ContainerInterface* reg : registry_list)
			reg->clear();
	}

	void list_all_components() {
		printf("Debug info on all registry entries:\n");
		for (ContainerInterface* reg : registry_list)
			if (reg->size() > 0)
				printf("%4d components of type %s\n", (int)reg->size(), typeid(*reg).name());
	}

	void list_all_components_of(Entity e) {
		printf("Debug info on components of entity %u:\n", (unsigned int)e);
		for (ContainerInterface* reg : registry_list)
			if (reg->has(e))
				printf("type %s\n", typeid(*reg).name());
	}

	void remove_all_components_of(Entity e) {
		for (ContainerInterface* reg : registry_list)
			reg->remove(e);
	}
};

extern ECSRegistry registry;