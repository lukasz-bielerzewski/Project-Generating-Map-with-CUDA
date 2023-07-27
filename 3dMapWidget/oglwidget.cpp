#include "oglwidget.h"

OGLWidget::OGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{

}

OGLWidget::~OGLWidget()
{

}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

     // OpenGL initialization code
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // Enable texturing
    glEnable(GL_TEXTURE_2D);

    this->loadImage();
    this->transformToTexture();
}

void OGLWidget::resizeGL(int w, int h)
{
    // Resize viewport and update projection matrix
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, h, 0, -1, 1);

    // Flip the y-axis
    glScalef(1, -1, 1);

    // Translate the coordinate system to the top-left corner
    glTranslatef(0, -h, 0);

    glMatrixMode(GL_MODELVIEW);
}

void OGLWidget::paintGL()
{
    // Render code
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind the image texture
    if (this->texture) {
        this->texture->bind();
    }

    // Render the image as a quad
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(width(), 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(width(), height());
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, height());
    glEnd();

    // Unbind the image texture
    if (this->texture) {
        this->texture->release();
    }
}

void OGLWidget::loadImage()
{
    // Load and prepare the image
    this->image = new QImage("office_kt0/rgb/1.png");
    if (image->isNull()) {
        qWarning("Failed to load image.");
        return;
    }
}

void OGLWidget::transformToTexture()
{
    // Convert image to OpenGL format
    QImage temp = this->image->convertToFormat(QImage::Format_RGBA8888);

    // Create a texture from the image
    this->texture = new QOpenGLTexture(temp.mirrored());
    this->texture->setMinificationFilter(QOpenGLTexture::Nearest);
    this->texture->setMagnificationFilter(QOpenGLTexture::Nearest);
}
