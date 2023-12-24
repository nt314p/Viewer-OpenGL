#include <cglm\cglm.h>
#include <cglm\version.h>
#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include "debug.h"
#include "timer.h"
#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "polygon.h"
#include "input.h"
#include "color.h"
#include "physics.h"

static const int WIDTH = 1280;
static const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window);

static double deltaTime;
static double lastFrameTime;

static float zoom = 10.0f;

struct Ball
{
    Circle* circle;
    Line* line;
    vec2 position;
    vec2 velocity;
    float time;
    float hitTime;
} typedef Ball;

static void UpdateBall(Ball* ball, float time)
{
    float deltaTime = time - ball->time;
    glm_vec2_copy(ball->position, ball->circle->position);
    glm_vec2_muladds(ball->velocity, deltaTime, ball->circle->position);

    glm_vec2_copy(ball->circle->position, ball->line->a);
    glm_vec2_copy(ball->velocity, ball->line->b);
    float length = glm_vec2_norm(ball->velocity);
    ball->line->length = length;
    ball->line->b[0] /= length;
    ball->line->b[1] /= length;
}

// TODO: is this uniformly distributed?
static float RandomRange(float min, float max)
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

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

    Circle* circle1 = PolygonCircle((vec2) { 0, 10 }, 2, (vec3)CLR_MEDIUMSEAGREEN);
    Circle* ghostCircle = PolygonCircle((vec2) { 0, 10 }, 2, (vec3)CLR_PALEGREEN);
    Line* velIndicator = PolygonLine(GLM_VEC2_ZERO, GLM_VEC2_ZERO, (vec3)CLR_WHITE);

    Line* bounds = PolygonLines(4);

    for (int i = 0; i < 4; i++)
    {
        glm_vec3_copy((vec3)CLR_ORANGE, (bounds + i)->color);
        (bounds + i)->length = 20;
    }

    glm_vec2_copy((vec2) { -10, 10 }, bounds[0].a);
    glm_vec2_copy((vec2) { 1, 0 }, bounds[0].b);

    glm_vec2_copy((vec2) { 10, -10 }, bounds[1].a);
    glm_vec2_copy((vec2) { 0, 1 }, bounds[1].b);

    glm_vec2_copy((vec2) { -10, -10 }, bounds[2].a);
    glm_vec2_copy((vec2) { 1, 0 }, bounds[2].b);

    glm_vec2_copy((vec2) { -10, -10 }, bounds[3].a);
    glm_vec2_copy((vec2) { 0, 1 }, bounds[3].b);

    Ball ball = { 0 };
    ball.circle = circle1;
    ball.line = velIndicator;
    ball.time = 0.0f;
    ball.hitTime = -1.0f;
    glm_vec2_copy((vec2) { 0, 0 }, ball.position);
    glm_vec2_copy((vec2) { 20, 6 }, ball.velocity);

    Interaction interaction;

    double startTime = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        double currentFrameTime = glfwGetTime();
        double currentSimTime = currentFrameTime - startTime;
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        ProcessInput(window);

        vec2 scrollDelta;
        InputScrollDelta(scrollDelta);

        zoom += scrollDelta[1];
        zoom = fmax(zoom, 0.1f);
        CameraZoom(zoom);

        mat4 m;
        CameraViewPerspectiveMatrix(m);
        PolygonUpdateViewPerspectiveMatrix(m);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        vec2 mouseCoords;
        InputMouseCoords(mouseCoords);

        vec3 worldMouseCoords;
        CameraViewToWorldPoint(mouseCoords, worldMouseCoords);
        //glm_vec2_copy(worldMouseCoords, ball.position);

        //LogVec3(worldMouseCoords);

        int computeHit = 0;

        if (ball.hitTime > 0 && ball.hitTime < currentSimTime)
        {
            glm_vec2_muladds(ball.velocity, ball.hitTime - ball.time, ball.position);

            if (interaction.id % 2 == 0) // horizontal lines, reflect vertical
            {
                glm_vec2_reflect(ball.velocity, (vec2) { 0, 1 }, ball.velocity);
            }
            else // vertical lines, reflect horizontal
            {
                glm_vec2_reflect(ball.velocity, (vec2) { 1, 0 }, ball.velocity);
            }

            ball.time = ball.hitTime;
            computeHit = 1;
            printf("Collided, updating...\n");
            printf("Ball hit time: %f\n", ball.hitTime);
            printf("Sim time: %f\n", currentSimTime);
        }

        if (ball.hitTime < 0 || computeHit)
        {
            float minTime = 999;

            vec2 times;

            for (int i = 0; i < 4; i++)
            {
                bool collided = CircleLineCollisionTime(
                    ball.position,
                    ball.velocity,
                    circle1->radius,
                    bounds[i].a,
                    bounds[i].b,
                    times);

                if (!collided) continue;

                float potentialTime = 999;

                if (times[1] < 0) continue; // both times are less than 0, no collision
                if (times[0] < 0)
                {
                    potentialTime = times[1]; // first time is < 0, use second time
                }
                else
                {
                    potentialTime = times[0]; // both times positive, use first time
                }
                if (potentialTime < minTime)
                {
                    minTime = potentialTime;
                    interaction.id = i;
                    interaction.time = minTime;
                }
            }

            minTime += ball.time - 0.00001f;

            glm_vec2_copy(ball.position, ghostCircle->position);
            glm_vec2_muladds(ball.velocity, minTime - ball.time, ghostCircle->position);
            ball.hitTime = minTime;
            printf("Computed min time: %f\n", minTime);
        }

        UpdateBall(&ball, currentSimTime);

        // if (collided)
        // {
        //     LogVec2(times);
        // }
        // else
        // {
        //     printf("No collision\n");
        // }

        PolygonRenderPolygons();
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