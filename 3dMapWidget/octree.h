#ifndef OCTREE_H
#define OCTREE_H

#include <vector>
#include <iostream>

struct Point {
    float x, y, z;
};

struct Voxel {
    float x, y, z;
    float sideLength;
    int pointCount;
    std::vector<Voxel*> children;

    Voxel(float x, float y, float z, float sideLength)
        : x(x), y(y), z(z), sideLength(sideLength), pointCount(0)
    {

    }

    ~Voxel()
    {
        for (Voxel* child : children) {
            delete child;
        }
    }
};

class Octree
{
public:
    Octree(float x, float y, float z, float initialSideLength, float minSideLength);
    ~Octree();

    void insertPoint(const Point& point);
    void printVoxels() const;

private:
    bool pointInVoxel(Voxel* voxel, const Point& point);
    void insertPoint(Voxel* voxel, const Point& point);
    void printVoxels(const Voxel* voxel) const;

    Voxel* rootVoxel;
    float minSideLength;
};

#endif // OCTREE_H
