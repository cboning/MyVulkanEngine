#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D texture1;


vec4 colorOffset()
{
    float offset = 0.005f;
    return vec4(texture(texture1, fragTexCoord).r, texture(texture1, fragTexCoord + (fragTexCoord * 2 - 1) * offset).g, texture(texture1, fragTexCoord + (fragTexCoord * 2 - 1) * offset * 2).b, 1.0f);
}


mat2 rotationMat2(float theta)
{
    float x = cos(theta);
    float y = sin(theta);
    return mat2(vec2(x, -y), vec2(y, x));
}

vec4 twistTransformation()
{
    int count = 6;
    vec2 texCoord = vec2(fragTexCoord.x, fragTexCoord.y) * 2.0 - 1.0;
    vec2 uv = texCoord;
    
    float distant = sqrt(dot(texCoord, texCoord));
    
    texCoord *= rotationMat2(distant / 3.0);
    

    vec2 normalizedTexCoord = normalize(texCoord);
    mat2 rotationMat = mat2(vec2(normalizedTexCoord.x, -normalizedTexCoord.y), vec2(normalizedTexCoord.y, normalizedTexCoord.x));
    for (int i = 0; i < count - 1; ++i)
        texCoord *= rotationMat;
    
    
    float weight = dot(normalize(texCoord), vec2(1.0f, 0.0f)) * 10.0 - 9.0f;


    vec2 result = weight > 0.0f ? (uv * rotationMat2(-distant / 3.0) * (1.0f + pow(weight, 3.0) * 0.2) * rotationMat2(distant / 3.0) * 0.5 + 0.5) : fragTexCoord;



    return vec4(texture(texture1, result).rgb, 1.0f);
}


void main() {

    outColor = twistTransformation();
}