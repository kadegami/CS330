// Karina Washington , Updated for CS-499 Enhancement (Novenber  2025)
// Added interactive camera orbit + light control features

#include <iostream>         // error handling and output
#include <cstdlib>          // EXIT_FAILURE

#include <GL/glew.h>        // GLEW library
#include "GLFW/glfw3.h"     // GLFW library

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SceneManager.h"
#include "ViewManager.h"
#include "ShapeMeshes.h"
#include "ShaderManager.h"

// Namespace for declaring global variables
namespace
{
    const char* const WINDOW_TITLE = "7-1 FinalProject and Milestones";

    GLFWwindow* g_Window = nullptr;

    SceneManager* g_SceneManager = nullptr;
    ShaderManager* g_ShaderManager = nullptr;
    ViewManager* g_ViewManager = nullptr;

    // -------------------------------
    // NEW INTERACTIVE GLOBAL CONTROLS
    // -------------------------------
    static float gYawDegrees = 0.0f;   // controls camera left/right rotation
    static float gRadius = 8.0f;       // distance of camera from scene
    static float gLightIntensity = 1.0f; // brightness of light source
}

// Function declarations
bool InitializeGLFW();
bool InitializeGLEW();

// ---------------------------------------------------------
// NEW: Keyboard Input Processing (Added Feb 2025)
// ---------------------------------------------------------
static void ProcessInput(GLFWwindow* window)
{
    // Rotate camera horizontally
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)  gYawDegrees -= 0.5f;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) gYawDegrees += 0.5f;

    // Zoom camera in/out
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)    gRadius -= 0.05f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)  gRadius += 0.05f;

    if (gRadius < 3.0f)  gRadius = 3.0f;
    if (gRadius > 20.0f) gRadius = 20.0f;

    // Adjust light intensity
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) gLightIntensity += 0.02f;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) gLightIntensity -= 0.02f;

    if (gLightIntensity < 0.2f) gLightIntensity = 0.2f;
    if (gLightIntensity > 2.0f) gLightIntensity = 2.0f;
}

/***********************************************************
 *  main(int, char*)
 ***********************************************************/
int main(int argc, char* argv[])
{
    if (InitializeGLFW() == false)
    {
        return(EXIT_FAILURE);
    }

    g_ShaderManager = new ShaderManager();
    g_ViewManager = new ViewManager(g_ShaderManager);
    g_Window = g_ViewManager->CreateDisplayWindow(WINDOW_TITLE);

    if (InitializeGLEW() == false)
    {
        return(EXIT_FAILURE);
    }

    g_ShaderManager->LoadShaders(
        "../../Utilities/shaders/vertexShader.glsl",
        "../../Utilities/shaders/fragmentShader.glsl");
    g_ShaderManager->use();

    g_SceneManager = new SceneManager(g_ShaderManager);
    g_SceneManager->PrepareScene();

    // -----------------------------------------------------
    // MAIN RENDER LOOP
    // -----------------------------------------------------
    while (!glfwWindowShouldClose(g_Window))
    {
        // 💡 NEW: Process keyboard input
        ProcessInput(g_Window);

        // 💡 NEW: Apply camera orbit & light adjustments
        g_ViewManager->SetOrbitCamera(gYawDegrees, gRadius);
        g_ShaderManager->SetLightIntensity(gLightIntensity);

        // Enable z-depth
        glEnable(GL_DEPTH_TEST);

        // Clear frame & depth
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        g_ViewManager->PrepareSceneView();
        g_SceneManager->RenderScene();

        glfwSwapBuffers(g_Window);
        glfwPollEvents();
    }

    // cleanup
    if (g_SceneManager) { delete g_SceneManager; g_SceneManager = NULL; }
    if (g_ViewManager) { delete g_ViewManager; g_ViewManager = NULL; }
    if (g_ShaderManager) { delete g_ShaderManager; g_ShaderManager = NULL; }

    exit(EXIT_SUCCESS);
}

/***********************************************************
 *  InitializeGLFW()
 ***********************************************************/
bool InitializeGLFW()
{
    glfwInit();

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    return(true);
}

/***********************************************************
 *  InitializeGLEW()
 ***********************************************************/
bool InitializeGLEW()
{
    GLenum GLEWInitResult = glewInit();
    if (GLEW_OK != GLEWInitResult)
    {
        std::cerr << glewGetErrorString(GLEWInitResult) << std::endl;
        return false;
    }

    std::cout << "INFO: OpenGL Successfully Initialized\n";
    std::cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << "\n" << std::endl;
    return(true);
}