#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>
#include <QImage>
#include <QVector3D>
#include <QColor>

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

private:
    QImage* image = nullptr;
    QVector<QPair<QVector3D, QColor>> pointCloud;

    int originalWidth = 1;
    int originalHeight = 1;
};

#endif // OGLWIDGET_H
