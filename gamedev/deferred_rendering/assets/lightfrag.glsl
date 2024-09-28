#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

struct Light 
{
    vec3 Position;
    vec3 Color;
};

const int LIGHT_COUNT = 32;
uniform Light lights[LIGHT_COUNT];
uniform vec3 viewPos;

void main()
{
    // Retrieve data from g buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    // TODO: more sophisticated lighting models
    vec3 lighting = Albedo * 0.1; // Hard-coded for now
    vec3 viewDir = normalize(viewPos - FragPos);
    for (int i = 0; i < LIGHT_COUNT; ++i)
    {
        // Diffuse
        vec3 lightDir = lights[i].Position - FragPos;
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;
        lighting += diffuse;
    }

    FragColor = vec4(lighting, 1.0);
}
