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

const int MAX_LIGHTS = 32;
uniform Light lights[MAX_LIGHTS];
uniform vec3 viewPos;
uniform int numLights;

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

    // Having trouble passing in light uniforms atm, so hard-coding a light because
    // I'm 30k ft in the air and don't have internet access :(

    // for (int i = 0; i < numLights; ++i)
    // {
    //     // Diffuse
    //     vec3 lightDir = lights[i].Position - FragPos;
    //     vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lights[i].Color;
    //     lighting += diffuse;
    // }

    // Manually use the single point light value that *would* be sent down via
    // uniforms

    const vec3 lightPos = vec3(0.0, 0.0, 4.0);
    const vec3 lightColor    = vec3(1.0, 1.0, 1.0);
    vec3 lightDir = lightPos - FragPos;
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Albedo * lightColor;
    lighting += diffuse;

    FragColor = vec4(lighting, 1.0);
}
