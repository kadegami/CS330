#version Karuna W. November 2025

out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;

uniform float lightIntensity; // <-- Added for enhancement

void main()
{
    // Ambient lighting
    float ambientStrength = 0.25;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Apply adjustable light intensity enhancement
    vec3 result = (ambient + diffuse * lightIntensity) * objectColor;

    FragColor = vec4(result, 1.0);
}