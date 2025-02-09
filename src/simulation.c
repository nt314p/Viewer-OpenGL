#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include "input.h"
#include "debug.h"
#include "polygon.h"
#include "camera.h"

GLFWwindow* Initialize();
void Update(float deltaTime);

// Needs to be called
GLFWwindow* SimInitWindow(int width, int height, const char* title, int isFullscreen)
{
    GLFWwindow* window;

    if (!glfwInit())
    {
        printf("Failed to initialize glfw!\n");
        return NULL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor* monitor = NULL;
    if (isFullscreen) monitor = glfwGetPrimaryMonitor();

    window = glfwCreateWindow(width, height, title, monitor, NULL);
    if (!window)
    {
        printf("Failed to create window!\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (glewInit() != GLEW_OK)
    {
        printf("Failed to initialize glew!\n");
        return NULL;
    }

    InputInitialize(window);
    PolygonInitialize();

    return window;
}

int main()
{
    GLFWwindow* window = Initialize();

    double lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentFrameTime = glfwGetTime();
        double deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        Update(deltaTime);

        // If camera input needs to be changed here... use update and render
        // Update?
        mat4 m;
        CameraViewPerspectiveMatrix(m);
        PolygonUpdateViewPerspectiveMatrix(m);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        // Render?

        PolygonRenderPolygons();
        InputReset();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    printf("Exiting...\n");

    glfwTerminate();
    return 0;
}