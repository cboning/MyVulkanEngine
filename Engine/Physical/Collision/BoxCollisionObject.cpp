#include "BoxCollisionObject.h"
#include "SphereCollisionObject.h"

constexpr float EPSILON = 1e-6f;

BoxCollisionObject::BoxCollisionObject() : CollisionObject(CollisionObjectType::Box) {}

CollisionResult BoxCollisionObject::performCollisionDetection(const CollisionObject &target) const
{
    if (target.type() == CollisionObjectType::Box)
        return performBoxCollisionDetection(static_cast<const BoxCollisionObject &>(target));
    else if (target.type() == CollisionObjectType::Sphere)
        return performEllipsoidCollisionDetection(static_cast<const EllipsoidCollisionObject &>(target));
    return {};
}

CollisionResult BoxCollisionObject::performBoxCollisionDetection(const BoxCollisionObject &target) const
{
    CollisionResult result;

    glm::vec3 halfSizeA = halfSize();
    glm::vec3 halfSizeB = target.halfSize();
    glm::vec3 translation = target.center() - center();

    glm::mat3 axesA = axes();
    glm::mat3 axesB = target.axes();

    glm::mat3 rotation;
    glm::mat3 absRotation;

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            rotation[i][j] = glm::dot(axesA[i], axesB[j]);
            absRotation[i][j] = std::abs(rotation[i][j]) + EPSILON;
        }

    glm::vec3 t;
    for (int i = 0; i < 3; i++)
        t[i] = glm::dot(translation, axesA[i]);

    float minOverlap = std::numeric_limits<float>::max();
    glm::vec3 bestAxis(0);

    auto updateMTV = [&](float overlap, const glm::vec3 &axis)
    {
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            bestAxis = axis;
        }
    };

    for (int i = 0; i < 3; ++i)
    {
        float ra = halfSizeA[i];
        float rb = halfSizeB[0] * absRotation[i][0] + halfSizeB[1] * absRotation[i][1] + halfSizeB[2] * absRotation[i][2];
        float dist = std::abs(glm::dot(translation, axesA[i]));
        float overlap = ra + rb - dist;
        if (overlap < 0)
            return result;
        updateMTV(overlap, axesA[i]);
    }

    for (int i = 0; i < 3; ++i)
    {
        float ra = halfSizeA[0] * absRotation[0][i] + halfSizeA[1] * absRotation[1][i] + halfSizeA[2] * absRotation[2][i];
        float rb = halfSizeB[i];
        float dist = std::abs(glm::dot(translation, axesB[i]));
        float overlap = ra + rb - dist;
        if (overlap < 0)
            return result;
        updateMTV(overlap, axesB[i]);
    }

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            float ra = halfSizeA[(i + 1) % 3] * absRotation[(i + 2) % 3][j] + halfSizeA[(i + 2) % 3] * absRotation[(i + 1) % 3][j];
            float rb = halfSizeB[(j + 1) % 3] * absRotation[i][(j + 2) % 3] + halfSizeB[(j + 2) % 3] * absRotation[i][(j + 1) % 3];
            float dist = std::abs(t[(i + 2) % 3] * rotation[(i + 1) % 3][j] - t[(i + 1) % 3] * rotation[(i + 2) % 3][j]);
            float overlap = ra + rb - dist;
            if (overlap < 0)
                return result;
            glm::vec3 axis = glm::cross(axesA[i], axesB[j]);
            if (glm::length(axis) > 1e-6f){
                axis = glm::normalize(axis);
            updateMTV(overlap, axis);}
        }

    result.intersect = true;

    if (glm::dot(target.center() - center(), bestAxis) < 0)
        bestAxis = -bestAxis;

    result.axis = bestAxis;
    result.depth = minOverlap;

    return result;
}

CollisionResult BoxCollisionObject::performEllipsoidCollisionDetection(const EllipsoidCollisionObject &target) const
{
    glm::mat3 ReT = glm::transpose(target.axes());

    glm::mat3 S = glm::mat3(1.0f);
    S[0][0] = 1.0f / target.boundBoxSize().x;
    S[1][1] = 1.0f / target.boundBoxSize().y;
    S[2][2] = 1.0f / target.boundBoxSize().z;
    glm::mat3 M = S * ReT;

    glm::vec3 transformedBoxCenter = M * (center() - target.center());
    glm::mat3 transformedBoxAxis;
    glm::vec3 transformedBoxHalfSize;

    for (int i = 0; i < 3; i++)
    {
        glm::vec3 axisT = M * axes()[i];
        float len = glm::length(axisT);
        if (len > 1e-8f)
        {
            transformedBoxAxis[i] = axisT / len;
            transformedBoxHalfSize[i] = halfSize()[i] * len;
        }
        else
        {
            transformedBoxAxis[i] = glm::vec3(0, 0, 0);
            transformedBoxHalfSize[i] = 0;
        }
    }

    glm::vec3 d = -transformedBoxCenter;
    glm::vec3 closest = transformedBoxCenter;
    for (int i = 0; i < 3; i++)
    {
        float dist = glm::dot(d, transformedBoxAxis[i]);
        float clamped = std::clamp(dist, -transformedBoxHalfSize[i], transformedBoxHalfSize[i]);
        closest += transformedBoxAxis[i] * clamped;
    }
    return {glm::length(closest) <= 1.0f};
}
