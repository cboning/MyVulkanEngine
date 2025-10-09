#version 450

layout(location = 0) out vec4 originColor;
layout(location = 1) out vec4 highLightColor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput position;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput normal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput albedoSpec;
layout(input_attachment_index = 3, binding = 3) uniform subpassInput lightSpacPos;
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
    float shadow = projCoords.z - max(0.0001 * (1.0 - dot(normalize(subpassLoad(normal).rgb), normalize(light.position - subpassLoad(position).rgb))), 0.0001) > texture(shadowMap, projCoords.xy * 0.5 + 0.5).r ? 1.0 : 0.0;
    return shadow;
}

void main()
{
    vec3 color = subpassLoad(albedoSpec).rgb;
    vec3 fragPos = subpassLoad(position).rgb;
    vec3 fragNormal = subpassLoad(normal).rgb;

    float weight = 0.0f;
    if (fragNormal != vec3(0.0f))
        weight = max(dot(normalize(light.position - fragPos), normalize(fragNormal)), 0.0f);

    float shadow = shadowCalculation(subpassLoad(lightSpacPos));
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
