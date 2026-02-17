#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 highLightColor;

layout(binding = 0) uniform sampler2D texture1;

vec2 getSampleUV(vec2 uv) { return uv * 0.5 + 0.5; }
void main()
{

    vec2 unitUV = fragTexCoord * 2.0 - 1.0;
    vec3 result = vec3(texture(texture1, getSampleUV(unitUV * 0.99)).r, texture(texture1, fragTexCoord).g, texture(texture1, getSampleUV(unitUV * 1.01)).b);
    outColor = vec4(result, 1.0f);

    // float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    // if (brightness > 1.0f)
    //     highLightColor = outColor;
    // else
    highLightColor = vec4(1.0f);
}