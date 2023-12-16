#version 330

// Input attributes
layout(location = 0) in vec2 in_position;

out vec3 vcolor;

// Application data
uniform mat3 transform;
uniform mat3 projection;

void main() {
	vec3 pos = projection * vec3(in_position, 1.0);
	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
