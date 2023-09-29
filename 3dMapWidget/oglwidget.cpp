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

    this->octreeMap = new Octree(0.f, 0.f, 0.f, 25000.f, 3125.f);
}

OGLWidget::~OGLWidget()
{

    delete this->image;
    delete this->depthImage;
    delete this->octreeMap;

    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
}

void OGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    // OpenGL initialization code
    glClearColor(0.0, 0.0, 0.0, 1.0);

    int image_index = 0;

    this->loadImage(image_index);
    this->readTrajectoryData("/home/maks/foto/salon/trajectory.txt", this->trajectoryData);
    this->transformToPointCloud();

    setupShaders();

    // Create the unit sphere geometry
    createUnitSphere();

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
//    glPointSize(1.0f);
//    glBegin(GL_POINTS);
//    for (const auto& point : pointCloud) {
//        glColor3f(point.second.redF(), point.second.greenF(), point.second.blueF());
//        glVertex3f(point.first.x(), point.first.y(), point.first.z());
//    }
//    glEnd();

    // Traverse the octree and render ellipsoids for the smallest voxels
   // renderEllipsoids(octreeMap->rootVoxel);

    renderSingleEllipsoid();
}

void OGLWidget::renderSingleEllipsoid()
{
    // Ensure a shader program is in use
    glUseProgram(shaderProgram);

    // Define synthetic data for a single ellipsoid
    Eigen::Vector3f meanColor = Eigen::Vector3f(1.0f, 0.0f, 0.0f); // Red color
    Eigen::Vector3f center = Eigen::Vector3f(0.0f, 0.0f, 0.0f); // Center at origin
    Eigen::Matrix3f covariance = Eigen::Matrix3f::Identity() * Eigen::Vector3f(0.1f, 0.2f, 0.3f).asDiagonal(); // Scaled identity matrix

    // Set the color uniform in the fragment shader
    glUniform3fv(colorLocation, 1, meanColor.data());

    // Compute the eigen decomposition of the covariance matrix
    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(covariance);

    // Check if the covariance matrix is positive definite
    if (solver.info() != Eigen::Success) {
        qDebug() << "error";
        return;
    }

    Eigen::Matrix3f axes = solver.eigenvectors();
    Eigen::Matrix3f scale = Eigen::Scaling(solver.eigenvalues().cwiseSqrt());

    Eigen::Affine3f transform;
    transform.fromPositionOrientationScale(center, Eigen::Quaternionf(axes), scale.diagonal());

    // Set the transformation matrix as a uniform variable in the shader
    glUniformMatrix4fv(transformLocation, 1, GL_FALSE, transform.matrix().data());

    glBindVertexArray(sphereVAO);

    // Draw the unit sphere (transformed to an ellipsoid)
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
}

void OGLWidget::loadImage(int imageIndex)
{
    // Load and prepare the image
    QString imagePath = QString("/home/maks/foto/salon/rgb/%1.png").arg(imageIndex);
    this->image = new QImage(imagePath);
    if (image->isNull())
    {
        qWarning("Failed to load image.");
        return;
    }

    this->originalWidth = this->image->width();
    this->originalHeight = this->image->height();

    QString depthImagePath = QString("/home/maks/foto/salon/depth/%1.png").arg(imageIndex);
    this->depthImage = new QImage(depthImagePath, "QImage::Format_Grayscale16");
    if (depthImage->isNull())
    {
        qWarning("Failed to load depthImage.");
        return;
    }

    // Increment x in trajectoryData[x][0]
    if (imageIndex < trajectoryData.size())
    {
        trajectoryData[imageIndex][0] += 1.0; // Increment x value
    }
}

void OGLWidget::transformToPointCloud()
{
    // Convert image to point cloud with pixel colors
    pointCloud.clear();

    float scalling_factor;

    int imageIndex = 0;

    // Check if trajectoryData has at least one position

    // Get the first position from trajectoryData
    // const std::vector<double>& firstPosition = trajectoryData[0];

    // Extract the transformation matrix elements


//    qDebug() << "Tx: " << Tx;
//    qDebug() << "Ty: " << Ty;
//    qDebug() << "Tz: " << Tz;
//    qDebug() << "pitch: " << pitch;
//    qDebug() << "yaw: " << yaw;
//    qDebug() << "roll: " << roll;

    for (int i = 1; i <= 50; ++i) {
        loadImage(i);

        double Tx =  this->trajectoryData[imageIndex][0];
        double Ty = this->trajectoryData[imageIndex][1];
        double Tz = this->trajectoryData[imageIndex][2];
        double qx = this->trajectoryData[imageIndex][3];
        double qy = this->trajectoryData[imageIndex][4];
        double qz = this->trajectoryData[imageIndex][5];
        double qw = this->trajectoryData[imageIndex][6];

     //   qDebug() << "Tx: " << Tx;
     //   qDebug() << "Ty: " << Ty;
     //   qDebug() << "Tz: " << Tz;
     //   qDebug() << "pitch: " << qx;
     //   qDebug() << "yaw: " << qy;
     //   qDebug() << "roll: " << qz;
     //   qDebug() << "qw: " << qw;

        for (int y = 0; y < this->image->height(); ++y) {
            for (int x = 0; x < this->image->width(); ++x) {
                QRgb pixel = this->image->pixel(x, y);
                qint16 grayValue = this->depthImage->pixel(x,y);
                float xpos = static_cast<float>(x);
                float ypos = static_cast<float>(y);

                // Euclidean to Planar depth conversion
                scalling_factor = sqrt((x - this->cx) * (x - this->cx) + (y - this->cy) * (y - this->cy) + this->focal_x * this->focal_x) / this->focal_x;
                float zpos = static_cast<float>(grayValue) / scalling_factor;

               // float t_xpos = xpos * cos(yaw) - ypos * sin(yaw) + Tx;
               // float t_ypos = xpos * sin(yaw) + ypos * cos(yaw) + Ty;
               // float t_zpos = zpos + Tz;

                std::vector<std::vector<double>> matrix = {
                    {1 - 2*qy*qy - 2*qz*qz , 2*qx*qy - 2*qz*qw , 2*qx*qz + 2*qy*qw, Tx},
                    {2*qx*qy + 2*qz*qw , 1 - 2*qx*qx - 2*qz*qz, 2*qy*qz - 2*qx*qw, Ty},
                    {2*qx*qz - 2*qy*qw, 2*qy*qz + 2*qx*qw, 1 - 2*qx*qx - 2*qy*qy, Tz},
                    {0, 0, 0, 1}
                };


                float transformed_xpos = matrix[0][0] * xpos + matrix[0][1] * ypos + matrix[0][2] * zpos + matrix[0][3];
                float transformed_ypos = matrix[1][0] * xpos + matrix[1][1] * ypos + matrix[1][2] * zpos + matrix[1][3];
                float transformed_zpos = matrix[2][0] * xpos + matrix[2][1] * ypos + matrix[2][2] * zpos + matrix[2][3];

                xpos = transformed_xpos;
                ypos = transformed_ypos;
                zpos = transformed_zpos;



                QVector3D position(xpos, ypos, zpos);
                QColor color(pixel);

                this->octreeMap->insertPoint({xpos, ypos, zpos}, color);

                //pointCloud.append({ position, color });
            }
        }

        //this->octreeMap->printVoxels();

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

void OGLWidget::renderEllipsoids(const Voxel *voxel)
{
    if (voxel->children.empty() && (voxel->pointCount != 0))
    {
        // Ensure a shader program is in use
        glUseProgram(shaderProgram);

        // Compute the mean color
        Eigen::Vector3f meanColor = voxel->accumulatedColor / voxel->colorCount;

        // Set the color uniform in the fragment shader
        glUniform3fv(colorLocation, 1, meanColor.data());

        // This is a smallest voxel, render an ellipsoid based on its Gaussian parameters
        Eigen::Vector3f center = voxel->mean;

        std::cout<< center[0] << " " << center[1] << " " << center[2] << std::endl;

        Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(voxel->covariance);

        // Check if the covariance matrix is positive definite
        if (solver.info() != Eigen::Success) {
            // Handle non-positive definite covariance matrix (skip rendering or apply correction)
            return;

            // Option 2: Apply a correction (e.g., add a small value to the diagonal)
            // voxel->covariance += Eigen::Matrix3f::Identity() * epsilon;
            // solver.compute(voxel->covariance);
        }

        Eigen::Matrix3f axes = solver.eigenvectors();
        Eigen::Matrix3f scale = Eigen::Scaling(solver.eigenvalues().cwiseSqrt());

        Eigen::Affine3f transform;
        transform.fromPositionOrientationScale(center, Eigen::Quaternionf(axes), scale.diagonal());

        // Set the transformation matrix as a uniform variable in the shader
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, transform.matrix().data());

        glBindVertexArray(sphereVAO);

        // Draw the unit sphere (transformed to an ellipsoid)
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
    } else {
        // Traverse the child voxels
        for (const Voxel* child : voxel->children) {
            renderEllipsoids(child);
        }
    }
}

void OGLWidget::createUnitSphere()
{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;

    // Define the UV sphere geometry
    unsigned int numLatitudeSegments = 10; // Number of latitude segments (horizontal)
    unsigned int numLongitudeSegments = 10; // Number of longitude segments (vertical)

    for (unsigned int i = 0; i <= numLatitudeSegments; ++i) {
        float theta = i * M_PI / numLatitudeSegments; // Latitude
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (unsigned int j = 0; j <= numLongitudeSegments; ++j) {
            float phi = j * 2.0f * M_PI / numLongitudeSegments; // Longitude
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            // Vertex positions
            vertices.push_back(x); // X
            vertices.push_back(y); // Y
            vertices.push_back(z); // Z
        }
    }

    // Define indices
    for (unsigned int i = 0; i < numLatitudeSegments; ++i) {
        for (unsigned int j = 0; j < numLongitudeSegments; ++j) {
            unsigned int first = (i * (numLongitudeSegments + 1)) + j;
            unsigned int second = first + numLongitudeSegments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    sphereIndexCount = indices.size();
}

void OGLWidget::setupShaders()
{
    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compile errors
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders to create the shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for shader program linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Get the location of the transform uniform variable
    transformLocation = glGetUniformLocation(shaderProgram, "transform");
    if (transformLocation == -1) {
        std::cerr << "ERROR::SHADER::UNIFORM::TRANSFORM_NOT_FOUND\n";
    }

    // Get the location of the color uniform variable in the shader program
    colorLocation = glGetUniformLocation(shaderProgram, "color");
    if (colorLocation == -1) {
        std::cerr << "ERROR::SHADER::UNIFORM::COLOR_NOT_FOUND\n";
    }

    // Delete the vertex and fragment shaders as they're no longer needed once linked into the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
