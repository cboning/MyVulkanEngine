#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texture1;

float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

float guassianBlur(int x)
{
    float sigma = 10.0f;
    return pow(2.718281828, - x * x / 2 / sigma / sigma) / 2.506628 / sigma;
}

void main() {
    vec2 texOffset = 1.0f / textureSize(texture1, 0);
    vec3 result = texture(texture1, fragTexCoord).rgb * guassianBlur(0);
    for (int i = 1; i < 20; ++i)
    {
        result += texture(texture1, fragTexCoord + vec2(i * texOffset.x, 0)).rgb * guassianBlur(i);
        result += texture(texture1, fragTexCoord - vec2(i * texOffset.x, 0)).rgb * guassianBlur(i);
    }
    outColor = vec4(result, 1.0f);
}