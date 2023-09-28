#include "octree.h"

Octree::Octree(float x, float y, float z, float initialSideLength, float minSideLength)
    : rootVoxel(new Voxel(x, y, z, initialSideLength)), minSideLength(minSideLength)
{

}

Octree::~Octree()
{
    delete rootVoxel;
}

void Octree::insertPoint(const Point &point, const QColor &color)
{
    Eigen::Vector3f _color = Eigen::Vector3f(static_cast<float>(color.red()), static_cast<float>(color.green()), static_cast<float>(color.blue()));
    this->insertPoint(rootVoxel, point, _color);
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

void Octree::insertPoint(Voxel* voxel, const Point& point, const Eigen::Vector3f& color)
{
    if (!pointInVoxel(voxel, point)) {
        return;
    }

    if (voxel->sideLength <= minSideLength) {

        // Update the accumulated color and color count
        voxel->accumulatedColor += color;
        voxel->colorCount++;

        // Convert Point to Eigen Vector
        Eigen::Vector3f pointVec(point.x, point.y, point.z);

        // Update the mean
        Eigen::Vector3f oldMean = voxel->mean;
        voxel->mean = (oldMean * voxel->pointCount + pointVec) / (voxel->pointCount + 1);

        // Update the covariance
        Eigen::Matrix3f pointDiff = (pointVec - oldMean) * (pointVec - voxel->mean).transpose();
        voxel->covariance = (voxel->covariance * voxel->pointCount + pointDiff) / (voxel->pointCount + 1);

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
        insertPoint(child, point, color);
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
