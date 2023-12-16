#include "collisions.hpp"
#include "components.hpp"
#include <stdint.h>

bool collides_AABB(Entity entity1, Entity entity2, float& min_overlap, vec2& overlap_normal)
{	
	// Always refresh the collision cache before fetching entries from it
	// In the case that cache needs to reallocate new space any references to the old space will be invalidated
	refreshCollisionCache(entity1);
	refreshCollisionCache(entity2);

	CollisionCacheEntry& entry1 = registry.collisionCache.get(entity1);
	CollisionCacheEntry& entry2 = registry.collisionCache.get(entity2);
	
	float overlap;
	min_overlap = std::numeric_limits<float>::infinity();
	for (int axis = 0; axis < axis_count; axis++) {
		// Check if Entity 1 is behind Entity 2 to in the axis
		overlap = entry1.AABB_max[axis] - entry2.AABB_min[axis];
		if (overlap < 0) {
			return false;
		} else if (overlap < min_overlap) {
			min_overlap = overlap;
			overlap_normal = { 0, 0 };
			overlap_normal[axis] = 1;
		}

		// Check if Entity 1 is ahead of Entity 2 in the axis
		overlap = entry2.AABB_max[axis] - entry1.AABB_min[axis];
		if (overlap < 0) {
			return false;
		} else if (overlap < min_overlap) {
			min_overlap = overlap;
			overlap_normal = {0, 0 };
			overlap_normal[axis] = -1;
		}
	}

	return true;
}

bool collides_SAT(Entity entity1, Entity entity2, float& collision_overlap, vec2& collision_normal)
{	
	// Always refresh the collision cache before fetching entries from it
	// In the case that cache needs to reallocate new space any references to the old space will be invalidated
	refreshCollisionCache(entity1);
	refreshCollisionCache(entity2);

	CollisionCacheEntry& entry1 = registry.collisionCache.get(entity1);
	CollisionMesh& mesh1 = registry.collisionMeshes.get(entity1);
	CollisionCacheEntry& entry2 = registry.collisionCache.get(entity2);
	CollisionMesh& mesh2 = registry.collisionMeshes.get(entity2);

	collision_overlap = -std::numeric_limits<float>::infinity();
	vec2 normal;
	float overlap;
	bool detected = false;

	// For collision meshes with multiple polygons we must check for collisions between all pairs of polygons
	for (int mesh1_poly_count = 0; mesh1_poly_count < mesh1.polygons.size(); mesh1_poly_count++) {
		for (int mesh2_poly_count = 0; mesh2_poly_count < mesh2.polygons.size(); mesh2_poly_count++) {
			if (collidesConvexPolygons(entry1.vertices, mesh1.polygons[mesh1_poly_count], entry2.vertices, mesh2.polygons[mesh2_poly_count], overlap, normal)) {
				detected = true;
				if (overlap > collision_overlap) {
					collision_overlap = overlap;
					collision_normal = normal;
				}
			}
		}
	}
	return detected;
}

bool collidesConvexPolygons(std::vector<vec2>& vertices1, std::vector<int>& indices1, std::vector<vec2>& vertices2, std::vector<int>& indices2, float& min_overlap, vec2& collision_normal)
{	
	vec2 edge, normal;
	float min1, max1, min2, max2;
	float overlap1, overlap2;
	min_overlap = std::numeric_limits<float>::infinity();
	// Loop through all normals of polygon 1
	for (int index1 = 0; index1 < indices1.size(); index1++) {
		edge = vertices1[indices1[(index1 + 1) % indices1.size()]] - vertices1[indices1[index1]];
		normal = getNormal(edge);
		projectPolygonOntoAxis(vertices1, indices1, normal, min1, max1);
		projectPolygonOntoAxis(vertices2, indices2, normal, min2, max2);
		if (min1 > max2 || min2 > max1) {
			// Separating axis found
			return false;
		}
		else {
			overlap1 = max1 - min2;
			overlap2 = max2 - min1;
			if (overlap1 < min_overlap) {
				min_overlap = overlap1;
				collision_normal = normal;
			}
			if (overlap2 < min_overlap) {
				min_overlap = overlap2;
				collision_normal = -normal;
			}
		}
	}
	// Loop through all normals of polygon 2
	for (int index2 = 0; index2 < indices2.size(); index2++) {
		edge = vertices2[indices2[(index2 + 1) % indices2.size()]] - vertices2[indices2[index2]];
		normal = getNormal(edge);
		projectPolygonOntoAxis(vertices1, indices1, normal, min1, max1);
		projectPolygonOntoAxis(vertices2, indices2, normal, min2, max2);
		if (min1 > max2 || min2 > max1) {
			// Separating axis found
			return false;
		}
		else {
			overlap1 = max1 - min2;
			overlap2 = max2 - min1;
			if (overlap1 < min_overlap) {
				min_overlap = overlap1;
				collision_normal = normal;
			}
			if (overlap2 < min_overlap) {
				min_overlap = overlap2;
				collision_normal = -normal;
			}
		}
	}
	return true;
}

void refreshCollisionCache(Entity entity)
{
	if (registry.collisionCache.has(entity)) {
		Motion& motion = registry.motions.get(entity);
		CollisionCacheEntry& entry = registry.collisionCache.get(entity);
		if (entry.position != motion.position || entry.angle != motion.angle) {
			updateCollisionCache(entity);
		}
	}
	else {
		insertCollisionCache(entity);
	}
}

void updateCollisionCache(Entity entity)
{
	assert(registry.collisionCache.has(entity) && "Entity does not have a collision cache entry");
	Motion& motion = registry.motions.get(entity);
	CollisionCacheEntry& entry = registry.collisionCache.get(entity);
	entry.vertices = registry.collisionMeshes.get(entity).vertices;
	rotateVertices(entry.vertices, motion.angle);
	translateVertices(entry.vertices, motion.position);
	entry.AABB_min = vec2(findMin(entry.vertices, AXIS::X), findMin(entry.vertices, AXIS::Y));
	entry.AABB_max = vec2(findMax(entry.vertices, AXIS::X), findMax(entry.vertices, AXIS::Y));
	entry.position = motion.position;
	entry.angle = motion.angle;
}

void insertCollisionCache(Entity entity)
{
	assert(!registry.collisionCache.has(entity) && "Entity already has a collision cache entry");
	Motion& motion = registry.motions.get(entity);
	CollisionCacheEntry& entry = registry.collisionCache.emplace(entity);
	entry.vertices = registry.collisionMeshes.get(entity).vertices;
	rotateVertices(entry.vertices, motion.angle);
	translateVertices(entry.vertices, motion.position);
	entry.AABB_min = vec2(findMin(entry.vertices, AXIS::X), findMin(entry.vertices, AXIS::Y));
	entry.AABB_max = vec2(findMax(entry.vertices, AXIS::X), findMax(entry.vertices, AXIS::Y));
	entry.position = motion.position;
	entry.angle = motion.angle;
}

std::unordered_map<std::string, std::pair<std::vector<ColoredVertex>, std::vector<uint16>>> meshColliderCache;

struct CollisionMesh createMeshCollider(Entity entity, std::string path)
{
	std::vector<ColoredVertex> outVerticies = std::vector<ColoredVertex>();
	std::vector<uint16_t> outVertexIndecies = std::vector<uint16_t>();
	vec2 out_size = vec2();

	if (meshColliderCache.find(path) != meshColliderCache.end())
	{
		std::pair<std::vector<ColoredVertex>, std::vector<uint16>> data = meshColliderCache[path];
		outVerticies = data.first;
		outVertexIndecies = data.second;
	}
	else
	{
		Mesh::loadBlenderFromOBJFile(mesh_path(path), outVerticies, outVertexIndecies, out_size);
		std::pair<std::vector<ColoredVertex>, std::vector<uint16>> data = { outVerticies, outVertexIndecies };
		meshColliderCache[path] = data;
	}

	assert(registry.motions.has(entity));

	Motion& entityMotion = registry.motions.get(entity);

	// printf("Creating collider mesh with %ld verticies and %ld indecies\n", (int)outVerticies.size(), (int)outVertexIndecies.size());
	// printf("Scale is X: %f, Y: %f\n", entityMotion.scale.x, entityMotion.scale.y);

	vec2 scaleMultiplier = entityMotion.scale * vec2(0.5f, 0.5f);
	CollisionMesh cm;
	for (int i = 0; i < outVerticies.size(); i++)
	{
		vec3 source = outVerticies[i].position;
		vec2 vertex = vec2(source.x, source.z);
		cm.vertices.push_back(vertex * scaleMultiplier);

		vec2 scaled = vertex * scaleMultiplier;
		// printf("Vertex from %f, %f now at %f, %f\n", vertex.x, vertex.y, scaled.x, scaled.y);
	}

	for (uint16_t i = 0; i < outVertexIndecies.size(); i += 3)
	{
		std::vector<int> shape = std::vector<int>();
		shape.push_back(outVertexIndecies[i + 0]);
		shape.push_back(outVertexIndecies[i + 1]);
		shape.push_back(outVertexIndecies[i + 2]);

		cm.polygons.push_back(shape);

		// printf("Face between verticies %d, %d, %d\n", shape[0], shape[1], shape[2]);
	}

	return cm;
}

struct CollisionMesh createBoxCollisionMesh(Entity entity, vec2 mesh_scale)
{
	// Get the motion component
	Motion& motion = registry.motions.get(entity);

	// Get the scale
	vec2 scale = mesh_scale * motion.scale;

	// Create the collision mesh
	CollisionMesh mesh;

	// Add the vertices in a counter-clockwise order
	mesh.vertices.push_back({ -scale.x / 2, -scale.y / 2 });
	mesh.vertices.push_back({ scale.x / 2, -scale.y / 2 });
	mesh.vertices.push_back({ scale.x / 2, scale.y / 2 });
	mesh.vertices.push_back({ -scale.x / 2, scale.y / 2 });
	std::vector<int> box = { 0, 1, 2, 3 };
	mesh.polygons.push_back(box);

	return mesh;
}

struct CollisionMesh createCapsuleCollisionMesh(Entity entity, float radius_to_half_length, int circle_segments, AXIS orientation)
{
	// Get the motion component
	Motion& motion = registry.motions.get(entity);
	float half_length;
	if (orientation == AXIS::X) {
		// We use the abs here because the scale can be negative
		half_length = std::abs(motion.scale.x) / 2;
	}
	else {
		// We use the abs here because the scale can be negative
		half_length = std::abs(motion.scale.y) / 2;
	}

	double radius = radius_to_half_length * half_length;
	double rectangle_half_length = half_length - radius;

	float angle_step = M_PI / (circle_segments - 1);
	// Create the collision mesh
	CollisionMesh mesh;

	// Add the vertices in a counter-clockwise order
	if (orientation == AXIS::X) {
		for (int i = 0; i < circle_segments; i++) {
			float angle = angle_step * i;
			mesh.vertices.push_back(vec2(radius * sin(angle) + rectangle_half_length, radius * cos(angle)));
		}
		for (int i = 0; i < circle_segments; i++) {
			float angle = angle_step * i + M_PI;
			mesh.vertices.push_back(vec2(radius * sin(angle) - rectangle_half_length, radius * cos(angle)));
		}
	}
	else {
		for (int i = 0; i < circle_segments; i++) {
			float angle = angle_step * i;
			mesh.vertices.push_back(vec2(radius * cos(angle), radius * sin(angle) + rectangle_half_length));
		}
		for (int i = 0; i < circle_segments; i++) {
			float angle = angle_step * i + M_PI;
			mesh.vertices.push_back(vec2(radius * cos(angle), radius * sin(angle) - rectangle_half_length));
		}
	}
	std::vector<int> capsule;
	for (int i = 0; i < circle_segments * 2; i++) {
		capsule.push_back(i);
	}
	mesh.polygons.push_back(capsule);
	return mesh;
}

struct CollisionMesh createEllipseCollisionMesh(Entity entity, int circle_segments)
{
	// Get the motion component
	Motion& motion = registry.motions.get(entity);
	float half_width = std::abs(motion.scale.x) / 2;
	float half_height = std::abs(motion.scale.y) / 2;

	float angle_step = 2 * M_PI / (circle_segments);
	// Create the collision mesh
	CollisionMesh mesh;

	// Add the vertices in a counter-clockwise order
	for (int i = 0; i < circle_segments; i++) {
		float angle = angle_step * i;
		mesh.vertices.push_back(vec2(half_width * cos(angle), half_height * sin(angle)));
	}
	std::vector<int> ellipse;
	for (int i = 0; i < circle_segments; i++) {
		ellipse.push_back(i);
	}
	mesh.polygons.push_back(ellipse);
	return mesh;
}

struct CollisionMesh createCollisionMeshFromMesh(Entity entity)
{
	Mesh* mesh = registry.meshPtrs.get(entity);

	// Get the motion component
	Motion& motion = registry.motions.get(entity);
	// Create the collision mesh
	CollisionMesh collision_mesh;

	// Add the vertices in a counter-clockwise order
	for (int i = 0; i < mesh->vertices.size(); i++) {
		vec2 vertex = mesh->vertices[i].position;
		collision_mesh.vertices.push_back(vertex * motion.scale);
	}
	// The mesh is defined by triangles
	for (int i = 0; i < mesh->vertex_indices.size(); i += 3) {
		std::vector<int> triangle;
		triangle.push_back(mesh->vertex_indices[i]);
		triangle.push_back(mesh->vertex_indices[i + 1]);
		triangle.push_back(mesh->vertex_indices[i + 2]);
		collision_mesh.polygons.push_back(triangle);
	}

	return collision_mesh;
}

void projectPolygonOntoAxis(std::vector<vec2>& vertices, std::vector<int>& indices, vec2 axis, float& min_proj, float& max_proj) {
	max_proj = dot(vertices[indices[0]], axis);
	min_proj = max_proj;
	for (int i = 1; i < indices.size(); i++) {
		float proj = dot(vertices[indices[i]], axis);
		if (proj > max_proj) {
			max_proj = proj;
		}
		else if (proj < min_proj) {
			min_proj = proj;
		}
	}
}

vec2 getNormal(vec2 edge)
{	
	float length = std::sqrt(edge.x * edge.x + edge.y * edge.y);
	return vec2(-edge.y, edge.x) / length;
}

float findMin(std::vector<vec2>& vertices, AXIS axis)
{
	float min = vertices[0][axis];
	for (int i = 1; i < vertices.size(); i++) {
		if (vertices[i][axis] < min) {
			min = vertices[i][axis];
		}
	}
	return min;
}

float findMax(std::vector<vec2>& vertices, AXIS axis)
{
	float max = vertices[0][axis];
	for (int i = 1; i < vertices.size(); i++) {
		if (vertices[i][axis] > max) {
			max = vertices[i][axis];
		}
	}
	return max;
}

void rotateVertices(std::vector<vec2>& vertices, float angle)
{
	for (int i = 0; i < vertices.size(); i++) {
		float x = vertices[i].x;
		float y = vertices[i].y;
		vertices[i].x = x * cos(angle) - y * sin(angle);
		vertices[i].y = x * sin(angle) + y * cos(angle);
	}
}

void translateVertices(std::vector<vec2>& vertices, vec2 postition) {
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i] += postition;
	}
}