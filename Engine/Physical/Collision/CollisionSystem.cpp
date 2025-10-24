#include "CollisionSystem.h"
#include "CollisionBox.h"
#include "CollisionCapsule.h"
#include "CollisionObject.h"
#include "CollisionObjectDelegator.h"
#include "CollisionTriangle.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <cmath>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <unordered_set>

constexpr float EPSILON = 1e-6f;

CollisionSystem::CollisionSystem()
    : _pos(glm::vec3(-1000.0f)), _size(glm::vec3(2000.0f)),
      _staticCollisionObjects(Octree<CollisionObjectDelegatorPtr>::createRoot(
          [&](const Octree<CollisionObjectDelegatorPtr> &octree, const CollisionObjectDelegatorPtr &collisionObject) -> MipResult
          {
              MipResult result = {false, 0};
              const uint32_t level = octree.level() + 1;
              const float invPowLevel2 = 1.0f / static_cast<float>(1u << level);

              // 单位旋转矩阵 & 基础大小
              const glm::mat3 identityAxes(1.0f);
              const glm::vec3 half = _size * 0.5f * invPowLevel2;

              // 预定义8个偏移
              static const glm::vec3 offsets[8] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}};

              CollisionBox octreeBoundary;
              octreeBoundary.setAxes(identityAxes);
              octreeBoundary.setBoundBoxSize(_size * invPowLevel2);

              for (uint8_t pos = 0; pos < 8; ++pos)
              {
                  const glm::vec3 offset = glm::vec3((octree.x() * 2 + offsets[pos].x), (octree.y() * 2 + offsets[pos].y), (octree.z() * 2 + offsets[pos].z));

                  octreeBoundary.setCenter(_pos + (_size * (offset + 0.5f) * invPowLevel2));

                  if (performCollisionDetection(collisionObject->collisionObject<CollisionObject>(), octreeBoundary).intersect)
                  {
                      if (result.mip)
                          return {false, 0};
                      result.mip = true;
                      result.pos = pos;
                  }
              }
              return result;
          }))
{
}
CollisionSystem::~CollisionSystem() {}

CollisionResult CollisionSystem::performCollisionDetection(const CollisionObject &src, const CollisionObject &dst) const
{
    CollisionResult result;
    if (src.type() == CollisionObjectType::Box && dst.type() == CollisionObjectType::Box)
        result = performBoxVBoxCollisionDetection(*dynamic_cast<const CollisionBox *>(&src), *dynamic_cast<const CollisionBox *>(&dst));
    else if (src.type() == CollisionObjectType::Box && dst.type() == CollisionObjectType::Capsule)
        result = performBoxVCapsuleCollisionDetection(*dynamic_cast<const CollisionBox *>(&src), *dynamic_cast<const CollisionCapsule *>(&dst));
    else if (src.type() == CollisionObjectType::Capsule && dst.type() == CollisionObjectType::Box)
        result = performCapsuleVBoxCollisionDetection(*dynamic_cast<const CollisionCapsule *>(&src), *dynamic_cast<const CollisionBox *>(&dst));
    else if (src.type() == CollisionObjectType::Capsule && dst.type() == CollisionObjectType::Capsule)
        result = performCapsuleVCapsuleCollisionDetection(*dynamic_cast<const CollisionCapsule *>(&src), *dynamic_cast<const CollisionCapsule *>(&dst));
    else if (src.type() == CollisionObjectType::Box && dst.type() == CollisionObjectType::Triangle)
        result = performBoxVTriangleCollisionDetection(*dynamic_cast<const CollisionBox *>(&src), *dynamic_cast<const CollisionTriangle *>(&dst));
    else if (src.type() == CollisionObjectType::Triangle && dst.type() == CollisionObjectType::Box)
        result = performTriangleVBoxCollisionDetection(*dynamic_cast<const CollisionTriangle *>(&src), *dynamic_cast<const CollisionBox *>(&dst));
    else if (src.type() == CollisionObjectType::Capsule && dst.type() == CollisionObjectType::Triangle)
        result = performCapsuleVTriangleCollisionDetection(*dynamic_cast<const CollisionCapsule *>(&src), *dynamic_cast<const CollisionTriangle *>(&dst));
    else if (src.type() == CollisionObjectType::Triangle && dst.type() == CollisionObjectType::Capsule)
        result = performTriangleVCapsuleCollisionDetection(*dynamic_cast<const CollisionTriangle *>(&src), *dynamic_cast<const CollisionCapsule *>(&dst));
    else if (src.type() == CollisionObjectType::Triangle && dst.type() == CollisionObjectType::Triangle)
        result = performTriangleVTriangleCollisionDetection(*dynamic_cast<const CollisionTriangle *>(&src), *dynamic_cast<const CollisionTriangle *>(&dst));
    else
        throw std::runtime_error("Unknown collision.");
    return result;
}

CollisionResult CollisionSystem::performBoxVBoxCollisionDetection(const CollisionBox &src, const CollisionBox &dst) const
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

CollisionResult CollisionSystem::performBoxVCapsuleCollisionDetection(const CollisionBox &box, const CollisionCapsule &capsule) const
{
    CollisionResult result;

    glm::vec3 dir = capsule.direction();
    float halfH = capsule.height() * 0.5f - capsule.radius();

    glm::vec3 p1 = capsule.center() + dir * halfH;
    glm::vec3 p2 = capsule.center() - dir * halfH;

    glm::mat3 axes = box.axes();
    glm::vec3 half = box.halfSize();
    glm::vec3 l1 = glm::transpose(axes) * (p1 - box.center());
    glm::vec3 l2 = glm::transpose(axes) * (p2 - box.center());

    float t = 0.0f;
    glm::vec3 m = (l1 + l2) * 0.5f;
    glm::vec3 closest(0.0f);
    for (int i = 0; i < 3; ++i)
    {
        float c = m[i];
        if (c < -half[i])
            closest[i] = -half[i];
        else if (c > half[i])
            closest[i] = half[i];
        else
            closest[i] = c;
    }

    glm::vec3 worldClosest = box.center() + axes * closest;

    glm::vec3 segDir = p2 - p1;
    float len2 = glm::dot(segDir, segDir);
    t = glm::clamp(glm::dot(worldClosest - p1, segDir) / len2, 0.0f, 1.0f);
    glm::vec3 closestOnSeg = p1 + t * segDir;

    glm::vec3 diff = worldClosest - closestOnSeg;
    float dist = glm::length(diff);
    if (dist < capsule.radius())
    {
        result.intersect = true;
        result.depth = capsule.radius() - dist;
        result.axis = dist > 1e-6f ? -diff / dist : axes[1];
    }
    return result;
}

CollisionResult CollisionSystem::performCapsuleVBoxCollisionDetection(const CollisionCapsule &src, const CollisionBox &dst) const
{
    CollisionResult result = performBoxVCapsuleCollisionDetection(dst, src);
    if (result.intersect)
        result.axis = -result.axis;
    return result;
}

CollisionResult CollisionSystem::performCapsuleVCapsuleCollisionDetection(const CollisionCapsule &src, const CollisionCapsule &dst) const
{
    CollisionResult result;

    glm::vec3 dirA = glm::vec3(0, 1, 0);
    glm::vec3 dirB = glm::vec3(0, 1, 0);
    glm::vec3 a1 = src.center() + dirA * (src.height() * 0.5f - src.radius());
    glm::vec3 a2 = src.center() - dirA * (src.height() * 0.5f - src.radius());
    glm::vec3 b1 = dst.center() + dirB * (dst.height() * 0.5f - dst.radius());
    glm::vec3 b2 = dst.center() - dirB * (dst.height() * 0.5f - dst.radius());

    glm::vec3 u = a2 - a1;
    glm::vec3 v = b2 - b1;
    glm::vec3 w = a1 - b1;

    float aDot = glm::dot(u, u);
    float bDot = glm::dot(v, v);
    float abDot = glm::dot(u, v);
    float awDot = glm::dot(u, w);
    float bwDot = glm::dot(v, w);

    float denom = aDot * bDot - abDot * abDot;
    float s, t;

    if (denom < 1e-6f)
    {
        s = 0.0f;
        t = bwDot / bDot;
    }
    else
    {
        s = (abDot * bwDot - bDot * awDot) / denom;
        t = (aDot * bwDot - abDot * awDot) / denom;
    }

    s = glm::clamp(s, 0.0f, 1.0f);
    t = glm::clamp(t, 0.0f, 1.0f);

    glm::vec3 closestA = a1 + s * u;
    glm::vec3 closestB = b1 + t * v;

    glm::vec3 diff = closestA - closestB;
    float dist = glm::length(diff);
    float radiusSum = src.radius() + dst.radius();

    if (dist < radiusSum)
    {
        result.intersect = true;
        result.depth = radiusSum - dist;
        result.axis = -glm::normalize(diff);
    }

    return result;
}

CollisionResult CollisionSystem::performBoxVTriangleCollisionDetection(const CollisionBox &box, const CollisionTriangle &tri) const
{
    CollisionResult result;
    glm::vec3 v0 = tri.vertex(0), v1 = tri.vertex(1), v2 = tri.vertex(2);
    glm::mat3 A = box.axes();
    glm::vec3 h = box.halfSize(), c = box.center();

    glm::vec3 e[3] = {v1 - v0, v2 - v1, v0 - v2};
    glm::vec3 n = glm::cross(e[0], e[1]);

    if (glm::length2(n) < 1e-8f)
        return result;

    auto projTri = [&](const glm::vec3 &ax)
    {
        float p0 = glm::dot(v0, ax), p1 = glm::dot(v1, ax), p2 = glm::dot(v2, ax);
        return std::pair(std::min({p0, p1, p2}), std::max({p0, p1, p2}));
    };
    auto projBox = [&](const glm::vec3 &ax)
    {
        float cProj = glm::dot(c, ax), r = 0;
        for (int i = 0; i < 3; ++i)
            r += std::abs(glm::dot(ax, A[i])) * h[i];
        return std::pair(cProj - r, cProj + r);
    };

    float minOverlap = FLT_MAX;
    glm::vec3 bestAx(0);
    auto test = [&](glm::vec3 ax)
    {
        if (glm::length2(ax) < 1e-12f)
            return true;
        ax = glm::normalize(ax);
        auto [t0, t1] = projTri(ax);
        auto [b0, b1] = projBox(ax);
        float o = std::min(t1, b1) - std::max(t0, b0);
        if (o < 0)
            return false;
        if (o < minOverlap)
        {
            minOverlap = o;
            bestAx = ax;
        }
        return true;
    };

    for (int i = 0; i < 3; ++i)
        if (!test(A[i]))
            return result;
    if (!test(n))
        return result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            if (!test(glm::cross(A[j], e[i])))
                return result;

    result.intersect = true;
    glm::vec3 tc = (v0 + v1 + v2) / 3.f;
    if (glm::dot(tc - c, bestAx) < 0)
        bestAx = -bestAx;
    result.axis = bestAx;
    result.depth = minOverlap;
    return result;
}

CollisionResult CollisionSystem::performTriangleVBoxCollisionDetection(const CollisionTriangle &tri, const CollisionBox &box) const
{
    auto r = performBoxVTriangleCollisionDetection(box, tri);
    if (r.intersect)
        r.axis = -r.axis;
    return r;
}

CollisionResult CollisionSystem::performCapsuleVTriangleCollisionDetection(const CollisionCapsule &cap, const CollisionTriangle &tri) const
{
    CollisionResult result;
    glm::vec3 dir = cap.direction();
    float half = cap.height() * 0.5f - cap.radius();
    glm::vec3 p1 = cap.center() + dir * half, p2 = cap.center() - dir * half;
    glm::vec3 v0 = tri.vertex(0), v1 = tri.vertex(1), v2 = tri.vertex(2);

    auto segClosest = [&](glm::vec3 p, glm::vec3 q, glm::vec3 x)
    {
        glm::vec3 d = q - p;
        float t = glm::clamp(glm::dot(x - p, d) / glm::dot(d, d), 0.f, 1.f);
        return p + t * d;
    };
    auto triClosest = [&](glm::vec3 p)
    {
        glm::vec3 ab = v1 - v0, ac = v2 - v0, ap = p - v0;
        float d1 = glm::dot(ab, ap), d2 = glm::dot(ac, ap);
        if (d1 <= 0 && d2 <= 0)
            return v0;
        glm::vec3 bp = p - v1;
        float d3 = glm::dot(ab, bp), d4 = glm::dot(ac, bp);
        if (d3 >= 0 && d4 <= d3)
            return v1;
        float vc = d1 * d4 - d3 * d2;
        if (vc <= 0 && d1 >= 0 && d3 <= 0)
            return v0 + (d1 / (d1 - d3)) * ab;
        glm::vec3 cp = p - v2;
        float d5 = glm::dot(ab, cp), d6 = glm::dot(ac, cp);
        if (d6 >= 0 && d5 <= d6)
            return v2;
        float vb = d5 * d2 - d1 * d6;
        if (vb <= 0 && d2 >= 0 && d6 <= 0)
            return v0 + (d2 / (d2 - d6)) * ac;
        float va = d3 * d6 - d5 * d4;
        if (va <= 0 && (d4 - d3) >= 0 && (d5 - d6) >= 0)
            return v1 + ((d4 - d3) / ((d4 - d3) + (d5 - d6))) * (v2 - v1);
        glm::vec3 n = glm::normalize(glm::cross(ab, ac));
        return p - glm::dot(p - v0, n) * n;
    };

    glm::vec3 cA = triClosest(p1), cB = triClosest(p2);
    glm::vec3 sA = segClosest(p1, p2, cA), sB = segClosest(p1, p2, cB);
    glm::vec3 pc, pt;
    float dA = glm::length2(cA - sA), dB = glm::length2(cB - sB);
    if (dA < dB)
    {
        pc = sA;
        pt = cA;
    }
    else
    {
        pc = sB;
        pt = cB;
    }
    glm::vec3 d = pc - pt;
    float dist = glm::length(d);
    if (dist < cap.radius())
    {
        result.intersect = true;
        result.axis = -glm::normalize(d);
        result.depth = cap.radius() - dist;
    }
    return result;
}

CollisionResult CollisionSystem::performTriangleVCapsuleCollisionDetection(const CollisionTriangle &tri, const CollisionCapsule &cap) const
{
    auto r = performCapsuleVTriangleCollisionDetection(cap, tri);
    if (r.intersect)
        r.axis = -r.axis;
    return r;
}

CollisionResult CollisionSystem::performTriangleVTriangleCollisionDetection(const CollisionTriangle &a, const CollisionTriangle &b) const
{
    CollisionResult result;
    auto A = a.vertices(), B = b.vertices();
    float best = FLT_MAX;
    glm::vec3 pa, pb;

    auto edgeEdge = [&](glm::vec3 p1, glm::vec3 q1, glm::vec3 p2, glm::vec3 q2)
    {
        glm::vec3 d1 = q1 - p1, d2 = q2 - p2, r = p1 - p2;
        float a1 = glm::dot(d1, d1), a2 = glm::dot(d2, d2), a12 = glm::dot(d1, d2);
        float det = a1 * a2 - a12 * a12, s = 0;
        if (det > 1e-8f)
            s = glm::clamp((a12 * glm::dot(d2, r) - a2 * glm::dot(d1, r)) / det, 0.f, 1.f);
        glm::vec3 c1 = p1 + s * d1;
        float t = glm::clamp(glm::dot(d2, c1 - p2) / a2, 0.f, 1.f);
        glm::vec3 c2 = p2 + t * d2;
        return std::pair(c1, c2);
    };

    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            auto [ca, cb] = edgeEdge(A[i], A[(i + 1) % 3], B[j], B[(j + 1) % 3]);
            float d2 = glm::length2(ca - cb);
            if (d2 < best)
            {
                best = d2;
                pa = ca;
                pb = cb;
            }
        }

    float dist = glm::sqrt(best);
    if (dist < 1e-6f)
    {
        glm::vec3 nA = glm::normalize(glm::cross(A[1] - A[0], A[2] - A[0]));
        glm::vec3 nB = glm::normalize(glm::cross(B[1] - B[0], B[2] - B[0]));
        result.intersect = true;
        result.axis = glm::normalize(nA + nB);
        result.depth = 1e-6f - dist;
    }
    return result;
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
        {
            try
            {
                CollisionResult result = performCollisionDetection(object.collisionObject<CollisionObject>(), targetObject->collisionObject<CollisionObject>());
                if (result.intersect)
                    result.pTarget = targetObject.get();
                object.recordCollisionResult(result);
            }
            catch (std::runtime_error e)
            {
#ifdef DEBUG
                std::cout << e.what() << std::endl;
#endif
            }
        }

        if (!pCollisionOctree->hasSubTrees())
            continue;

        for (uint8_t pos = 0; pos < 8; ++pos)
        {
            const Octree<CollisionObjectDelegatorPtr>::Ptr &pSubCollisionOctree = pCollisionOctree->subTree(pos);
            if (collisionObjectWithOctree(object, *pSubCollisionOctree))
                pPCollisionOctrees.push_back(&pSubCollisionOctree);
        }
    }
}

bool CollisionSystem::collisionObjectWithOctree(const CollisionObjectDelegator &object, const Octree<CollisionObjectDelegatorPtr> &octree)
{
    float powLevel2 = 1u << octree.level();
    CollisionBox octreeBoundary;
    octreeBoundary.setAxes(glm::toMat3(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)));
    octreeBoundary.setBoundBoxSize(_size / powLevel2);
    octreeBoundary.setCenter(_pos + _size * glm::vec3(octree.x() + 0.5f, octree.y() + 0.5f, octree.z() + 0.5f) / powLevel2);
    return performCollisionDetection(object.collisionObject<CollisionObject>(), octreeBoundary).intersect;
}

void CollisionSystem::updateWithDynamicCollisionObjects(CollisionObjectDelegator &object)
{
    for (const CollisionObjectDelegatorPtr &targetObject : _dynamicCollisionObjects)
        if (targetObject.get() != &object)
        {
            CollisionResult result = performCollisionDetection(object.collisionObject<CollisionObject>(), targetObject->collisionObject<CollisionObject>());
            if (result.intersect)
                result.pTarget = targetObject.get();
            object.recordCollisionResult(result);
        }
}

CollisionSystem &CollisionSystem::instance()
{
    static CollisionSystem instance;
    return instance;
}

void CollisionSystem::destroyDynamicObject(CollisionObjectDelegator *pCollisionObject)
{
    for (auto it = _dynamicCollisionObjects.begin(); it != _dynamicCollisionObjects.end(); ++it)
        if (it->get() == pCollisionObject)
        {
            _dynamicCollisionObjects.erase(it);
            break;
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

const Octree<CollisionSystem::CollisionObjectDelegatorPtr> *CollisionSystem::octree() { return _staticCollisionObjects.get(); }

void CollisionSystem::Deleter::operator()(CollisionObjectDelegator *pCollisionObjectDelegator) { delete pCollisionObjectDelegator; }

void CollisionSystem::Deleter::operator()(CollisionObject *pCollisionObject) { delete pCollisionObject; }
