// https://www.youtube.com/watch?v=um9iCPUGyU4&ab_channel=VictorGordan
#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

out vec2 texCoords;

void main()
{
    gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
    texCoords = (inPos.xy + 1) / 2.f;
}
