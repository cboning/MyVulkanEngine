#include "CollisionSystem.h"
#include "CollisionObjectDelegator.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <unordered_set>

constexpr float EPSILON = 1e-6f;

CollisionSystem::CollisionSystem()
    : _staticCollisionObjects(
          Octree<CollisionObjectDelegatorPtr>::createRoot([](const Octree<CollisionObjectDelegatorPtr> &, const CollisionObjectDelegatorPtr &) -> MipResult {}))
{
}
CollisionSystem::~CollisionSystem() {}

CollisionResult CollisionSystem::performCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const
{
    if (src.type() == CollisionObjectType::Box && dst.type() == CollisionObjectType::Box)
        return performBoxVBoxCollisionDetection(src, dst);
    else if (src.type() == CollisionObjectType::Box && dst.type() == CollisionObjectType::Ellipsoid)
        return performBoxVEllipsoidCollisionDetection(src, dst);
}

CollisionResult CollisionSystem::performBoxVBoxCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const
{
    CollisionResult result;

    glm::vec3 halfSizeA = src.halfSize();
    glm::vec3 halfSizeB = dst.halfSize();
    glm::vec3 translation = dst.center() - src.center();

    glm::mat3 axesA = src.axes();
    glm::mat3 axesB = dst.axes();

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
            if (glm::length(axis) > 1e-6f)
            {
                axis = glm::normalize(axis);
                updateMTV(overlap, axis);
            }
        }

    result.intersect = true;

    if (glm::dot(dst.center() - src.center(), bestAxis) < 0)
        bestAxis = -bestAxis;

    result.axis = bestAxis;
    result.depth = minOverlap;

    return result;
}

CollisionResult CollisionSystem::performBoxVEllipsoidCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const
{
    glm::mat3 ReT = glm::transpose(dst.axes());

    glm::mat3 S = glm::mat3(1.0f);
    S[0][0] = 1.0f / dst.boundBoxSize().x;
    S[1][1] = 1.0f / dst.boundBoxSize().y;
    S[2][2] = 1.0f / dst.boundBoxSize().z;
    glm::mat3 M = S * ReT;

    glm::vec3 transformedBoxCenter = M * (src.center() - dst.center());
    glm::mat3 transformedBoxAxis;
    glm::vec3 transformedBoxHalfSize;

    for (int i = 0; i < 3; i++)
    {
        glm::vec3 axisT = M * src.axes()[i];
        float len = glm::length(axisT);
        if (len > 1e-8f)
        {
            transformedBoxAxis[i] = axisT / len;
            transformedBoxHalfSize[i] = src.halfSize()[i] * len;
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
        float clamped = glm::clamp(dist, -transformedBoxHalfSize[i], transformedBoxHalfSize[i]);
        closest += transformedBoxAxis[i] * clamped;
    }
    return {glm::length(closest) <= 1.0f};
}

void CollisionSystem::updateWithStaticCollisionObjects(CollisionObjectDelegator &object)
{
    std::vector<const Octree<CollisionObjectDelegatorPtr>::Ptr *> pPCollisionOctrees = {&_staticCollisionObjects};
    while (pPCollisionOctrees.size())
    {
        const Octree<CollisionObjectDelegatorPtr>::Ptr &pCollisionOctree = *pPCollisionOctrees.back();
        pPCollisionOctrees.pop_back();

        const std::vector<CollisionSystem::CollisionObjectDelegatorPtr> &targetObjects = pCollisionOctree->objects();

        for (const CollisionSystem::CollisionObjectDelegatorPtr &targetObject : targetObjects)
            object.recordCollisionResult(performCollisionDetection(object, *targetObject));

        if (!pCollisionOctree->hasSubTrees())
            continue;

        for (uint8_t pos = 0; pos < 8; ++pos)
        {
            const Octree<CollisionObjectDelegatorPtr>::Ptr &pSubCollisionOctree = pCollisionOctree->subTree(pos);
            float powLevel2 = std::pow(2, pSubCollisionOctree->level());

            CollisionObject octreeBoundary(CollisionObjectType::Box);
            octreeBoundary.setAxes(glm::toMat3(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
            octreeBoundary.setBoundBoxSize(_size / powLevel2);
            octreeBoundary.setPosition(_pos + _size * glm::vec3(pSubCollisionOctree->x() / powLevel2));
            octreeBoundary.setPositionInBoundBox(glm::vec3(0.0f));

            CollisionResult result = performCollisionDetection(object, octreeBoundary);
            if (result.intersect)
                pPCollisionOctrees.push_back(&pSubCollisionOctree);
        }
    }
}

void CollisionSystem::updateWithDynamicCollisionObjects(CollisionObjectDelegator &object)
{
    for (const CollisionObjectDelegatorPtr &targetObject : _dynamicCollisionObjects)
        if (targetObject.get() != &object)
            object.recordCollisionResult(performCollisionDetection(object, *targetObject));
}

CollisionSystem &CollisionSystem::instance()
{
    static CollisionSystem instance;
    return instance;
}

CollisionObjectDelegator *CollisionSystem::createStaticObject(CollisionObjectType type, const Object &object)
{
    CollisionObjectDelegator *pCollisionObject = new CollisionObjectDelegator(type);
    std::unique_ptr<CollisionObjectDelegator, Deleter> collisionObject(pCollisionObject);
    collisionObject->updateWithObject(object);
    _staticCollisionObjects->addObject(std::move(collisionObject));
    return pCollisionObject;
}

CollisionObjectDelegator *CollisionSystem::createDynamicObject(CollisionObjectType type)
{
    CollisionObjectDelegator *pCollisionObject = new CollisionObjectDelegator(type);
    std::unique_ptr<CollisionObjectDelegator, Deleter> collisionObject(pCollisionObject);
    _dynamicCollisionObjects.push_back(std::move(collisionObject));
    return pCollisionObject;
}

void CollisionSystem::destoryDynamicObject(CollisionObjectDelegator *pCollisionObject)
{
    for (auto it = _dynamicCollisionObjects.begin(); it != _dynamicCollisionObjects.end(); ++it)
    {
        if (it->get() == pCollisionObject)
        {
            _dynamicCollisionObjects.erase(it);
            break;
        }
    }
}

void CollisionSystem::update()
{
    for (auto &dynamicCollisionObject : _dynamicCollisionObjects)
    {
        dynamicCollisionObject.get()->cleanResult();
        updateWithStaticCollisionObjects(*dynamicCollisionObject);
        updateWithDynamicCollisionObjects(*dynamicCollisionObject);
    }
}
