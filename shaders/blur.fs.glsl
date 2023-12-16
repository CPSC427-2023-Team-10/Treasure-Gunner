// adapted from https://www.youtube.com/watch?v=um9iCPUGyU4&ab_channel=VictorGordan
#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D screenTexture;
uniform bool horizontal;

// How far from the center to take samples from the fragment you are currently on
uniform int radius = 8;
// Keep it between 1.0f and 2.0f (the higher this is the further the blur reaches)
// float spreadBlur = 2.0f;
uniform float weights[8] = float[8](
    0.06691601246638082,
    0.06690249649931793,
    0.0668622335396275,
    0.06679530648220221,
    0.0667017897089233,
    0.06658179010260332,
    0.0664354467640494,
    0.06626293067008591
);

void main()
{
    vec2 tex_offset = 1.0f / textureSize(screenTexture, 0);
    vec3 result = texture(screenTexture, texCoords).rgb * weights[0];

    // Calculate horizontal blur
    if(horizontal)
    {
        for(int i = 1; i < radius; i++)
        {
            // Take into account pixels to the right
            result += texture(screenTexture, texCoords + vec2(tex_offset.x * i, 0.0)).rgb * weights[i];
            // Take into account pixels on the left
            result += texture(screenTexture, texCoords - vec2(tex_offset.x * i, 0.0)).rgb * weights[i];
        }
    }
    // Calculate vertical blur
    else
    {
        for(int i = 1; i < radius; i++)
        {
            // Take into account pixels above
            result += texture(screenTexture, texCoords + vec2(0.0, tex_offset.y * i)).rgb * weights[i];
            // Take into account pixels below
            result += texture(screenTexture, texCoords - vec2(0.0, tex_offset.y * i)).rgb * weights[i];
        }
    }
    FragColor = vec4(result, 1.0f);
}
