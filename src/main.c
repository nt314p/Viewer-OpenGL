#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "model_parser.h"
#include "polygon.h"
#include <cglm\cglm.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>

const int WIDTH = 1280;
const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window);
static void MouseCallback(GLFWwindow* window, double x, double y);
static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

static float deltaTime;
static float lastFrame;
static vec2 mouseCoords;
static vec2 scrollDelta;
static vec2 mouseDelta;

float yaw;
float pitch;

float RandomRange(float min, float max)
{
    return rand() * (max - min) / RAND_MAX + min;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Viewer", NULL, NULL);
    if (!window)
    {
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

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    unsigned int rectShaderId = ShaderCreate("shaders/InstancedRectangle.glsl", "shaders/BasicFrag.glsl");
    unsigned int circleShaderId = ShaderCreate("shaders/InstancedCircle.glsl", "shaders/BasicFrag.glsl");

    GLCall(glUseProgram(rectShaderId));

    mat4 vpMatrix;
    UniformBuffer vpMatrixUB;
    UniformBufferInitialize(&vpMatrixUB, vpMatrix, sizeof(mat4), GL_DYNAMIC_DRAW);

    int mvpMatrixId = ShaderGetUniformId(rectShaderId, "mvpMatrix");
    int rectsBufferIndex = ShaderGetUniformBlockIndex(rectShaderId, "Rectangles");

    int mvpMatrixId2 = ShaderGetUniformId(circleShaderId, "mvpMatrix");
    int circlesBufferIndex = ShaderGetUniformBlockIndex(circleShaderId, "Circles");

    int maxUniformBlockSize;
    GLCall(glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize));

    //printf("Circle buffer id is: %d\n", circlesBufferIndex);
    printf("Max buffer size is %d bytes\n", maxUniformBlockSize);

    // GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    PolygonInitialize();

    const int numCircles = 200;

    Circle* circlesBuffer = malloc(sizeof(Circle) * numCircles);
    for (int i = 0; i < numCircles; i++)
    {
        circlesBuffer[i].radius = RandomRange(0.1f, 2);
        circlesBuffer[i].position[0] = RandomRange(-10, 10);
        circlesBuffer[i].position[1] = RandomRange(-10, 10);

        circlesBuffer[i].color[0] = RandomRange(0, 1);
        circlesBuffer[i].color[1] = RandomRange(0, 1);
        circlesBuffer[i].color[2] = RandomRange(0, 1);
    }

    UniformBuffer circles;
    UniformBufferInitialize(&circles, circlesBuffer, sizeof(Circle) * numCircles, GL_DYNAMIC_DRAW);
    UniformBufferBindPoint(&circles, circlesBufferIndex);

    const int numRects = 200;
    Rectangle* rectsBuffer = malloc(sizeof(Rectangle) * numRects);
    for (int i = 0; i < numRects; i++)
    {
        rectsBuffer[i].width = RandomRange(0.1f, 2);
        rectsBuffer[i].height = RandomRange(0.1f, 2);
        rectsBuffer[i].position[0] = RandomRange(-10, 10);
        rectsBuffer[i].position[1] = RandomRange(-10, 10);

        rectsBuffer[i].color[0] = RandomRange(0, 1);
        rectsBuffer[i].color[1] = RandomRange(0, 1);
        rectsBuffer[i].color[2] = RandomRange(0, 1);
    }

    UniformBuffer rects;
    // TODO: investigate why using GL_DYNAMIC_DRAW causes flickering
    UniformBufferInitialize(&rects, rectsBuffer, sizeof(Rectangle) * numRects, GL_STATIC_DRAW);
    UniformBufferBindPoint(&rects, rectsBufferIndex);

    // CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);
    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    float zoom = 10.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // printf("%d\n", (int)(1 / deltaTime));

        ProcessInput(window);

        // CameraRotate(yaw, pitch, 0.0f);

        zoom += scrollDelta[1];
        zoom = fmax(zoom, 0.1f);
        CameraZoom(zoom);
        scrollDelta[1] = 0;

        CameraViewPerspectiveMatrix(vpMatrix);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        // for (int i = 0; i < numCircles; i++) {
        //     glm_vec2_rotate(circlesBuffer[i].position, RandomRange(-0.001, 0.001), circlesBuffer[i].position);
        // }

        // UniformBufferUpdate(&circles);

        //PolygonBindUnitSquare();

        PolygonBindUnitSquare();

        GLCall(glUniformMatrix4fv(vpMatrixId, 1, GL_FALSE, vpMatrix[0]));
        GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, numRects));

        // VertexBufferBind(&circles->vertexBuffer);
        // for (int i = 0; i < numCircles; i++)
        // {
        //     Polygon* circle = circles + i;
        //     glm_mat4_mul(vpMatrix, circle->transform, mvpMatrix);
        //     GLCall(glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, mvpMatrix[0]));
        //     PolygonDraw(circle);
        // }

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

    yaw -= dx * deltaTime * 6;
    pitch += dy * deltaTime * 6;
    // printf("dx: %f; dy: %f\n", dx, dy);
}

int KeyPressed(GLFWwindow* window, int key);

inline int KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void ProcessInput(GLFWwindow* window)
{
    float speed = 10.0f;
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