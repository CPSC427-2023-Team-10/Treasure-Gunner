#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// Output color
layout(location = 0) out vec4 color;
layout(location = 1) out vec4 emissive;

void main()
{
	vec4 mainColor = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
	if (mainColor.a == 0)
	{
		discard;
	}
	else
	{
		color = mainColor;
		emissive = vec4(0, 0, 0, 1);
	}
}
