#include "glrenderer.h"

#include <QCoreApplication>
#include <QWindow>

#include "shaderloader.h"
#include "examplehelpers.h"
#include "cube.h"
#include "camera.h"
#include "terraingenerator.h"

GLRenderer::GLRenderer(QWidget *parent)
  : QOpenGLWidget(parent),
    m_ka(0.5),
    m_kd(0.5),
    m_ks(0.5),
    m_angleX(6),
    m_angleY(0),
    m_zoom(2)
{
    rebuildCameraMatrices(this->width(), this->height());

    m_prev_mouse_pos = glm::vec2(size().width()/2, size().height()/2);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;

    m_proj = glm::perspective(glm::radians(45.0), 1.0 * this->width() / this->height(), 0.01,100.0);
}

void GLRenderer::finish()
{
  glDeleteProgram(m_texture_shader);
  glDeleteProgram(m_phong_shader);
  glDeleteVertexArrays(1, &m_cube_vao);
  glDeleteBuffers(1, &m_cube_vbo);
  glDeleteVertexArrays(1, &m_fullscreen_vao);
  glDeleteBuffers(1, &m_fullscreen_vbo);

  // Task 35: Delete OpenGL memory here
  glDeleteTextures(1, &m_fbo_texture);
  glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
  glDeleteFramebuffers(1, &m_fbo);

  doneCurrent();
}

// Protected

void GLRenderer::initializeGL()
{
  m_timer = startTimer(1000/60);
  m_elapsedTimer.start();

  m_devicePixelRatio = this->devicePixelRatio();

  m_defaultFBO = 2;
  m_screen_width = size().width() * m_devicePixelRatio;
  m_screen_height = size().height() * m_devicePixelRatio;
  m_fbo_width = m_screen_width;
  m_fbo_height = m_screen_height;
  initTextureMap();


  // GLEW is a library which provides an implementation for the OpenGL API
  // Here, we are setting it up
  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  if (err != GLEW_OK) fprintf(stderr, "Error while initializing GLEW: %s\n", glewGetErrorString(err));
  fprintf(stdout, "Successfully initialized GLEW %s\n", glewGetString(GLEW_VERSION));
  
  // Set some default values for the OpenGL context
  glClearColor(0.52, .80, 0.92, 1);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND); // For fade out
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  // Load shaders
  m_texture_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/texture.frag");
  m_phong_shader   = ShaderLoader::createShaderProgram(":/resources/shaders/phong.vert", ":/resources/shaders/phong.frag");
  
  // Prepare example geometry for rendering later
  Cube* newCube = new Cube(glm::mat4(1.0f));
  m_cube_data = newCube->initialize(1, 1, 0.0f/16.0f,0.0f/16.0f);
  initializeExampleGeometry();

  // Task 9: Set the active texture slot to texture slot 0
  glActiveTexture(GL_TEXTURE0);

  // Task 6: Set min and mag filters' interpolation mode to linear
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Task 10: Set the texture.frag uniform for our texture
  // Activate the shader program
  glUseProgram(m_texture_shader);

  // Get the location of the texture sampler uniform
  GLint textureUniform = glGetUniformLocation(m_texture_shader, "myTexture");

  // Set the uniform to texture slot 0
  glUniform1i(textureUniform, 0); // 0 because the texture is bound to GL_TEXTURE0

  glUseProgram(0);

  // Task 11: Fix this "fullscreen" quad's vertex data

  std::vector<GLfloat> fullscreen_quad_data = {
      // Positions    // Texture Coords
      -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
      -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,
      1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
     1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
      -1.0f,  1.0f, 0.0f,   0.0f, 1.0f,
      1.0f, -1.0f, 0.0f,   1.0f, 0.0f
  };

  // Generate and bind a VBO and a VAO for a fullscreen quad
  glGenBuffers(1, &m_fullscreen_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
  glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
  glGenVertexArrays(1, &m_fullscreen_vao);
  glBindVertexArray(m_fullscreen_vao);

  // Task 14: modify the code below to add a second attribute to the vertex attribute array
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);
  glEnableVertexAttribArray(1); // Texture coordinate attribute
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

  // Unbind the fullscreen quad's VBO and VAO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  makeFBO();
}

void GLRenderer::makeFBO(){
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
  glGenTextures(1, &m_fbo_texture);
  glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_fbo_width, m_fbo_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);


    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLRenderer::paintGL()
{

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);


    // Task 28: Call glViewport
    glViewport(0,0, m_screen_width, m_screen_height);


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for(const auto& shape : cubesVector){
        glBindVertexArray(m_cube_vao);
        glUseProgram(m_phong_shader);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_grass_texture);
        glUniform1i(glGetUniformLocation(m_phong_shader, "myTexture"), 1);

        // Set uniforms for Phong vertex shader
        auto modelLoc = glGetUniformLocation(m_phong_shader, "modelMatrix");
        auto viewLoc  = glGetUniformLocation(m_phong_shader, "viewMatrix");
        auto projLoc  = glGetUniformLocation(m_phong_shader, "projMatrix");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &shape->getCTM()[0][0]);
        glUniformMatrix4fv(viewLoc,  1, GL_FALSE, &m_view[0][0]);
        glUniformMatrix4fv(projLoc,  1, GL_FALSE, &m_proj[0][0]);

        // Set uniforms for Phong fragment shader
        glUniform4f(glGetUniformLocation(m_phong_shader, "light.position"), 10, 0, 0, 1);
        glUniform3f(glGetUniformLocation(m_phong_shader, "light.color"), 1, 1,1);
        glUniform4f(glGetUniformLocation(m_phong_shader, "light.direction"), 0, 0, -1.0,1.0f);

        glUniform1f(glGetUniformLocation(m_phong_shader, "ka"),m_ka);
        glUniform1f(glGetUniformLocation(m_phong_shader, "kd"),m_kd);
        glUniform1f(glGetUniformLocation(m_phong_shader, "ks"),m_ks);
        // deciding which texture based off of layers
        glm::vec3 position = shape->getPosition();
        std::tuple<float, float, float> positionAbove = {position[0], position[1], position[2] + 1};
//        if (blockMap[positionAbove] == true){
//            continue;
//        }
        blockMap[{position[0], position[1], position[2]}] = true;
        float z_val = position[2];
        if (z_val < -18){
            if (shape->getID() == 0){
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::cobblestone].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::cobblestone].y);
            }
            else if(shape->getID() == 1) {
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::stone].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::stone].y);
            }

        }
        else if (z_val >= -18 && z_val < -15){
            glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::sand].x);
            glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::sand].y);
        }
        else {
            if(shape->getID() == 2){
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::logSide].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::logSide].y);

            }else{
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::grassTop].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::grassTop].y);

            }
        }

        // Draw
        glBindVertexArray(m_cube_vao);
        glDrawArrays(GL_TRIANGLES, 0, 6); // draw top face

        if (z_val < -18){
            if (shape->getID() == 0){
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::cobblestone].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::cobblestone].y);
            }
            else {
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::stone].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::stone].y);
            }
        }
        else if (z_val >= -18 && z_val < -15){
            glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::sand].x);
            glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::sand].y);
        }
        else {
            if(shape->getID() == 2){
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::logSide].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::logSide].y);
            }else{
                glUniform1f(glGetUniformLocation(m_phong_shader, "xOffset"), textureMap[blockType::dirt].x);
                glUniform1f(glGetUniformLocation(m_phong_shader, "yOffset"), textureMap[blockType::dirt].y);
            }

        }

        glDrawArrays(GL_TRIANGLES, 6, (m_cube_data.size() / 8) - 6); // draw remaining faces
        // Unbind
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }

    // Task 25: Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

    // Task 26: Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Task 27: Call paintTexture to draw our FBO color attachment texture | Task 31: Set bool parameter to true
    paintTexture(m_fbo_texture, true);
}

// Task 31: Update the paintTexture function signature
void GLRenderer::paintTexture(GLuint texture, bool postProcessing){
    glUseProgram(m_texture_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn
    glUniform1i(glGetUniformLocation(m_texture_shader, "postProcessing"),postProcessing);

    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0

    glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void GLRenderer::resizeGL(int w, int h){
    // Task 34: Delete Texture, Renderbuffer, and Framebuffer memory

    glDeleteTextures(1, &m_fbo_texture);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);
    glDeleteFramebuffers(1, &m_fbo);

    m_screen_width = size().width() * m_devicePixelRatio;
    m_screen_height = size().height() * m_devicePixelRatio;
    m_fbo_width = m_screen_width;
    m_fbo_height = m_screen_height;
    // Task 34: Regenerate your FBOs

    makeFBO();


    m_proj = glm::perspective(glm::radians(45.0), 1.0 * w / h, 0.01, 100.0);
}

// ============== DO NOT EDIT PAST THIS LINE ==================== //

void GLRenderer::initializeExampleGeometry()
{
  // Create geometry for a sphere
    // Task 1: Obtain image from filepath
    QString grassFilepath = QString(":/resources/images/minecraftTextureMap.png");

    m_image = QImage(grassFilepath);

    // Task 2: Format image to fit OpenGL
    m_image = m_image.convertToFormat(QImage::Format_RGBA8888).mirrored();

    // Task 3: Generate kitten texture
    glGenTextures(1, &m_grass_texture);

    // Task 9: Set the active texture slot to texture slot 1
    glActiveTexture(GL_TEXTURE1);

    // Task 4: Bind kitten texture
    glBindTexture(GL_TEXTURE_2D, m_grass_texture);

    // Task 5: Load image into kitten texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_image.width(), m_image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image.bits());


    // Task 6: Set min and mag filters' interpolation mode to linear
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Task 7: Unbind kitten texture
    glBindTexture(GL_TEXTURE_2D, 0);

  // Generate and bind the VBO
  glGenBuffers(1, &m_cube_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_cube_vbo);

  // Put data into the VBO

  cubesVector = generator.updatePlayerPosition(cameraPos, std::vector<Cube*>(), true);

  // Translate the matrix by (1, 1, 1)

  glBufferData(GL_ARRAY_BUFFER, m_cube_data.size() * sizeof(GLfloat), m_cube_data.data(), GL_STATIC_DRAW);

  // Generate and bind the VAO, with our VBO currently bound
  glGenVertexArrays(1, &m_cube_vao);
  glBindVertexArray(m_cube_vao);

  // Define VAO attributes
  glEnableVertexAttribArray(0); // handles Vertex positions
  glEnableVertexAttribArray(1); // handles Vertex normals
  glEnableVertexAttribArray(2); // handles Vertex normals

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void*>(0)); // position
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat))); // normal
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), reinterpret_cast<void *>(6 * sizeof(GLfloat))); // texture uv coor

  // Unbind
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}


void GLRenderer::rebuildCameraMatrices(int w, int h)
{
  // Initialize cameraUp
  cameraUp = glm::vec3(0, 0, 1);  // Assuming Z-axis is up

  // Calculate the eye position based on m_angleX, m_angleY, and m_zoom
  glm::mat4 rotX = glm::rotate(glm::radians(-10 * m_angleX), glm::vec3(0, 0, 1));
  glm::vec3 eye = glm::vec3(2, 0, 0);  // Initial eye position
  eye = glm::vec3(rotX * glm::vec4(eye, 1));

  glm::mat4 rotY = glm::rotate(glm::radians(-10 * m_angleY), glm::cross(glm::vec3(0, 0, 1), eye));
  eye = glm::vec3(rotY * glm::vec4(eye, 1));

  eye *= m_zoom;

  // Set cameraPos to the calculated eye position
  cameraPos = glm::vec3(0, 0, eye.z);

  // Calculate the direction the camera is facing
  cameraFront = glm::normalize(glm::vec3(0, 0, 0) - eye); // Assuming the camera is looking at (0,0,0)

  // Create the view matrix
  m_view = glm::lookAt(eye, glm::vec3(0, 0, 0), cameraUp);

  // Create the projection matrix
  m_proj = glm::perspective(glm::radians(45.0), 1.0 * w / h, 0.01, 100.0);

  update();
}

void GLRenderer::keyPressEvent(QKeyEvent *event) {
  m_keyMap[Qt::Key(event->key())] = true;
}

void GLRenderer::keyReleaseEvent(QKeyEvent *event) {
  m_keyMap[Qt::Key(event->key())] = false;
}

void GLRenderer::mousePressEvent(QMouseEvent *event) {
  if (event->buttons().testFlag(Qt::LeftButton)) {
      m_mouseDown = true;
      m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
  }
}

void GLRenderer::mouseReleaseEvent(QMouseEvent *event) {
  if (!event->buttons().testFlag(Qt::LeftButton)) {
      m_mouseDown = false;
  }
}

void GLRenderer::mouseMoveEvent(QMouseEvent *event) {
      int centerX = width() / 2;
      int centerY = height() / 2;
      int posX = event->position().x();
      int posY = event->position().y();
      int deltaX = posX - m_prev_mouse_pos.x;
      int deltaY = posY - m_prev_mouse_pos.y;
      m_prev_mouse_pos = glm::vec2(posX, posY);
      if (m_mouseSnap) { // change in glrenderer.h if you don't want the mouse to stay in the middle of the screen
          QCursor::setPos(mapToGlobal(QPoint(centerX, centerY)));
          if (posX > centerX && deltaX < 0){
            return;
          }
          if (posX < centerX && deltaX > 0){
            return;
          }
          if (posY > centerY && deltaY < 0){
            return;
          }
          if (posY < centerY && deltaY > 0){
            return;
          }
          deltaX *= 3;
          deltaY *= 3;
      }

      // rotation speed from assignment
      float rotationSpeed = m_rotationSpeed;

      // Calculate the new front vector
      glm::mat4 rotator;

      // Rotate around the vertical (world Y) axis
      if (deltaX != 0) {
          float angleX = -deltaX * rotationSpeed;
          rotator = glm::rotate(glm::mat4(1.0f), glm::radians(angleX), glm::vec3(0, 0, 1));
          cameraFront = glm::vec3(rotator * glm::vec4(cameraFront, 0.0));
          cameraUp = glm::vec3(rotator * glm::vec4(cameraUp, 0.0));
      }

      // Rotate around the horizontal axis (cross product of camera's up and front)
      if (deltaY != 0) {
          glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));
          float angleY = -deltaY * rotationSpeed;
          rotator = glm::rotate(glm::mat4(1.0f), glm::radians(angleY), right);
          cameraFront = glm::vec3(rotator * glm::vec4(cameraFront, 0.0));
          cameraUp = glm::vec3(rotator * glm::vec4(cameraUp, 0.0));
      }

      // normalize vectors
      cameraFront = glm::normalize(cameraFront);
      cameraUp = glm::normalize(cameraUp);

      // update camera with the new vectors
      updateCamera();
      update();

      QCursor::setPos(mapToGlobal(QPoint(width() / 2, height() / 2)));
}

void GLRenderer::timerEvent(QTimerEvent *event) {
  int elapsedms   = m_elapsedTimer.elapsed();
  float deltaTime = elapsedms * 0.001f;
  m_elapsedTimer.restart();

  // speed of 5, multiply by deltatime to account for specs from handout
  float speed = movementSpeed;
  float distance = speed * deltaTime;

  float currentHeight = cameraPos.z;

  // if s is pressed move backwards in x-y look direction
  if (m_keyMap[Qt::Key_S]) {
      glm::vec3 newPos = cameraPos - distance * glm::normalize(glm::vec3(cameraFront.x, cameraFront.y, 0.f));
      float newHeight = generator.getGroundHeight(newPos);
      if (newHeight <= currentHeight) {
          cameraPos = newPos;
      }
  }

  // if w is pressed move forwards in x-y look direction
  if (m_keyMap[Qt::Key_W]) {
      glm::vec3 newPos = cameraPos + distance * glm::normalize(glm::vec3(cameraFront.x, cameraFront.y, 0.f));
      float newHeight = generator.getGroundHeight(newPos);
      if (newHeight <= currentHeight) {
          cameraPos = newPos;
      }
  }

  // if A is pressed move to the left of the cross product of up and front vectors
  if (m_keyMap[Qt::Key_A]) {
      glm::vec3 newPos = cameraPos - glm::normalize(glm::cross(cameraFront, cameraUp)) * distance;
      float newHeight = generator.getGroundHeight(newPos);
      if (newHeight <= currentHeight) {
          cameraPos = newPos;
      }

  }
  // if D is pressed move to the right of the cross product of up and front vectors
  if (m_keyMap[Qt::Key_D]) {
      glm::vec3 newPos = cameraPos + glm::normalize(glm::cross(cameraFront, cameraUp)) * distance;
      float newHeight = generator.getGroundHeight(newPos);
      if (newHeight <= currentHeight) {
          cameraPos = newPos;
      }
  }
  // translate camera among world space vector up
  if (m_keyMap[Qt::Key_Space]) {
      if (!inTheAir){
          inTheAir = true;
          velocity = reboundVelocity;
      }
  }

  // Jump movement stuff
  float groundPosition = generator.getGroundHeight(cameraPos);
  velocity = fmax(velocity + acceleration*deltaTime, minimumVelocity);
  float newZPosition = cameraPos.z + velocity*deltaTime;
  if (newZPosition < groundPosition) {
      velocity = 0;
      inTheAir = false;
      cameraPos.z = groundPosition;
  }
  else cameraPos.z = newZPosition;

  // update player position in terrain generator so it knows what chunks to load
  cubesVector = generator.updatePlayerPosition(cameraPos, cubesVector, false);

  // recalculate viewMatrix with the updated parameters
  updateCamera();
  update(); // asks for a PaintGL() call to occur
}

void GLRenderer::updateCamera() {
  m_view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  // Update m_proj here if needed, for example:
  update();
}

void GLRenderer::initTextureMap(){
  for (float row = 0; row < 16; row++){
      for (float col = 0; col < 16; col++){
          textureMap[(row * 16) + col] = glm::vec2((col / 16.f), (15 - row) / 16.f);
      }
  }
}

