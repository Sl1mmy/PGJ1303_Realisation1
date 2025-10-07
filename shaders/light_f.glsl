#version 330 core

in vec3 v_normal;
in vec3 v_worldPos;
out vec4 fragColor;

#define MAX_LIGHTS 2

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct Light
{
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;

    vec4 dir;
    vec4 pos;

    float linAttenuation;
    float quadAttenuation;
    int type;
    float padding;
};

layout(std140) uniform Lights
{
    vec4 viewDir;
    Light lights[MAX_LIGHTS];
};

vec4 computeLighting(int lightIndex, vec3 normal)
{
    Light light = lights[lightIndex];
    vec3 lightDir = vec3(0, 0, 0);
    float attenuation = 1;

    if(light.type == LIGHT_TYPE_DIRECTIONAL)
    {
        lightDir = light.dir.xyz; // <<< FIXED HERE
    }
    else if(light.type == LIGHT_TYPE_POINT)
    {
        vec3 lightVec = light.pos.xyz - v_worldPos;
        lightDir = normalize(lightVec);
        float lightDist = length(lightVec);
        attenuation = 1.0 / (1 + light.linAttenuation * lightDist +
                             light.quadAttenuation * (lightDist * lightDist));
    }

    float diffuseComp = clamp(dot(normal, lightDir), 0, 1);
    vec3 reflectDir = reflect(-lightDir, normal);
    float specComp = pow(clamp(dot(normalize(viewDir.xyz), reflectDir), 0.0, 1.0), 100.0);

    return attenuation * (light.ambientColor + diffuseComp * light.diffuseColor + specComp * light.specularColor);
}

void main()
{
    vec3 nNormal = normalize(v_normal);
    vec4 finalColor = vec4(0);
    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        finalColor += computeLighting(i, nNormal);
    }

    fragColor = vec4(finalColor.xyz, 1);
}
