#version 450
layout(location = 0) in vec2 TexCoords;
layout(location = 0) out vec4 color;

layout(binding = 1) uniform UniformBufferObject
{
    vec4 color;
    uvec4 rect;
    uvec4 type;
}
ubo;

layout(binding = 2) uniform sampler2D image;

vec4 frame() { return ubo.color; }

vec4 character() { return ubo.color * vec4(1.0, 1.0, 1.0, texture(image, TexCoords).r); }

void main()
{
    uvec2 pos = uvec2(gl_FragCoord.xy);
    // if (pos.x < ubo.rect.x || pos.y < ubo.rect.y || pos.x >= ubo.rect.x + ubo.rect.z || pos.y >= ubo.rect.y + ubo.rect.w)
    //     discard;

    if (ubo.type.x == 0)
        color = character();
    else if (ubo.type.x == 1)
        color = frame();
    else if (ubo.type.x == 2)
        discard;
}
