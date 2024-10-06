#version 330 core
layout (location = 0) out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D diffuse_texture;

void main()
{           
    FragColor = texture(diffuse_texture, TexCoords);
}
