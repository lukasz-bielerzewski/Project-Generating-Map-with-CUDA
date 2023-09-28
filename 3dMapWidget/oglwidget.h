#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QImage>
#include <QVector3D>
#include <QMouseEvent>
#include <QPoint>
#include <QMatrix4x4>
#include <QMatrix3x3>
#include <QString>

#include <fstream>
#include <string>
#include <sstream>

#include <eigen3/Eigen/Geometry>

#include "octree.h"

class OGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    OGLWidget(QWidget* parent = nullptr);
    virtual ~OGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void loadImage(int imageIndex);
    void transformToPointCloud();
    void readTrajectoryData(const std::string& filePath, std::vector<std::vector<double>>& trajectoryData);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

    float getDepthVal(int x, int y);

    void renderEllipsoids(const Voxel* voxel);
    void createUnitSphere();

    void renderSingleEllipsoid();

private:
    QImage* image = nullptr;
    QImage* depthImage = nullptr;
    QVector<QPair<QVector3D, QColor>> pointCloud;

    int originalWidth = 1;
    int originalHeight = 1;

    QPoint lastMousePos;
    QMatrix4x4 rotationMatrix;

    float cx;
    float cy;
    float focal_x;
    float focal_y;

    std::vector<std::vector<double>> trajectoryData;

    Octree *octreeMap;

    GLuint sphereVBO, sphereVAO, sphereEBO;
    GLsizei sphereIndexCount;

    GLint colorLocation;
    GLuint shaderProgram;
    GLint transformLocation;

    void setupShaders();

    const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0

uniform mat4 transform; // transformation matrix

void main()
{
    gl_Position = transform * vec4(aPos, 1.0); // transform the vertex position
}
)glsl";

    const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

uniform vec3 color; // uniform color

void main()
{
    FragColor = vec4(color, 1.0); // set the output color of the fragment
}
)glsl";
};

#endif // OGLWIDGET_H
