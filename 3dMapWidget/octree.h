#ifndef OCTREE_H
#define OCTREE_H

#include <QColor>

#include <vector>
#include <iostream>
#include <eigen3/Eigen/Dense>

struct Point {
    float x, y, z;
};

struct Voxel {
    float x, y, z;
    float sideLength;
    int pointCount;
    std::vector<Voxel*> children;

    // Members for NDT-OM
    Eigen::Vector3f mean; // Mean of the Gaussian distribution (3x1 vector)
    Eigen::Matrix3f covariance; // Covariance matrix of the Gaussian distribution (3x3 matrix)

    // Members for accumulating color
    Eigen::Vector3f accumulatedColor; // Accumulated color of the points
    unsigned int colorCount; // Number of points contributing to the color

    Voxel(float x, float y, float z, float sideLength)
        : x(x), y(y), z(z), sideLength(sideLength), pointCount(0)
    {
        mean.setZero();
        covariance.setZero();

        accumulatedColor.setZero();
        colorCount = 0;
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

    void insertPoint(const Point& point, const QColor &color);
    void printVoxels() const;

    Voxel* rootVoxel;

private:
    bool pointInVoxel(Voxel* voxel, const Point& point);
    void insertPoint(Voxel* voxel, const Point& point, const Eigen::Vector3f& color);
    void printVoxels(const Voxel* voxel) const;

    float minSideLength;
};

#endif // OCTREE_H
