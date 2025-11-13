///////////////////////////////////////////////////////////////////////////////
// ViewManager.cpp
// ===============
// Manages the camera view, projection, and user interactions (keyboard & mouse)
//
// Enhanced by: Karina Washington
// Date: November 2025 (Capstone Enhancement - CS 499)
// Added: Interactive camera controls (WASD), mouse movement, and zoom
///////////////////////////////////////////////////////////////////////////////

#include "ViewManager.h"

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Global variables used internally for camera control
namespace
{
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const char* g_ViewName = "view";
    const char* g_ProjectionName = "projection";

    Camera* g_pCamera = nullptr;

    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    bool bOrthographicProjection = false;
}

ViewManager::ViewManager(ShaderManager* pShaderManager)
{
    m_pShaderManager = pShaderManager;
    m_pWindow = NULL;

    // Initialize camera position and orientation
    g_pCamera = new Camera();
    g_pCamera->Position = glm::vec3(0.0f, 5.0f, 12.0f);
    g_pCamera->Front = glm::vec3(0.0f, -0.5f, -2.0f);
    g_pCamera->Up = glm::vec3(0.0f, 1.0f, 0.0f);
    g_pCamera->Zoom = 80.0f;
}

ViewManager::~ViewManager()
{
    m_pShaderManager = NULL;
    m_pWindow = NULL;
    if (g_pCamera != NULL) {
        delete g_pCamera;
        g_pCamera = NULL;
    }
}

GLFWwindow* ViewManager::CreateDisplayWindow(const char* windowTitle)
{
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle, NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    // Mouse and scroll callbacks enabled
    glfwSetCursorPosCallback(window, ViewManager::Mouse_Position_Callback);
    glfwSetScrollCallback(window, ViewManager::Mouse_Scroll_Callback);

    // Optional: Lock cursor for FPS-style free-look
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_pWindow = window;
    return window;
}

// CAMERA MOVEMENT CONTROLS (W A S D + Q/E + ESC close)
void ViewManager::ProcessKeyboardEvents()
{
    if (glfwGetKey(m_pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_pWindow, true);

    float speed = 5.0f * gDeltaTime;

    if (glfwGetKey(m_pWindow, GLFW_KEY_W) == GLFW_PRESS)
        g_pCamera->Position += speed * g_pCamera->Front;

    if (glfwGetKey(m_pWindow, GLFW_KEY_S) == GLFW_PRESS)
        g_pCamera->Position -= speed * g_pCamera->Front;

    if (glfwGetKey(m_pWindow, GLFW_KEY_A) == GLFW_PRESS)
        g_pCamera->Position -= glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * speed;

    if (glfwGetKey(m_pWindow, GLFW_KEY_D) == GLFW_PRESS)
        g_pCamera->Position += glm::normalize(glm::cross(g_pCamera->Front, g_pCamera->Up)) * speed;
}

// MOUSE MOVEMENT CAMERA LOOK AROUND
void ViewManager::Mouse_Position_Callback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;
    gLastX = xpos;
    gLastY = ypos;

    g_pCamera->ProcessMouseMovement(xoffset, yoffset);
}

// SCROLL WHEEL ZOOM CONTROL
void ViewManager::Mouse_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset)
{
    g_pCamera->ProcessMouseScroll(yoffset);
}

//  PREPARES CAMERA + VIEW + PROJECTION EVERY FRAME
void ViewManager::PrepareSceneView()
{
    float currentFrame = glfwGetTime();
    gDeltaTime = currentFrame - gLastFrame;
    gLastFrame = currentFrame;

    ProcessKeyboardEvents();

    glm::mat4 view = g_pCamera->GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(g_pCamera->Zoom), (float)WINDOW_WIDTH / WINDOW_HEIGHT, 0.1f, 100.0f);

    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setMat4Value(g_ViewName, view);
        m_pShaderManager->setMat4Value(g_ProjectionName, projection);
        m_pShaderManager->setVec3Value("viewPosition", g_pCamera->Position);
    }
}
