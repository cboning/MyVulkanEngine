#version 450

layout(location = 0) out vec4 originColor;
layout(location = 1) out vec4 hightLightColor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput position;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput normal;
layout(input_attachment_index = 2, binding = 2) uniform subpassInput albedoSpec;

struct PointLight {
    vec3 position;
    vec3 color;
};

const float gamma = 2.2f;
const float exposure = 1.0f;
PointLight light = {vec3(50, 50, 10), vec3(1.0, 0.996, 0.871) * 1.1};

void main() {
    vec3 color = subpassLoad(albedoSpec).rgb;

    vec3 lightColor;
    float weight = max(dot(normalize(light.position - subpassLoad(position).rgb), subpassLoad(normal).rgb), 0.0f);
    if(weight < 0.4)
        lightColor = vec3(0.3f);
    else
        lightColor = light.color;
    
    color *= vec3(0.1) + lightColor;
    originColor = vec4(color, 1.0f);
    
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0f)
        hightLightColor = originColor;
}
