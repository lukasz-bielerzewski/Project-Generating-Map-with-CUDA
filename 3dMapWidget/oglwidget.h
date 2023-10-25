#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QImage>
#include <QVector3D>
#include <QColor>
#include <QMouseEvent>
#include <QPoint>
#include <QMatrix4x4>
#include <QMatrix3x3>

#include <fstream>
#include <string>
#include <sstream>

#include "octree.h"

#include <cmath>



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
    void displayTrajectoryDataPoints();
//    void displayTrajectoryVectors();

    static OGLWidget* currentInstance;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;


    float getDepthVal(int x, int y);

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


    float zoomFactor=1.0f;


    std::vector<std::vector<double>> trajectoryData;
    Octree *octreeMap;
};

#endif // OGLWIDGET_H
