#include <cglm\cglm.h>
#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "debug.h"
#include "timer.h"
#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "polygon.h"
#include "input.h"

static const int WIDTH = 1280;
static const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window);

static double deltaTime;
static double lastFrame;

static float zoom = 10.0f;

float RandomRange(float min, float max)
{
    return rand() * (max - min) / RAND_MAX + min;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWmonitor* monitor = NULL;//glfwGetPrimaryMonitor();

    window = glfwCreateWindow(WIDTH, HEIGHT, "Viewer", monitor, NULL);
    if (!window)
    {
        printf("Failed to create window!\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    InputInitialize(window);

    if (glewInit() != GLEW_OK)
        printf("Error initializing glew!\n");

    printf("%s\n", glGetString(GL_VERSION));

    PolygonInitialize();

    int maxUniformBlockSize;
    GLCall(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize));
    printf("Max buffer size is %d bytes\n", maxUniformBlockSize);

    //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    const int numLines = 4096;

    vec4* lineColors = malloc(sizeof(vec4) * numLines);
    LineInternal* lines = malloc(sizeof(LineInternal) * numLines);

    for (int i = 0; i < numLines; i++)
    {
        lines[i].a[0] = RandomRange(-50, 50);
        lines[i].a[1] = RandomRange(-50, 50);
        lines[i].b[0] = RandomRange(-50, 50);
        lines[i].b[1] = RandomRange(-50, 50);

        lineColors[i][0] = RandomRange(0, 1);
        lineColors[i][1] = RandomRange(0, 1);
        lineColors[i][2] = RandomRange(0, 1);
    }

    UniformBuffer lineColorsUB;
    UniformBufferInitialize(&lineColorsUB, lineColors, sizeof(vec4) * numLines, GL_STATIC_DRAW);
    //ShaderBindUniformBuffer(lineShaderId, "LineColors", &lineColorsUB);

    unsigned int linesVAO;
    VertexArrayInitialize(&linesVAO);
    VertexArrayBind(linesVAO);

    VertexBuffer lineVertices;
    VertexBufferInitialize(&lineVertices, lines, sizeof(LineInternal) * numLines);
    VertexBufferBind(&lineVertices);
    VertexAttribPointerFloats(0, 2);

    // CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);
    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    Rect* rect1 = PolygonRect((vec2) { 40, 20 }, 20, 40, (vec3) { 0.7f, 0.1f, 0.8f });
    Circle* circle1 = PolygonCircle((vec2) { 0, 0 }, 3, (vec3) { 0, 0.7f, 0 });

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // printf("%f\n", deltaTime);

        ProcessInput(window);

        //CameraRotate(yaw, pitch, 0.0f);

        vec2 scrollDelta;
        InputScrollDelta(scrollDelta);

        zoom += scrollDelta[1];
        zoom = fmax(zoom, 0.1f);
        CameraZoom(zoom);
        scrollDelta[1] = 0; // TODO: how to reset scoll delta when not scrolling

        mat4 m;
        CameraViewPerspectiveMatrix(m);
        PolygonUpdateViewPerspectiveMatrix(m);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        rect1->position[1] = 10 * sin(glfwGetTime() / 0.5f);

        PolygonRenderPolygons();

        // TimerStart();

        // ShaderUse(lineShaderId);
        // VertexArrayBind(linesVAO);
        // GLCall(glDrawArrays(GL_LINES, 0, 2 * numLines));

        // TimerStop();
        // printf("%f ms\n", TimerGetNanosecondsElapsed() / 1000000.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // TODO: Delete things??

    glfwTerminate();
    return 0;
}

void ProcessInput(GLFWwindow* window)
{
    float speed = zoom / 2.0f;
    vec3 movement = { 0.0f };

    InputGetAxes(window, movement); // TODO: passing vec3 as a vec2?

    if (InputKeyPressed(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}