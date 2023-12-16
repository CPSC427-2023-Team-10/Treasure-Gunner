#version 330

uniform sampler2D screen_texture;
uniform sampler2D bloom_texture;

uniform float screen_darken_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main()
{
    // Sample the color buffer
    vec4 sample_color = texture(screen_texture, texcoord);

    // Sample the emissive buffer
    vec4 emissive = texture(bloom_texture, texcoord);

    // Right now emissive is set to not completely override the source, can be easily changed.
    if (emissive.a == 0)
    {
        color = sample_color;
    }
    else
    {
    	color = sample_color + (emissive * vec4(1.0, 1.0, 1.0, 0.75));
    }
    // Debug view emissive buffer
    //color = emissive;
}
