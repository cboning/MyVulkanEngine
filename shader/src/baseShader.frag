#version 450
layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 originColor;
layout(location = 1) out vec4 highLightColor;

layout(binding = 0) uniform sampler2D position;
layout(binding = 1) uniform sampler2D normal;
layout(binding = 2) uniform sampler2D albedoSpec;
layout(binding = 3) uniform sampler2D lightSpacPos;
layout(binding = 4) uniform sampler2D shadowMap;

struct PointLight
{
    vec3 position;
    vec3 color;
};

PointLight light = {vec3(50, 30, 40), vec3(1.0, 0.996, 0.871) * 3};

float shadowCalculation(vec4 fragPosLightSpace)
{
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    float shadow = projCoords.z - max(0.0001 * (1.0 - dot(normalize(texture(normal, inTexCoord).rgb), normalize(light.position - texture(position, inTexCoord).rgb))), 0.0001) >
                           texture(shadowMap, projCoords.xy * 0.5 + 0.5).r
                       ? 1.0
                       : 0.0;
    return shadow;
}

void main()
{
    vec3 color = texture(albedoSpec, inTexCoord).rgb;
    vec3 fragPos = texture(position, inTexCoord).rgb;
    vec3 fragNormal = texture(normal, inTexCoord).rgb;

    float weight = 0.0f;
    if (fragNormal != vec3(0.0f))
        weight = max(dot(normalize(light.position - fragPos), normalize(fragNormal)), 0.0f);

    float shadow = shadowCalculation(texture(lightSpacPos, inTexCoord));
    vec3 lightColor = (1.0 - shadow) * (light.color * weight);
    // if (weight < 0.4)
    //     lightColor = vec3(0.3f);
    // else
    //     lightColor = light.color;

    color *= vec3(0.1) + lightColor;

    originColor = vec4(color, 1.0f);
    // originColor = vec4(vec3(texture(shadowMap, gl_FragCoord.xy / vec2(1600, 1200)).r), 1.0f);
    if (fragNormal == vec3(0.0f))
        originColor = vec4(1.0f);

    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > 1.0f)
        highLightColor = originColor;
}
