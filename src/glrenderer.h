#pragma once

#include "GL/glew.h"
#include <QOpenGLWidget>

#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/glm.hpp"
#include "src/camera.h"
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QMouseEvent>
#include <QKeyEvent>



class GLRenderer : public QOpenGLWidget
{

public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer
public:
    GLRenderer(QWidget *parent = nullptr);

    void finish();

protected:
    void initializeGL()         override; // Called once before the first call to paintGL() or resizeGL()
    void paintGL()              override; // Called whenever the widget needs to be painted
    void resizeGL(int w, int h) override; // Called whenever widget has been resized

private:
    void makeFBO();
    // Task 30: Update the paintTexture function signature
    void paintTexture(GLuint texture, bool postProcessing);
    void initializeExampleGeometry();
    void paintExampleGeometry();

    void rebuildCameraMatrices(int w, int h);

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void updateCamera();

    int m_devicePixelRatio;
    GLuint m_defaultFBO;
    int m_fbo_width;
    int m_fbo_height;
    int m_screen_width;
    int m_screen_height;

    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;
    float cameraSpeed;


    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;

    SceneCameraData m_renderData;
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not



    GLuint m_texture_shader;
    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;
    QImage m_image;
    GLuint m_kitten_texture;
    GLuint m_fbo;
    GLuint m_fbo_texture;
    GLuint m_fbo_renderbuffer;

    GLuint m_phong_shader;
    std::vector<float> m_sphere_data;
    GLuint m_sphere_vbo;
    GLuint m_sphere_vao;

    glm::mat4 m_model = glm::mat4(1);
    glm::mat4 m_view = glm::mat4(1);
    glm::mat4 m_proj = glm::mat4(1);
    float m_ka;
    float m_kd;
    float m_ks;
    float m_angleX;
    float m_angleY;
    float m_zoom;
};
