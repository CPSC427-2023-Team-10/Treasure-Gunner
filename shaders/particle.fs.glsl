#version 330 core

uniform sampler2D base;
uniform sampler2D samplerGlow;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 emissive;

uniform float particleAngle;

vec2 rotateTextureCoordinates(vec2 coords, float angle) {
    coords -= 0.5; // 0-1 is the range of the texture coordinates.
    float s = sin(angle);
    float c = cos(angle);
    vec2 rotated = vec2(c * coords.x - s * coords.y, s * coords.x + c * coords.y);
    rotated += 0.5;
    return rotated;
}

void main() {
    float rotationAngle = particleAngle * 3.14159265 / 180.0;

    vec2 rotatedCoords = rotateTextureCoordinates(gl_PointCoord, rotationAngle);

    vec4 mainColor = texture(base, vec2(rotatedCoords.x, 1.0 - rotatedCoords.y));
    if (mainColor.a == 0) {
        discard;
    } else {
        fragColor = mainColor;
        emissive = texture(samplerGlow, rotatedCoords);
    }
}
