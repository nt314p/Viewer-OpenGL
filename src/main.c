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

const char* BasicFragShaderPath = "shaders/BasicFrag.glsl";
const char* RectVertShaderPath = "shaders/InstancedRectangle.glsl";
const char* CircleVertShaderPath = "shaders/InstancedCircle.glsl";
const char* LineVertShaderPath = "shaders/InstancedLine.glsl";

const int WIDTH = 1280;
const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window);
static void MouseCallback(GLFWwindow* window, double x, double y);
static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

static double deltaTime;
static double lastFrame;
static vec2 mouseCoords;
static vec2 scrollDelta;
static vec2 mouseDelta;

static float yaw;
static float pitch;

float zoom = 10.0f;

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
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    if (glewInit() != GLEW_OK)
        printf("Error initializing glew!\n");

    printf("%s\n", glGetString(GL_VERSION));

    unsigned int rectShaderId = ShaderCreate(RectVertShaderPath, BasicFragShaderPath);
    unsigned int circleShaderId = ShaderCreate(CircleVertShaderPath, BasicFragShaderPath);
    unsigned int lineShaderId = ShaderCreate(LineVertShaderPath, BasicFragShaderPath);

    PolygonInitialize();
    PolygonBindUnitSquare();

    mat4 vpMatrix;
    UniformBuffer vpMatrixUB;
    UniformBufferInitialize(&vpMatrixUB, vpMatrix, sizeof(mat4), GL_DYNAMIC_DRAW);
    ShaderBindUniformBuffer(rectShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(circleShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(lineShaderId, "Matrices", &vpMatrixUB);

    int maxUniformBlockSize;
    GLCall(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize));
    printf("Max buffer size is %d bytes\n", maxUniformBlockSize);

    //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    const int numCircles = 2048;

    Circle* circlesBuffer = malloc(sizeof(Circle) * numCircles);
    for (int i = 0; i < numCircles; i++)
    {
        circlesBuffer[i].radius = RandomRange(0.1f, 2);
        circlesBuffer[i].position[0] = RandomRange(-50, 50);
        circlesBuffer[i].position[1] = RandomRange(-50, 50);

        circlesBuffer[i].color[0] = RandomRange(0, 1);
        circlesBuffer[i].color[1] = RandomRange(0, 1);
        circlesBuffer[i].color[2] = RandomRange(0, 1);
    }

    UniformBuffer circles;
    UniformBufferInitialize(&circles, circlesBuffer, sizeof(Circle) * numCircles, GL_DYNAMIC_DRAW);
    ShaderBindUniformBuffer(circleShaderId, "Circles", &circles);

    const int numRects = 2048;
    Rect* rectsBuffer = malloc(sizeof(Rect) * numRects);
    memset(rectsBuffer, 0, sizeof(Rect) * numRects);

    for (int i = 0; i < numRects; i++)
    {
        rectsBuffer[i].width = RandomRange(0.1f, 2);
        rectsBuffer[i].height = RandomRange(0.1f, 2);
        rectsBuffer[i].position[0] = RandomRange(-50, 50);
        rectsBuffer[i].position[1] = RandomRange(-50, 50);

        rectsBuffer[i].color[0] = RandomRange(0, 1);
        rectsBuffer[i].color[1] = RandomRange(0, 1);
        rectsBuffer[i].color[2] = RandomRange(0, 1);
    }

    UniformBuffer rects;
    UniformBufferInitialize(&rects, rectsBuffer, sizeof(Rect) * numRects, GL_STATIC_DRAW);
    ShaderBindUniformBuffer(rectShaderId, "Rectangles", &rects);

    const int numLines = 4096;

    vec4* lineColors = malloc(sizeof(vec4) * numLines);
    Line* lines = malloc(sizeof(Line) * numLines);

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
    ShaderBindUniformBuffer(lineShaderId, "LineColors", &lineColorsUB);

    unsigned int linesVAO;
    VertexArrayInitialize(&linesVAO);
    VertexArrayBind(linesVAO);

    VertexBuffer lineVertices;
    VertexBufferInitialize(&lineVertices, lines, sizeof(Line) * numLines);
    VertexBufferBind(&lineVertices);
    VertexAttribPointerFloats(0, 2);

    // CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);
    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    while (!glfwWindowShouldClose(window))
    {
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // printf("%f\n", deltaTime);

        ProcessInput(window);

        //CameraRotate(yaw, pitch, 0.0f);

        zoom += scrollDelta[1];
        zoom = fmax(zoom, 0.1f);
        CameraZoom(zoom);
        scrollDelta[1] = 0;

        CameraViewPerspectiveMatrix(vpMatrix);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        UniformBufferUpdate(&vpMatrixUB);

        for (int i = 0; i < numCircles; i++)
        {
            glm_vec2_rotate(circlesBuffer[i].position, deltaTime * 0.1f, circlesBuffer[i].position);
        }


        // UniformBufferUpdate(&circles);
        // ShaderUse(circleShaderId);
        // PolygonBindUnitCircle();
        // GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 52, numCircles));

        // ShaderUse(rectShaderId);
        // PolygonBindUnitSquare();
        // GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, numRects));


        TimerStart();

        ShaderUse(lineShaderId);
        VertexArrayBind(linesVAO);
        GLCall(glDrawArrays(GL_LINES, 0, 2 * numLines));

        TimerStop();
        printf("%f ms\n", TimerGetNanosecondsElapsed() / 1000000.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Delete things??

    glfwTerminate();
    return 0;
}

static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    scrollDelta[0] = xOffset;
    scrollDelta[1] = yOffset;
}

void MouseCallback(GLFWwindow* window, double x, double y)
{
    const float sensitivity = 6.0f;

    static float prevX, prevY;
    static bool firstCall = true;

    if (firstCall)
    {
        prevX = x;
        prevY = y;
        firstCall = false;
    }

    float dx = x - prevX;
    float dy = -(y - prevY);

    prevX = x;
    prevY = y;

    mouseCoords[0] = x;
    mouseCoords[1] = y;
    mouseDelta[0] = dx;
    mouseDelta[1] = dy;

    yaw -= dx * deltaTime * sensitivity;
    pitch += dy * deltaTime * sensitivity;
}

int KeyPressed(GLFWwindow* window, int key);

inline int KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void ProcessInput(GLFWwindow* window)
{
    float speed = zoom / 2.0f;
    vec3 movement = { 0.0f };

    if (KeyPressed(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (KeyPressed(window, GLFW_KEY_D))
    {
        movement[0] += 1;
    }

    if (KeyPressed(window, GLFW_KEY_A))
    {
        movement[0] -= 1;
    }

    if (KeyPressed(window, GLFW_KEY_W))
    {
        movement[1] += 1;
    }

    if (KeyPressed(window, GLFW_KEY_S))
    {
        movement[1] -= 1;
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}