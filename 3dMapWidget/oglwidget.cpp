#include "oglwidget.h"
#include <cmath>




OGLWidget::OGLWidget(QWidget* parent) : QOpenGLWidget(parent)
{
    this->cx = 319.5f;
    this->cy = 239.5f;
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

    this->readTrajectoryData("/home/maks/foto/salon/trajectory.txt", this->trajectoryData);
    
    this->transformToPointCloud();

    this->displayTrajectoryDataPoints();



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

    GLfloat	mat[16];
    glPushMatrix();
    glMultMatrixf(mat);

    float lineWidth = 3.0;

    glLineWidth((GLfloat)lineWidth);

    glBegin(GL_LINES);

    glColor3f (1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);

    glColor3f (0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);

    glColor3f (0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);
    glEnd();
    glPopMatrix();


    // Render the point cloud
    glPointSize(3.0f);
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
    QString imagePath = QString("/home/maks/foto/salon/rgb/%1.png").arg(imageIndex);
    this->image = new QImage(imagePath);
    if (image->isNull()) {
        qWarning("Failed to load image.");
        return;
    }

    this->originalWidth = this->image->width();
    this->originalHeight = this->image->height();

    // Load the corresponding depth image
    QString depthImagePath = QString("/home/maks/foto/salon/depth/%1.png").arg(imageIndex);
    this->depthImage = new QImage(depthImagePath, "QImage::Format_Grayscale16");
    if (depthImage->isNull()) {
        qWarning("Failed to load depthImage.");
        return;
    }

}


void OGLWidget::transformToPointCloud()
{
    // Convert image to point cloud with pixel colors
    pointCloud.clear();

    int imageIndex=0;
     int trajectoryIndex = 0;



    float xpos = 0;
    float ypos = 0;



    for (int i = 1; i <= 50; i++) {
        loadImage(i);


//        qDebug() << "imageIndex: " << imageIndex;


        double Tx =  this->trajectoryData[trajectoryIndex][0];
        double Ty = this->trajectoryData[trajectoryIndex][1];
        double Tz = this->trajectoryData[trajectoryIndex][2];
        double qx = this->trajectoryData[trajectoryIndex][3];
        double qy = this->trajectoryData[trajectoryIndex][4];
        double qz = this->trajectoryData[trajectoryIndex][5];
        double qw = this->trajectoryData[trajectoryIndex][6];


        // First row of the rotation matrix
       float r00 = (2 * (qx * qx + qy * qy)) - 1;
       float r01 = 2 * (qy * qz - qx * qw);
       float r02 = 2 * (qy * qw + qx * qz);

        // Second row of the rotation matrix
       float r10 = 2 * (qy * qz + qx * qw);
       float r11 = (2 * (qx * qx + qz * qz)) - 1;
       float r12 = 2 * (qz * qw - qx * qy);

        // Third row of the rotation matrix
       float r20 = 2 * (qy * qw - qx * qz);
       float r21 = 2 * (qz * qw + qx * qy);
       float r22 =(2 * (qx * qx + qw * qw)) - 1;


        std::vector<std::vector<double>> matrix = {
            {r00 , r01 , r02, Tx},
            {r10 , r11, r12, Ty},
            {r20, r21, r22, Tz},
            {0, 0, 0, 1}
        };



//           qDebug() << "Tx: " << Tx;
//           qDebug() << "Ty: " << Ty;
//           qDebug() << "Tz: " << Tz;
//           qDebug() << "pitch: " << qx;
//           qDebug() << "yaw: " << qy;
//           qDebug() << "roll: " << qz;
//           qDebug() << "qw: " << qw;


        // Apply the transformation matrix to xpos, ypos, and zpos
        for (int v = 0; v < this->image->height(); ++v) {
            for (int u = 0; u < this->image->width(); ++u) {
                QRgb pixel = this->image->pixel(u, v);
                qint16 grayValue = this->depthImage->pixel(u, v);
                float zpos = static_cast<float>(grayValue)*1000/65536;

                //Euclidean to Planar depth conversion

                xpos = (u - cx) / focal_x;
                ypos = (v - cy) / focal_y;

                xpos = -xpos * zpos;
                ypos = ypos * zpos;


//                if(u%10000==0)
//                {
//                            qDebug() << "xpos: " << xpos << "ypos:" << ypos << "zpos: " << zpos;
//                }

                //quaternion to rotation matrix
                float transformed_xpos = matrix[0][0] * xpos + matrix[0][1] * ypos + matrix[0][2] * zpos + matrix[0][3];
                float transformed_ypos = matrix[1][0] * xpos + matrix[1][1] * ypos + matrix[1][2] * zpos + matrix[1][3];
                float transformed_zpos = matrix[2][0] * xpos + matrix[2][1] * ypos + matrix[2][2] * zpos + matrix[2][3];



                xpos = transformed_xpos;
                ypos = transformed_ypos;
                zpos = transformed_zpos;


                QVector3D position(xpos, ypos, zpos);


                this->octreeMap->insertPoint({xpos, ypos, zpos});

                QColor color(pixel);

                pointCloud.append({ position, color });


            }
        }

        imageIndex++;
        trajectoryIndex = imageIndex-1;
        displayTrajectoryDataPoints();

    }




    update();

}

void OGLWidget::displayTrajectoryDataPoints()
{
    // Assuming trajectoryData contains at least 3 values for each cell
    for (const auto& cell : trajectoryData)
    {
        if (cell.size() >= 3)
        {
            float xpos = static_cast<float>(cell[0]);
            float ypos = static_cast<float>(cell[1]);
            float zpos = static_cast<float>(cell[2]);

            xpos = xpos * 65536/1000;
            ypos = ypos * 65536/1000;
            zpos = zpos * 65536/1000;

//            qDebug() << "traj_x: " << xpos << "traj_y: " << ypos << "traj_z" << zpos;



            // Color the point pink
            QColor color(Qt::magenta);

            // Create a point at the specified position with the pink color
            QVector3D position(xpos, ypos, zpos);
            pointCloud.append({ position, color });
        }
    }

    // Trigger a repaint to update the rendered image
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

void OGLWidget::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->angleDelta().y() / 8; // Get the wheel rotation value

    // Adjust the zoom factor based on the scroll direction
    if (numDegrees > 0) {
        zoomFactor *= 1.1; // Zoom out
    } else {
        zoomFactor *= 0.9; // Zoom in
    }

    update(); // Request widget redraw
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




//przyblizyc
//poprawic wyglad kodu
//skala
