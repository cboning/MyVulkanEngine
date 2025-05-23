#version 450

layout(location = 0) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;


layout(std140, binding = 0) uniform UniformBufferObject {
    vec3 pos;
    float time;

    vec3 front;
    float padding2;   // 填充

    vec2 screenSize;
    float near;
    float fov;
} ubo;

const uint UI0 = 2394723994U;
const uint UI1 = 3495358349U;
const uint UI2_EXTRA = 2983457348U;

const uvec2 UI2 = uvec2(UI0, UI1);
const uvec3 UI3 = uvec3(UI0, UI1, UI2_EXTRA);

// 预先计算浮点数，减少运行时开销
const float UIF = 2.3283064e-10; // 1.0 / 4294967295.0

const float PI = 3.1415926535f;

const mat4 M = {
    {0.36f, -0.096f, 0.928f, 0.0f},
    {0.48f, 0.872f, -0.096f, 0.0f},
    {-0.8f, 0.48f, 0.36f, 1.0f},
    {0.0f, 0.0f, 0.0f, 1.0f}
};

struct BoundingBox
{
    vec3 position;
    vec3 front;
    vec3 halfDiagonal;
};

struct Surface
{
    vec3 point;
    vec3 normal;
};


struct BoundingBoxOrthogonalBasis
{
    vec3 forward;
    vec3 up;
    vec3 right;
};

struct Ray
{
    vec3 origin;
    vec3 direction;
};

struct BoundingBoxSurfaces
{
    Surface positiveForwardSurface;
    Surface negativeForwardSurface;

    Surface positiveUpSurface;
    Surface negativeUpSurface;

    Surface positiveRightSurface;
    Surface negativeRightSurface;
};

struct BoundingBoxIntersections
{
    vec3 firstIntersectionPoint;
    vec3 secondIntersectionPoint;
};

vec3 hash33(vec3 p) {
    p += vec3(435.456f, 43.683f, 4958.32f);
    p = fract(p * 0.1031f);               // 简单缩放减少大数运算
    p += dot(p, p.yzx + 3.3543f);          // 维度混合，打破对称性
    return fract((p.xzx + p.yxz) * p.zyx); // 直接组合操作，避免 sin
}

float perlinNoise(vec3 pos, float freq)
{
    vec3 p = floor(pos);
    vec3 w = fract(pos);
    vec3 u = w * w * (3.0 - 2.0 * w); 

    float a = dot(hash33(p + vec3(0.0f, 0.0f, 0.0f)), w - vec3(0.0f, 0.0f, 0.0f));
    float b = dot(hash33(p + vec3(1.0f, 0.0f, 0.0f)), w - vec3(1.0f, 0.0f, 0.0f));
    float c = dot(hash33(p + vec3(0.0f, 1.0f, 0.0f)), w - vec3(0.0f, 1.0f, 0.0f));
    float d = dot(hash33(p + vec3(1.0f, 1.0f, 0.0f)), w - vec3(1.0f, 1.0f, 0.0f));
    float e = dot(hash33(p + vec3(0.0f, 0.0f, 1.0f)), w - vec3(0.0f, 0.0f, 1.0f));
    float f = dot(hash33(p + vec3(1.0f, 0.0f, 1.0f)), w - vec3(1.0f, 0.0f, 1.0f));
    float g = dot(hash33(p + vec3(0.0f, 1.0f, 1.0f)), w - vec3(0.0f, 1.0f, 1.0f));
    float h = dot(hash33(p + vec3(1.0f, 1.0f, 1.0f)), w - vec3(1.0f, 1.0f, 1.0f));

    float s = mix(mix(a, b, u.x), mix(c, d, u.x), u.y);
    float t = mix(mix(e, f, u.x), mix(g, h, u.x), u.y);
    return mix(s, t, u.z);

}

float worleyNoise(vec3 pos, float freq)
{
    vec3 p = floor(pos);
    vec3 u = fract(pos);
    float minDistance = 10000.0f;

    for (float x = -1.0f; x <= 1.0f; ++x)
        for (float y = -1.0f; y <= 1.0f; ++y)
            for (float z = -1.0f; z <= 1.0f; ++z)
            {
                vec3 offset = vec3(x, y, z);

                vec3 h = offset + hash33(mod(p + offset, freq)) * 0.5f + 0.5f;

                vec3 d = u - h;

                minDistance = min(minDistance, dot(d, d));
            }
    
    return 1 - minDistance;
}

float fbmPerlinNoise(vec3 pos, float freq, int octaves)
{

    float G = exp2(-.85);
    float amp = 1.0f;
    float noise = 0.0f;
    mat3 m = mat3(M);

    for (int i = 0; i < octaves; ++i)
    {
        noise += perlinNoise(m * pos / amp, freq);
        freq *= 2.0f;
        amp *= G;
        m *= mat3(M);
    }

    return noise;
}

vec3 getBoundingBoxSize(BoundingBox boundingBox)
{
    vec3 result;
    vec3 halfDiagonal = boundingBox.halfDiagonal;
    result = abs(halfDiagonal) * 2.0f;
    return result;
}

BoundingBoxOrthogonalBasis getBoundingBoxOrthogonalBasis(BoundingBox boundingBox)
{
    BoundingBoxOrthogonalBasis result;

    result.forward = boundingBox.front;
    result.up = vec3(0.0f, 1.0f, 0.0f);
    result.right = cross(result.forward, result.up);
    result.up = cross(result.right, result.forward);

    result.forward = normalize(result.forward);
    result.up = normalize(result.up);
    result.right = normalize(result.right);

    return result;
}

bool isPointInBoundingBox(BoundingBox boundingBox, vec3 point)
{
    point -= boundingBox.position;

    BoundingBoxOrthogonalBasis orthogonalBasis = getBoundingBoxOrthogonalBasis(boundingBox);
    float projectForward = abs(dot(orthogonalBasis.forward, point));
    float projectRight = abs(dot(orthogonalBasis.right, point));
    float projectUp = abs(dot(orthogonalBasis.up, point));


    vec3 halfSize = getBoundingBoxSize(boundingBox) / 2.0f;

    float littleCompensate = 0.0005f;

    halfSize += littleCompensate;

    if (projectForward > halfSize.z)
        return false;
    if (projectUp > halfSize.y)
        return false;
    if (projectRight > halfSize.x)
        return false;
    
    if (point == vec3(0.0f))
        return false;

    return true;
}

bool raySurfaceIntersect(Ray ray, Surface surface, out float intersectDistance)
{
    if (dot(ray.direction, surface.normal) == 0)
        return false;
    vec3 normal = normalize(surface.normal);
    vec3 direction = normalize(ray.direction);
    intersectDistance = dot(surface.point - ray.origin, normal) / dot(direction, normal);
    return true;
}

BoundingBoxSurfaces getBoundingBoxSurfaces(BoundingBox boundingBox)
{
    BoundingBoxSurfaces result;
    vec3 halfSize = getBoundingBoxSize(boundingBox) / 2.0f;
    vec3 pos = boundingBox.position;
    BoundingBoxOrthogonalBasis orthogonalBasis = getBoundingBoxOrthogonalBasis(boundingBox);


    result.positiveForwardSurface.point = pos + halfSize.z * orthogonalBasis.forward;
    result.positiveForwardSurface.normal = orthogonalBasis.forward;

    result.negativeForwardSurface.point = pos - halfSize.z * orthogonalBasis.forward;
    result.negativeForwardSurface.normal = -orthogonalBasis.forward;

    result.positiveRightSurface.point = pos + halfSize.x * orthogonalBasis.right;
    result.positiveRightSurface.normal = orthogonalBasis.right;

    result.negativeRightSurface.point = pos - halfSize.x * orthogonalBasis.right;
    result.negativeRightSurface.normal = -orthogonalBasis.right;

    result.positiveUpSurface.point = pos + halfSize.y * orthogonalBasis.up;
    result.positiveUpSurface.normal = orthogonalBasis.up;

    result.negativeUpSurface.point = pos - halfSize.y * orthogonalBasis.up;
    result.negativeUpSurface.normal = -orthogonalBasis.up;


    return result;
}

bool BoundingBoxIntersect(Ray ray, BoundingBox boundingBox, out BoundingBoxIntersections intersections)
{
    BoundingBoxSurfaces surfaces = getBoundingBoxSurfaces(boundingBox);
    float distances[6];
    bool intersectStates[6];

    intersectStates[0] = raySurfaceIntersect(ray, surfaces.positiveForwardSurface, distances[0]);
    intersectStates[1] = raySurfaceIntersect(ray, surfaces.negativeForwardSurface, distances[1]);
    intersectStates[2] = raySurfaceIntersect(ray, surfaces.positiveUpSurface, distances[2]);
    intersectStates[3] = raySurfaceIntersect(ray, surfaces.negativeUpSurface, distances[3]);
    intersectStates[4] = raySurfaceIntersect(ray, surfaces.positiveRightSurface, distances[4]);
    intersectStates[5] = raySurfaceIntersect(ray, surfaces.negativeRightSurface, distances[5]);


    uint intersectionCount = 0;

    for (uint i = 0; i < 6; ++i)
        if (intersectStates[i])
        {
            vec3 surfaceIntersection = ray.origin + ray.direction * distances[i];
            if (!isPointInBoundingBox(boundingBox, surfaceIntersection))
                intersectStates[i] = false;
            else
                intersectionCount++;
        }
    
    if (intersectionCount == 0)
        return false;
    if (intersectionCount == 1)
        return false;
    if (intersectionCount == 2)
    {
        float intersectionDistances[2];
        uint index = 0;
        for (uint i = 0; i < 6; ++i)
            if (intersectStates[i])
                intersectionDistances[index++] = distances[i];
        float maxDistance = max(intersectionDistances[0], intersectionDistances[1]);
        float minDistance = min(intersectionDistances[0], intersectionDistances[1]);

        if (minDistance > 0 && maxDistance > 0)
        {
            intersections.firstIntersectionPoint = ray.origin + ray.direction * minDistance;
            intersections.secondIntersectionPoint = ray.origin + ray.direction * maxDistance;
            return true;
        }
        if (minDistance < 0 && maxDistance > 0)
        {
            intersections.firstIntersectionPoint = ray.origin;
            intersections.secondIntersectionPoint = ray.origin + ray.direction * maxDistance;
            return true;
        }
        return false;
    }
    return false;
}

vec3 rayMatch(BoundingBoxIntersections keyPositions, float stepLength)
{
    float integral = 0.0f;

    vec3 stepPos = keyPositions.firstIntersectionPoint;

    vec3 rayDir = keyPositions.secondIntersectionPoint - stepPos;


    int max_step = 40;
    
    float matchDistance = length(rayDir);

    float step = abs(matchDistance) / max_step;
    
    rayDir = normalize(rayDir);

    for (uint i = 0; i < max_step; ++i)
    {
        if (fbmPerlinNoise((keyPositions.firstIntersectionPoint + i * step * rayDir) / 30.0f, 19.0, 6) > /*abs(mod(ubo.time, 5.0f) / 5.0f * 4.0f - 2.0f) - 1.0f*/ 0.3f)
        {
            integral += step / 1.0f;
        }
    }

    return vec3(integral);
}






BoundingBox boundingBox1 = {
        vec3(0.0f, 0.0f, 0.0f),
        vec3(1.0f, 0.0f, 0.0f),
        vec3(100.0f, 5.0f, 100.0f)
    };

void main()
{
    vec3 cameraFront = ubo.front;
    vec3 cameraUp = vec3(0.0f, 1.0f, 0.0f);
    vec3 cameraRight = cross(cameraFront, cameraUp);
    cameraUp = cross(cameraRight, cameraFront);

    cameraFront = normalize(cameraFront);
    cameraUp = normalize(cameraUp);
    cameraRight = normalize(cameraRight);

    Ray ray;
    ray.origin = ubo.pos;

    vec2 screenNdc = vec2(fragTexCoord.x, 1.0f - fragTexCoord.y) * 2.0f - 1.0f;
    ray.direction = cameraFront * ubo.near + (cameraUp * screenNdc.y / ubo.screenSize.x * ubo.screenSize.y + cameraRight * screenNdc.x) * ubo.near * tan(ubo.fov / 360.0f * PI);
    ray.direction = normalize(ray.direction);
    outColor = vec4(1.0f);

    BoundingBoxIntersections intersections;
    
    

    outColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    if (BoundingBoxIntersect(ray, boundingBox1, intersections))
        outColor = vec4(rayMatch(intersections, 0.1f), 1.0f);
}