// internal
#include "render_system.hpp"
#include <SDL.h>
#include <vector>
#include "tiny_ecs_registry.hpp"

void RenderSystem::drawTexturedMesh(RenderRequest& render, Motion& motion, Entity entity,
	const mat3& projection)
{
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(motion.position);
	transform.rotate(motion.angle);
	transform.scale(motion.scale);

	const RenderRequest& render_request = render;

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		GLuint texture_id =
			texture_gl_handles[(GLuint)render_request.used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	} else if (render_request.used_effect == EFFECT_ASSET_ID::ANIMATED)
	{
		if (!registry.animations.has(entity))
		{
			printf("An entity is using the animated effect without an animation entry\n");
			assert(false);
		}

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(AnimatedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(AnimatedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		// printf("Frame: %d\n", registry.animations.get(entity).frame);
		glUniform1i(glGetUniformLocation(program, "frame"), registry.animations.get(entity).frame);
		glUniform1i(glGetUniformLocation(program, "row"), registry.animations.get(entity).row);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::BOSS_ANIMATE)
	{
		if (!registry.animations.has(entity))
		{
			printf("An entity is using the animated effect without an animation entry\n");
			assert(false);
		}

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(BossVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(BossVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();

		// printf("Frame: %d\n", registry.animations.get(entity).frame);
		glUniform1i(glGetUniformLocation(program, "frame"), registry.animations.get(entity).frame);
		glUniform1i(glGetUniformLocation(program, "row"), registry.animations.get(entity).row);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED_GLOW)
	{
		if (!registry.glows.has(entity))
		{
			printf("An entity is using the glow effect without a glow texture entry");
			assert(false);
		}
		
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position


		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		GLuint texture_id =
			texture_gl_handles[(GLuint)render_request.used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
		GLuint glowTexture = texture_gl_handles[(GLuint)registry.glows.get(entity).glowmap];
		// Enable and bind texture slot 1
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, glowTexture);
		gl_has_errors();


		// Set the emissive and colour textures in asset.
		// Code largely generated with ChatGPT
		GLuint emissiveTextureLoc = glGetUniformLocation(program, "samplerGlow");
		GLuint baseColorTextureLoc = glGetUniformLocation(program, "base");

		if (baseColorTextureLoc != -1)
		{
			glUniform1i(baseColorTextureLoc, 0); // Texture Unit 0
		}

		if (emissiveTextureLoc != -1)
		{
			glUniform1i(emissiveTextureLoc, 1); // Texture Unit 1
		}
		gl_has_errors();
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float*)&color);
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float*)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();

	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// https://stackoverflow.com/questions/5839095/sample-texture-in-gl-points-shaders
// https://gamedev.stackexchange.com/questions/27071/fastest-way-to-draw-small-particles-in-opengl
// https://www.khronos.org/opengl/wiki/Primitive
// Also helped with ChatGPT code
void RenderSystem::drawParticleSystem(const mat3& projection, ParticleSystem& particleSystem)
{
	ParticleGLDetails renderDetails = particleBufferPool[particleBufferIndex];
	particleBufferIndex++;
	if (particleBufferIndex >= particleBufferPool.size())
	{
		particleBufferIndex = 0;
	}

	// UPDATE COORDINATES
	GLfloat* coordinates = (GLfloat*)renderDetails.particleBufferData;

	int particleCount = particleSystem.num_particles < MAX_PARTICLES ? particleSystem.num_particles : MAX_PARTICLES;
	for (int i = 0; i < particleCount; i++)
	{
		vec2 particlePos = particleSystem.particles[i].position;
		coordinates[i * 2] = particlePos.x;
		coordinates[i * 2 + 1] = particlePos.y;
	}

	size_t particleSize = BUFFER_SIZE(particleCount);

	glFlushMappedNamedBufferRange(renderDetails.particleVBO, 0, particleSize);

	gl_has_errors();
	// END UPDATE COORDINATES

	const GLuint program = (GLuint)effects[(GLuint)EFFECT_ASSET_ID::PARTICLE];
	glUseProgram(program);

	// SETUP SHADERS
	GLuint projection_loc = glGetUniformLocation(program, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float*)&projection);
	// END SETUP SHADERS


	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(particleSystem.particlePointSize * resolutionScale);

	// BIND BEFORE RENDER
	glBindBuffer(GL_ARRAY_BUFFER, renderDetails.particleVBO);

	GLint posAttrib = glGetAttribLocation(program, "in_position");
	//glEnableVertexAttribArray(posAttrib); // Don't need this
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);

	gl_has_errors();

	// Prep texture info
	// Enabling and binding texture to slot 0
	glActiveTexture(GL_TEXTURE0);
	GLuint texture_id =
		texture_gl_handles[(GLuint)particleSystem.texture];

	glBindTexture(GL_TEXTURE_2D, texture_id);

	GLuint glowTexture = texture_gl_handles[(GLuint)particleSystem.texture_glow];
	// Enable and bind texture slot 1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, glowTexture);
	gl_has_errors();

	GLuint angleLoc = glGetUniformLocation(program, "particleAngle");
	glUniform1f(angleLoc, particleSystem.angle);
	gl_has_errors();

	// Set the emissive and colour textures in asset.
	// Code largely generated with ChatGPT
	GLuint emissiveTextureLoc = glGetUniformLocation(program, "samplerGlow");
	GLuint baseColorTextureLoc = glGetUniformLocation(program, "base");

	if (baseColorTextureLoc != -1)
	{
		glUniform1i(baseColorTextureLoc, 0); // Texture Unit 0
	}

	if (emissiveTextureLoc != -1)
	{
		glUniform1i(emissiveTextureLoc, 1); // Texture Unit 1
	}
	gl_has_errors();

	glDrawArrays(GL_POINTS, 0, particleSystem.num_particles);

	// CLEANUP
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	gl_has_errors();
}

void RenderSystem::blurBloom()
{
	// Bloom https://www.youtube.com/watch?v=um9iCPUGyU4&ab_channel=VictorGordan
	bool horizontal = true, first_iteration = true;
	int amount = 2;
	GLuint bloomEffect = effects[(GLuint)EFFECT_ASSET_ID::BLOOM];
	glUseProgram(bloomEffect);
	for (int i = 0; i < amount; i++)
	{
		// printf("Binding textures\n");
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform1i(glGetUniformLocation(bloomEffect, "horizontal"), horizontal);
		gl_has_errors();

		if (first_iteration)
		{
			// printf("First iteration\n");
			glBindTexture(GL_TEXTURE_2D, off_screen_emissive_buffer);
			first_iteration = false;
		}
		else
		{
			// printf("subsequent iteration\n");
			glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
		}
		gl_has_errors();

		// Draw the screen texture on the quad geometry
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
		glBindBuffer(
			GL_ELEMENT_ARRAY_BUFFER,
			index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
		gl_has_errors();

		glDisable(GL_DEPTH_TEST);

		GLint in_position_loc = glGetAttribLocation(bloomEffect, "inPos");
		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);

		// Draw
		glDrawElements(
			GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
			nullptr);

		horizontal = !horizontal;
	}
	gl_has_errors();
}

// Composite render layers together to create the final image
void RenderSystem::drawToScreen()
{
	blurBloom();
	// Setting shaders
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();

	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();

	// Enabling alpha channel for textures
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	gl_has_errors();

	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];

	// Set the vertex position and vertex texture coordinates
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	gl_has_errors();

	// Bind emissive texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();

	// Bind base color texture in Texture Unit 1
	glActiveTexture(GL_TEXTURE1);
	// Debug functionality that pipes just the emissive buffer without bluring into the shader.
	//glBindTexture(GL_TEXTURE_2D, off_screen_emissive_buffer);
	glBindTexture(GL_TEXTURE_2D, pingpongBuffer[0]);
	gl_has_errors();

	// Set uniforms for emissive and base color textures
	// Code largely generated with ChatGPT
	GLuint emissiveTextureLoc = glGetUniformLocation(water_program, "bloom_texture");
	GLuint baseColorTextureLoc = glGetUniformLocation(water_program, "screen_texture");

	if (baseColorTextureLoc != -1)
	{
		glUniform1i(baseColorTextureLoc, 0); // Texture Unit 0
	}

	if (emissiveTextureLoc != -1)
	{
		glUniform1i(emissiveTextureLoc, 1); // Texture Unit 1
	}

	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr);
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw()
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(0, 0, 0, 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Emissive
	glBindFramebuffer(GL_FRAMEBUFFER, off_screen_emissive_buffer);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
	// and alpha blending, one would have to sort
	// sprites back to front
	gl_has_errors();
	mat3 projection_2D = createProjectionMatrix();
	// Draw all textured meshes that have a position and size component
	for (int layer = (int)RENDER_LAYER_ID::BACKGROUND_FAR; layer < render_layer_count; layer++)
	{
		for (int i = 0, count = registry.renderRequests.components[layer].size(); i < count; i++)
		{
			RenderRequest& renderer = registry.renderRequests.components[layer][i];
			Entity entity = registry.renderRequests.entities[layer][i];
			if (!registry.motions.has(entity))
			{
				continue;
			}
			Motion& motion = registry.motions.get(entity);
			drawTexturedMesh(renderer, motion, entity, projection_2D);
		}
	}

	//renderParticleArrays(projection_2D);
	int particleCount = registry.particleSystems.size() < MAX_PARTICLES_ON_SCREEN ? registry.particleSystems.size() : MAX_PARTICLES_ON_SCREEN;
	for (int i = 0; i < particleCount; i++)
	{
		ParticleSystem& system = registry.particleSystems.components[i];
		if (system.has_spawned) {
			drawParticleSystem(projection_2D, system);
		}
	}
	// Truely render to the screen
	drawToScreen();
	ui_system->render();
	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}

mat3 RenderSystem::createProjectionMatrix()
{
	float left = 0.f;
	float top = 0.f;

	float right = 1920.0f;
	float bottom = 1080.0f;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return { {sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f} };
}
