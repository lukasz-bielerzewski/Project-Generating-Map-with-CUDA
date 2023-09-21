#include "octree.h"

Octree::Octree(float x, float y, float z, float initialSideLength, float minSideLength)
    : rootVoxel(new Voxel(x, y, z, initialSideLength)), minSideLength(minSideLength)
{

}

Octree::~Octree()
{
    delete rootVoxel;
}

void Octree::insertPoint(const Point &point)
{
    this->insertPoint(rootVoxel, point);
}

void Octree::printVoxels() const
{
    printVoxels(rootVoxel);
}

bool Octree::pointInVoxel(Voxel* voxel, const Point& point)
{
    return (point.x >= voxel->x && point.x <= voxel->x + voxel->sideLength) &&
           (point.y >= voxel->y && point.y <= voxel->y + voxel->sideLength) &&
           (point.z >= voxel->z && point.z <= voxel->z + voxel->sideLength);
}

void Octree::insertPoint(Voxel* voxel, const Point& point)
{
    if (!pointInVoxel(voxel, point)) {
        return;
    }

    if (voxel->sideLength <= minSideLength) {
        voxel->pointCount++;
        return;
    }

    float half = voxel->sideLength / 2;
    float x = voxel->x, y = voxel->y, z = voxel->z;

    if (voxel->children.empty()) {
        for (int i = 0; i < 8; i++) {
            float newX = x + (i & 1) * half;
            float newY = y + ((i >> 1) & 1) * half;
            float newZ = z + ((i >> 2) & 1) * half;
            voxel->children.push_back(new Voxel(newX, newY, newZ, half));
        }
    }

    for (Voxel* child : voxel->children) {
        insertPoint(child, point);
    }

    voxel->pointCount++;
}

void Octree::printVoxels(const Voxel* voxel) const
{
    std::cout << "Voxel at (" << voxel->x << ", " << voxel->y << ", " << voxel->z
              << ") with side length " << voxel->sideLength << " contains " << voxel->pointCount
              << " points" << std::endl;
    for (Voxel* child : voxel->children) {
        printVoxels(child);
    }
}
