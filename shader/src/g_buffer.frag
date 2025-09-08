#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 fragPos;

layout(set = 1, binding = 0) uniform sampler2D texSampler1;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedoSpec;

struct PointLight {
    vec3 position;
    vec3 color;
};

PointLight light = {vec3(50, 50, 10), vec3(1.0, 0.996, 0.471)};
void main() {
    vec4 color = texture(texSampler1, texCoord);
    if (color.a < 0.1)
        discard;
    gPosition = vec4(fragPos, 1.0f);
    gNormal = vec4(normal, 1.0f);
    gAlbedoSpec.rgb = color.rgb;
    gAlbedoSpec.a = 1.0f;


    // vec4 color = texture(texSampler1, texCoord);
    // if (color.a < 0.1)
    //     discard;
    
    // float weight = max(dot(normalize(light.position - fragPos), normal), 0.0f);
    // vec3 lightColor;
    // if (weight < 0.4)
    //     lightColor = vec3(0.0f);
    // else
    //     lightColor = light.color;
    
    // color *= vec4(vec3(0.1) + lightColor, 1.0f);
    // float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    // vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
    // // Gamma校正 
    // mapped = pow(mapped, vec3(1.0 / gamma));
    // outColor = vec4(mapped, color.a);

    // if(brightness > 1.0f)
    //     outColor1 = outColor;
    
}