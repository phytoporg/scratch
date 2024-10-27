#version 330 core
const int MAX_TILES = 4;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 tileMatrices[MAX_TILES];
uniform int tileIndex;

out vec2 TexCoords;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoords = (tileMatrices[tileIndex] * vec3(aTexCoords, 1.0)).xy;
}
