#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(input_attachment_index = 0, binding = 0) uniform subpassInput originColor;
layout(input_attachment_index = 1, binding = 1) uniform subpassInput bloomColor;

const float gamma = 2.2f;
const float exposure = 1.0f;

void main() {

    vec4 color = vec4(subpassLoad(bloomColor).rgb, 1.0f);
    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    
    vec3 mapped = vec3(1.0) - exp(-color.rgb * exposure);
    // Gamma校正 
    mapped = pow(mapped, vec3(1.0 / gamma));
    outColor = vec4(mapped, color.a);

}