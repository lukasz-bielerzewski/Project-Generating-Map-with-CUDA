#ifndef OGLWIDGET_H
#define OGLWIDGET_H

#include <QOpenGLWidget>
#include <QWidget>

//forward declarations
class QPaintEvent;

class OGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    OGLWidget(QWidget* parent = nullptr);

    void paintEvent(QPaintEvent* event);
};

#endif // OGLWIDGET_H
