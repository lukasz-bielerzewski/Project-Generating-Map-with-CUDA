#include "oglwidget.h"

#include <cmath>

OGLWidget::OGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    this->cx = 319.5f;
    this->cy = -239.5f;
    this->focal_x = 481.2f;
    this->focal_y = -480.f;

    this->setFocus();
    this->setFocusPolicy(Qt::StrongFocus);

    this->octreeMap = new Octree(0.f, 0.f, 0.f, 10000.f, 1250.f);
}

OGLWidget::~OGLWidget()
{

    delete this->image;
    delete this->depthImage;
    delete this->octreeMap;
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // OpenGL initialization code
    glClearColor(1.0, 1.0, 1.0, 1.0);
    int imageIndex=0;
    this->loadImage(imageIndex);

    this->readTrajectoryData("/home/maks/foto/biuro/trajectory.txt", this->trajectoryData);
    
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



void OGLWidget::loadImage(int imageIndex)
{
    // Load and prepare the image
    QString imagePath = QString("/home/maks/foto/biuro/rgb/%1.png").arg(imageIndex);
    this->image = new QImage(imagePath);
    if (image->isNull()) {
        qWarning("Failed to load image.");
        return;
    }

    this->originalWidth = this->image->width();
    this->originalHeight = this->image->height();

    // Load the corresponding depth image
    QString depthImagePath = QString("/home/maks/foto/biuro/depth/%1.png").arg(imageIndex);
    this->depthImage = new QImage(depthImagePath, "QImage::Format_Grayscale16");
    if (depthImage->isNull()) {
        qWarning("Failed to load depthImage.");
        return;
    }

    // Increment x in trajectoryData[x][0]
    if (imageIndex < trajectoryData.size()) {
        trajectoryData[imageIndex][0] += 1.0; // Increment x value
    }
}


void OGLWidget::transformToPointCloud()
{
    // Convert image to point cloud with pixel colors
    pointCloud.clear();

    float scaling_factor;
    int imageIndex = 0;

    // Check if trajectoryData has at least one position

    // Get the first position from trajectoryData
    // const std::vector<double>& firstPosition = trajectoryData[0];

    // Extract the transformation matrix elements
       
       double Tx =  trajectoryData[imageIndex][0];
       double Ty = trajectoryData[imageIndex][1];
       double Tz = trajectoryData[imageIndex][2];
       double pitch = trajectoryData[imageIndex][3];
       double yaw = trajectoryData[imageIndex][4];
       double roll = trajectoryData[imageIndex][5];

    
    qDebug() << "Tx: " << Tx;
    qDebug() << "Ty: " << Ty;
    qDebug() << "Tz: " << Tz;
    qDebug() << "pitch: " << pitch;
    qDebug() << "yaw: " << yaw;
    qDebug() << "roll: " << roll;

    for (int i = 1; i <= 300; ++i) {
        loadImage(i);

    // Apply the transformation matrix to xpos, ypos, and zpos
    for (int y = 0; y < this->image->height(); ++y) {
        for (int x = 0; x < this->image->width(); ++x) {
            QRgb pixel = this->image->pixel(x, y);
            qint16 grayValue = this->depthImage->pixel(x, y);
            float xpos = static_cast<float>(x);
            float ypos = static_cast<float>(y);

            // Euclidean to Planar depth conversion
            scaling_factor = sqrt((x - this->cx) * (x - this->cx) + (y - this->cy) * (y - this->cy) + this->focal_x * this->focal_x) / this->focal_x;
            float zpos = static_cast<float>(grayValue) / scaling_factor;


            float transformed_xpos = xpos * cos(yaw) - ypos * sin(yaw) + Tx;
            float transformed_ypos = xpos * sin(yaw) + ypos * cos(yaw) + Ty;
            float transformed_zpos = zpos + Tz;



            // Update position
            xpos = transformed_xpos;
            ypos = transformed_ypos;
            zpos = transformed_zpos;
            this->octreeMap->insertPoint({xpos, ypos, zpos});

            QVector3D position(xpos, ypos, zpos);
            QColor color(pixel);

            pointCloud.append({ position, color });
        }
    }

    this->octreeMap->printVoxels();
    imageIndex++;
    }

    update();

}

void OGLWidget::readTrajectoryData(const std::string& filePath, std::vector<std::vector<double>>& trajectoryData)
{
    // Open the file
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        qDebug() << "Failed to open trajectory file: " << filePath.c_str();
        return;
    }

    // Read and parse the data line by line
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::vector<double> rowData;
        double value;

        // Skip the first column and read the rest
        iss >> value;
        while (iss >> value)
        {
            rowData.push_back(value);
        }

        if (!rowData.empty())
        {
            trajectoryData.push_back(rowData);
        }
    }

    // Close the file
    file.close();
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

void OGLWidget::keyPressEvent(QKeyEvent* event)
{
    // Calculate the forward and right vectors based on the current rotation
    QVector3D forward = -QVector3D(rotationMatrix.column(2));
    QVector3D right = QVector3D(rotationMatrix.column(0));

    // Scale the vectors to control movement speed
    float movementSpeed = 100.0f;
    forward *= movementSpeed;
    right *= movementSpeed;

    if (event->key() == Qt::Key_W)
    {
        // Move forward
        rotationMatrix.translate(forward);
    }
    else if (event->key() == Qt::Key_S)
    {
        // Move backward
        rotationMatrix.translate(-forward);
    }
    else if (event->key() == Qt::Key_D)
    {
        // Move left
        rotationMatrix.translate(-right);
    }
    else if (event->key() == Qt::Key_A)
    {
        // Move right
        rotationMatrix.translate(right);
    }
    else if (event->key() == Qt::Key_R)
    {
        // Reset view to original position
        rotationMatrix.setToIdentity();
    }

    // Trigger a repaint to update the rendered image
    update();


    // Pass the event to the base class to handle other key presses
    QOpenGLWidget::keyPressEvent(event);
}
