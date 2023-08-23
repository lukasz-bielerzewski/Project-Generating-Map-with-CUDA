#include "oglwidget.h"

OGLWidget::OGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{

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

    glOrtho(0.f, static_cast<float>(w), static_cast<float>(h), 0.f, -255.f, 0.f);
    glScalef(scaleX, scaleY, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Trigger a repaint to show the point cloud
    update();
}

void OGLWidget::paintGL()
{
    // Render code
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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

    this->depthImage = new QImage("office_kt0/depth/1.png");
    if (depthImage->isNull()) {
        qWarning("Failed to load depthImage.");
        return;
    }
}

void OGLWidget::transformToPointCloud()
{
    // Convert image to point cloud with pixel colors
    pointCloud.clear();
    for (int y = 0; y < this->image->height(); ++y) {
        for (int x = 0; x < this->image->width(); ++x) {
            QRgb pixel = this->image->pixel(x, y);
            QRgb depthPixel = this->depthImage->pixel(x,y);
            int grayValue = qGray(depthPixel);
            float xpos = static_cast<float>(x);
            float ypos = static_cast<float>(y);
            float zpos = static_cast<float>(grayValue);
            QVector3D position(xpos, ypos, zpos);
            QColor color(pixel);

            pointCloud.append({ position, color });
        }
    }
}
