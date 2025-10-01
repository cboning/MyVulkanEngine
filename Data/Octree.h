#pragma once
#include <functional>
#include <string>
#include <vector>

struct MipResult
{
    bool mip;
    uint8_t pos;
};

template <typename T> class Octree
{
private:
    struct Deleter
    {
        void operator()(Octree<T> *pOctree) { delete pOctree; }
        void operator()(T *pData) { delete pData; }
    };

public:
    using Ptr = std::unique_ptr<Octree<T>, Deleter>;
    static Ptr createRoot(std::function<MipResult(const Octree<T> &, const T &)> mipFunc);
    void addObject(T);
    uint32_t level() const;
    uint32_t x() const;
    uint32_t y() const;
    uint32_t z() const;
    bool hasSubTrees() const;
    const Ptr &subTree(uint8_t pos) const;
    const Ptr &subTree(bool x, bool y, bool z) const;
    const std::vector<T> &objects() const;

private:
    Ptr _pSubTrees[8];

    std::vector<T> _objects;
    uint32_t _level;
    uint32_t _x;
    uint32_t _y;
    uint32_t _z;
    bool _mipped = false;

    std::function<MipResult(const Octree<T> &, const T &)> _mipFunc;

    Octree(uint32_t _level, uint32_t _x, uint32_t _y, uint32_t _z, std::function<MipResult(const Octree<T> &, const T &)> mipFunc);
    ~Octree();
};

template <typename T> inline Octree<T>::Ptr Octree<T>::createRoot(std::function<MipResult(const Octree<T> &, const T &)> mipFunc)
{
    return Ptr(new Octree<T>(0, 0, 0, 0, mipFunc));
}

template <typename T> inline void Octree<T>::addObject(T object)
{
    MipResult result;
    Octree *pBlock = this;
    do
    {
        result = _mipFunc(*pBlock, object);
        if (!result.mip)
        {
            pBlock->_objects.push_back(std::move(object));
            break;
        }

        if (!pBlock->_mipped)
            for (uint8_t i = 0; i < 8; ++i)
                pBlock->_pSubTrees[i] =
                    Ptr(new Octree<T>(_level + 1, pBlock->_x * 2 + ((i >> 0) & 1), pBlock->_y * 2 + ((i >> 1) & 1), pBlock->_z * 2 + ((i >> 2) & 1), _mipFunc));
        pBlock->_mipped = true;
        if (result.pos >= 8 || !pBlock->_pSubTrees[result.pos])
            throw std::runtime_error("Invalid mip result position");

        pBlock = pBlock->_pSubTrees[result.pos].get();

    } while (result.mip);
}

template <typename T> inline uint32_t Octree<T>::level() const { return _level; }

template <typename T> inline uint32_t Octree<T>::x() const { return _x; }

template <typename T> inline uint32_t Octree<T>::y() const { return _y; }

template <typename T> inline uint32_t Octree<T>::z() const { return _z; }

template <typename T> inline bool Octree<T>::hasSubTrees() const { return _mipped; }

template <typename T> inline const Octree<T>::Ptr &Octree<T>::subTree(uint8_t pos) const { return _pSubTrees[pos]; }

template <typename T> inline const Octree<T>::Ptr &Octree<T>::subTree(bool x, bool y, bool z) const { return subTree(x + (y << 1) + (z << 2)); }

template <typename T> inline const std::vector<T> &Octree<T>::objects() const { return _objects; }

template <typename T>
inline Octree<T>::Octree(uint32_t level, uint32_t x, uint32_t y, uint32_t z, std::function<MipResult(const Octree<T> &, const T &)> mipFunc)
    : _level(level), _x(x), _y(y), _z(z), _mipFunc(mipFunc)
{
}

template <typename T> inline Octree<T>::~Octree() {}