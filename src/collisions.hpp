#pragma once

#include "components.hpp"
#include "tiny_ecs_registry.hpp"
#include "common.hpp"

enum AXIS {
	X = 0,
	Y = X+1,
	AXIS_COUNT = Y+1
};

const int axis_count = AXIS::AXIS_COUNT;

// Checks whether two entities are colliding using an Axis Aligned Bounding Box
// Less accurate if the collesion mesh is not a box or the entity is rotated
// Returns the minimum overlap and the normal of the overlap for collision resolution
bool collides_AABB(Entity entity1, Entity entity2, float& min_overlap, vec2& collision_normal);

// Checks whether two entities are colliding
// No requirements on the collision mesh or rotation but is more expensive than AABB
bool collides_SAT(Entity entity1, Entity entity2, float& min_overlap, vec2& collision_normal);

// Checks whether two convex polygons are colliding
bool collidesConvexPolygons(std::vector<vec2>& vertices1, std::vector<int>& indices1, std::vector<vec2>& vertices2, std::vector<int>& indices2, float& min_overlap, vec2& collision_normal);

struct CollisionMesh createMeshCollider(Entity entity, std::string path);

// Creates a collision mesh box for the entity
// Mesh_scale is the scale of the collision mesh relative to the scale of the entity
struct CollisionMesh createBoxCollisionMesh(Entity entity, vec2 mesh_scale);

// Creates a collision mesh capsule for the entity
// radius_to_halflenth represnts the ratio of the radius to half the length of the capsule
// A ratio of 1 will create a circle
// circle_segments is the number of segments used to approximate the circles
struct CollisionMesh createCapsuleCollisionMesh(Entity entity, float radius_to_half_length, int circle_segments, AXIS orientation);

// creates an ellipse collision mesh
// if the scale of the entity is equal in x and y directions the ellipse will be a circle
struct CollisionMesh createEllipseCollisionMesh(Entity entity, int circle_segments);

// Creates a collision mesh from the mesh of the entity
struct CollisionMesh createCollisionMeshFromMesh(Entity entity);

// Projects the vertices onto the given axis and returns the minimum and maximum projections
void projectPolygonOntoAxis(std::vector<vec2>& vertices, std::vector<int>& indices, vec2 axis, float& min_proj, float& max_proj);

// Returns the normal of the edge
vec2 getNormal(vec2 edge);

// Returns the minimum value of the vertices in the given axis
float findMin(std::vector<vec2>& vertices, AXIS axis);

// Returns the maximum value of the vertices in the given axis
float findMax(std::vector<vec2>& vertices, AXIS axis);

// Rotates the vertices by the given angle around (0,0)
void rotateVertices(std::vector<vec2>& vertices, float angle);

// Translates the vertices by the given position
void translateVertices(std::vector<vec2>& vertices, vec2 position);

// Refreshes the collision cache entry for the entity if the entity has moved or rotated since the last collision check
void refreshCollisionCache(Entity entity);

// Updates the collision cache entry for the entity
void updateCollisionCache(Entity entity);

// Inserts a new collision cache entry for the entity
void insertCollisionCache(Entity entity);