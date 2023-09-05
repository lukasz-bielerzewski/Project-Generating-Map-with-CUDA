#include "oglwidget.h"

#include <cmath>

OGLWidget::OGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    this->cx = 319.5f;
    this->cy = -239.5f;
    this->focal_x = 481.2f;
    this->focal_y = -480.f;
}

OGLWidget::~OGLWidget()
{

    delete this->image;
    delete this->depthImage;
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // OpenGL initialization code
    glClearColor(1.0, 1.0, 1.0, 1.0);

    this->loadImage();
    this->transformToPointCloud();

    rotationMatrix.setToIdentity();
}

void OGLWidget::resizeGL(int w, int h)
{
    // Resize viewport and update projection matrix
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Calculate scaling factors to stretch the point cloud
    float scaleX = static_cast<float>(w) / static_cast<float>(this->originalWidth);
    float scaleY = static_cast<float>(h) / static_cast<float>(this->originalHeight);

    glOrtho(0.f, static_cast<float>(w), static_cast<float>(h), 0.f, -99999.f, 99999.f);
    glScalef(scaleX, scaleY, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Trigger a repaint to show the point cloud
    update();
}

void OGLWidget::paintGL()
{
    // Clear the buffer and apply the rotation transformation
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Apply the rotation matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(rotationMatrix.constData());

    // Render the point cloud
    glPointSize(1.0f);
    glBegin(GL_POINTS);
    for (const auto& point : pointCloud) {
        glColor3f(point.second.redF(), point.second.greenF(), point.second.blueF());
        glVertex3f(point.first.x(), point.first.y(), point.first.z());
    }
    glEnd();
}



void OGLWidget::loadImage()
{
    // Load and prepare the image
    this->image = new QImage("office_kt0/rgb/1.png");
    if (image->isNull()) {
        qWarning("Failed to load image.");
        return;
    }

    this->originalWidth = this->image->width();
    this->originalHeight = this->image->height();

    this->depthImage = new QImage("office_kt0/depth/1.png", "QImage::Format_Grayscale16");
    if (depthImage->isNull()) {
        qWarning("Failed to load depthImage.");
        return;
    }
}

void OGLWidget::transformToPointCloud()
{
    // Convert image to point cloud with pixel colors
    pointCloud.clear();

    float scalling_factor;

    for (int y = 0; y < this->image->height(); ++y) {
        for (int x = 0; x < this->image->width(); ++x) {
            QRgb pixel = this->image->pixel(x, y);
            qint16 grayValue = this->depthImage->pixel(x,y);
            float xpos = static_cast<float>(x);
            float ypos = static_cast<float>(y);

            // Euclidean to Planar depth conversion
            scalling_factor = sqrt((x - this->cx) * (x - this->cx) + (y - this->cy) * (y - this->cy) + this->focal_x * this->focal_x) / this->focal_x;
            float zpos = static_cast<float>(grayValue) / scalling_factor;

            qDebug() << " z =" << zpos ;

            QVector3D position(xpos, ypos, zpos);
            QColor color(pixel);

            pointCloud.append({ position, color });
        }
    }
}


void OGLWidget::mousePressEvent(QMouseEvent* event)
{
    lastMousePos = event->pos();
}


void OGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    QVector2D diff = QVector2D(event->pos() - lastMousePos);
    rotationMatrix.rotate(diff.x() / 100.f, 0.0f, 1.0f, 0.0f); // Yaw rotation
    rotationMatrix.rotate(diff.y() / 100.f, 1.0f, 0.0f, 0.0f); // Pitch rotation

    // Store the current mouse position for the next movement
    lastMousePos = event->pos();

    // Trigger a repaint to update the rendered image
    update();
}






