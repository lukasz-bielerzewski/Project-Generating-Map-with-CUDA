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
    void loadImage();
    void transformToPointCloud();

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;


private:
    QImage* image = nullptr;
    QImage* depthImage = nullptr;
    QVector<QPair<QVector3D, QColor>> pointCloud;

    int originalWidth = 1;
    int originalHeight = 1;

    QPoint lastMousePos;
    QMatrix4x4 rotationMatrix;

};

#endif // OGLWIDGET_H
