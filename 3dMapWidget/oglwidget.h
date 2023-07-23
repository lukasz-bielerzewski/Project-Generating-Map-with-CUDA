#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLTexture>

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

private:
    QOpenGLTexture* texture = nullptr;
};

#endif // OGLWIDGET_H
