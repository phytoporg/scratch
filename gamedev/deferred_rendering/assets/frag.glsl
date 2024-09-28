#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main() 
{
    // Store the fragment position in the gBuffer texture
    gPosition = FragPos;

    // Same with normals
    gNormal = normalize(Normal);

    // Diffuse color in rgb components
    gAlbedoSpec.rgb = texture(texture_diffuse1, TexCoords).rgb;

    // Specular value in alpha component
    gAlbedoSpec.a = texture(texture_specular1, TexCoords).r;
}
