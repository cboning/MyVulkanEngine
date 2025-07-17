#version 450

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec2 inTexCoord;


layout(location = 0) out vec2 fragTexCoord;

void main()
{
    gl_Position = vec4(iPosition, 1.0f);
    fragTexCoord = inTexCoord;
}
