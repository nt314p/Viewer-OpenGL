#include <cglm\cglm.h>
#include <cglm\version.h>
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

struct Ball
{
    Circle* circle;
    Line* line;
    vec2 position;
    vec2 velocity;
} typedef Ball;

static void UpdateBall(Ball* ball, float deltaTime)
{
    glm_vec2_copy(ball->position, ball->circle->position);

    glm_vec2_copy(ball->position, ball->line->a);
    glm_vec2_copy(ball->position, ball->line->b);
    glm_vec2_add(ball->line->b, ball->velocity, ball->line->b);
}

static bool CircleCircleCollisionTime(vec2 p1, vec2 v1, float r1, vec2 p2, vec2 v2, float r2,
    vec2 outTimes)
{
    vec2 dp, dv;
    glm_vec2_sub(p2, p1, dp); // delta of the positions
    glm_vec2_sub(v2, v1, dv); // delta of the velocities;

    // Collision times is a quadratic function of t
    // a * t^2 + b * t + c = 0
    float a = glm_vec2_dot(dv, dv);
    float b = glm_vec2_dot(dp, dv);
    float c = glm_vec2_dot(dp, dp);

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;

    float a2 = 2 * a;
    float v = sqrtf(discriminant) / a2;
    float u = -b / a2;

    outTimes[0] = u - v; // TODO: check for negative collision times
    outTimes[1] = u + v;

    return true;
}

// Requires that `lineDir` is normalized
// TODO: write simplified methods for horizontal or vertical lines
static float CircleLineCollisionTime(vec2 circleP, vec2 circleV, vec2 lineP, vec2 lineDir)
{

    return 0;
}

float RandomRange(float min, float max)
{
    return rand() * (max - min) / RAND_MAX + min;
}

static GLFWwindow* InitializeWindow(int width, int height, int isFullscreen)
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

    window = glfwCreateWindow(width, height, "Viewer", monitor, NULL);
    if (!window)
    {
        printf("Failed to create window!\n");
        glfwTerminate();
        return NULL;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    return window;
}

int main(void)
{
    GLFWwindow* window = InitializeWindow(WIDTH, HEIGHT, false);
    if (window == NULL) return -1;

    if (glewInit() != GLEW_OK)
        printf("Error initializing glew!\n");

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("cglm version: %d.%d.%d\n",
        CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH);

    InputInitialize(window);
    PolygonInitialize();

    //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    // CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);
    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    vec3 wallColor = { 0.557, 0.675, 0.769 };

    Rect* floor = PolygonRect((vec2) { 0, -1 }, 44, 2, wallColor);
    Rect* leftWall = PolygonRect((vec2) { -21, 10 }, 2, 20, wallColor);
    Rect* rightWall = PolygonRect((vec2) { 21, 10 }, 2, 20, wallColor);
    Rect* ceiling = PolygonRect((vec2) { 0, 21 }, 44, 2, wallColor);
    Circle* circle1 = PolygonCircle((vec2) { 0, 10 }, 2, (vec3) { 0.471, 0.722, 0.435 });
    Line* velIndicator = PolygonLine((vec2) {0, 0}, (vec2) {0, 0}, (vec3){1, 1, 1});

    Ball ball;
    ball.circle = circle1;
    ball.line = velIndicator;
    glm_vec2_copy((vec2) {0, 10}, ball.position);
    glm_vec2_copy((vec2) {10.0f, 3.0f}, ball.velocity);

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

        mat4 m;
        CameraViewPerspectiveMatrix(m);
        PolygonUpdateViewPerspectiveMatrix(m);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        InputMouseCoords(ball.position);

        UpdateBall(&ball, deltaTime);
        //ball.velocity[1] -= 10 * deltaTime;

        // if (ball.position[1] < 2) ball.velocity[1] *= -1;
        // if (ball.position[1] > 18) ball.velocity[1] *= -1;
        // if (ball.position[0] > 18) ball.velocity[0] *= -1;
        // if (ball.position[0] < -18) ball.velocity[0] *= -1;

        // glm_vec2_muladds(ball.velocity, deltaTime, ball.position);

        PolygonRenderPolygons();

        // TimerStart();

        // ShaderUse(lineShaderId);
        // VertexArrayBind(linesVAO);
        // GLCall(glDrawArrays(GL_LINES, 0, 2 * numLines));

        // TimerStop();
        // printf("%f ms\n", TimerGetNanosecondsElapsed() / 1000000.0f);

        InputReset();

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