#include "oglwidget.h"

#include <QPainter>
#include <QPaintEvent>

OGLWidget::OGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{

}

void OGLWidget::paintEvent(QPaintEvent *event)
{
    QPainter paint(this);

    auto size = this->size();
    auto offset = 4;

    paint.setBrush(Qt::white);

    paint.drawEllipse(0, 0, size.width() - offset, size.height() - offset);
}
