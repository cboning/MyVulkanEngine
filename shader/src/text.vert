#version 450
layout(location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
layout(location = 0) out vec2 TexCoords;

layout(set = 1, binding = 0) uniform UniformBufferObject{ mat4 projective; }
ubo;

void main()
{
    gl_Position = ubo.projective * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}