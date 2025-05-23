#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texture1;

float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

float guassianBlur(int x)
{
    float sigma = 20.0f;
    return pow(2.718281828, - x * x / 2 / sigma / sigma) / 2.506628 / sigma;
}


void main() {
    vec2 texOffset = 1.0f / textureSize(texture1, 0);

    vec3 result;
    result.r = texture(texture1, fragTexCoord + vec2(30 * texOffset.x, 0)).r;
    result.g = texture(texture1, fragTexCoord + vec2(50 * texOffset.x, 0)).g;
    result.b = texture(texture1, fragTexCoord + vec2(80 * texOffset.x, 0)).b;
    result *= guassianBlur(0);
    for (int i = 1; i < 50; ++i)
    {
        // result += texture(texture1, fragTexCoord + vec2(i * texOffset.x, 0)).rgb * guassianBlur(i);
        // result += texture(texture1, fragTexCoord - vec2(i * texOffset.x, 0)).rgb * guassianBlur(i);
        vec3 color;
        float guassianBlurValue = guassianBlur(i);
        color.r = texture(texture1, fragTexCoord + vec2((i + 30) * texOffset.x, 0)).r;
        color.g = texture(texture1, fragTexCoord + vec2((i + 50) * texOffset.x, 0)).g;
        color.b = texture(texture1, fragTexCoord + vec2((i + 80) * texOffset.x, 0)).b;

        result += color * guassianBlurValue;


        result.r += texture(texture1, fragTexCoord - vec2((i + 10) * texOffset.x, 0)).r * guassianBlurValue;
        result.g += texture(texture1, fragTexCoord - vec2((i + 50) * texOffset.x, 0)).g * guassianBlurValue;
        result.b += texture(texture1, fragTexCoord - vec2((i + 80) * texOffset.x, 0)).b * guassianBlurValue;
    }


    outColor = vec4(result, 1.0f);// * vec4(fragColor, 1.0);
}
