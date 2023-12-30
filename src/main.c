#include <cglm\cglm.h>
#include <cglm\version.h>
#define GLEW_STATIC
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdint.h>
#include <float.h>
#include "debug.h"
#include "timer.h"
#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "polygon.h"
#include "input.h"
#include "color.h"
#include "physics.h"

#define NUM_BALLS 3

static const int WIDTH = 1280;
static const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window);

static double deltaTime;
static double lastFrameTime;

static float zoom = 12.0f;

static int paused = 0;

struct Ball
{
    Circle* circle; // underlying circle render object
    Circle* ghost; // ghost circle render object
    Line* line; // velocity indicator
    vec2 position;
    vec2 velocity;
    float time; // the ball was at position `position` at time `time`
} typedef Ball;

// TODO: hit param is only needed for ghost effect,
// can remove later.
static void UpdateBall(Ball* ball, float time, float hitTime)
{
    float deltaTime = time - ball->time;
    glm_vec2_copy(ball->position, ball->circle->position);
    glm_vec2_muladds(ball->velocity, deltaTime, ball->circle->position);

    if (ball->line != NULL)
    {
        glm_vec2_copy(ball->circle->position, ball->line->a);
        glm_vec2_copy(ball->velocity, ball->line->b);
        float length = glm_vec2_norm(ball->velocity);
        ball->line->length = length;
        ball->line->b[0] /= length;
        ball->line->b[1] /= length;
    }

    if (ball->ghost != NULL)
    {
        float deltaTime = hitTime - ball->time;
        glm_vec2_copy(ball->position, ball->ghost->position);
        glm_vec2_muladds(ball->velocity, deltaTime, ball->ghost->position);
    }
}

/*
Bug:
Circle A and circle B

Circle B is on course to hit a wall
Circle A bounces off a wall and now will collide with circle B
Circle B still thinks it will hit the wall, but circle A bounces

... bug behavior ensues

*/

// Returns the index of the object that the ball at `ballIndex` will now collide with
static int UpdateBallCollisions(int ballIndex, Ball* balls, Interaction* interactions, Line* bounds)
{
    float minTime = FLT_MAX;
    vec2 times;

    // Line collisions
    for (int boundIndex = 0; boundIndex < 4; boundIndex++)
    {
        bool collided = CircleLineCollisionTime(
            balls[ballIndex].position,
            balls[ballIndex].velocity,
            balls[ballIndex].circle->radius,
            bounds[boundIndex].a,
            bounds[boundIndex].b,
            times);

        if (!collided) continue;
        if (times[0] < 0) continue; // discard collision in past

        // current interaction is more in the future than some
        // other hit time. also discard
        if (minTime <= times[0]) continue;

        // new closest hit time found
        minTime = times[0];
        interactions[ballIndex].id = boundIndex;
    }

    // Circle collisions
    for (int otherBallIndex = 0; otherBallIndex < NUM_BALLS; otherBallIndex++)
    {
        if (otherBallIndex == ballIndex) continue;

        vec2 otherBallPos;

        // Move other ball to the position it would have been in
        // at the current ball's local time
        glm_vec2_copy(balls[otherBallIndex].position, otherBallPos);
        float diff = balls[ballIndex].time - balls[otherBallIndex].time;
        glm_vec2_muladds(balls[otherBallIndex].velocity, diff, otherBallPos);

        bool collided = CircleCircleCollisionTime(
            balls[ballIndex].position,
            balls[ballIndex].velocity,
            balls[ballIndex].circle->radius,
            otherBallPos,
            balls[otherBallIndex].velocity,
            balls[otherBallIndex].circle->radius,
            times);

        if (!collided) continue;
        if (times[0] < 0) continue; // discard collision in past

        // current interaction is more in the future than some
        // other hit time. also discard
        if (minTime <= times[0]) continue;

        // new closest hit time found
        minTime = times[0];
        interactions[ballIndex].id = otherBallIndex + 4;
    }

    // convert local time (relative to ball time) to
    // global time (relative to start of sim)
    minTime += balls[ballIndex].time;
    interactions[ballIndex].time = minTime;

    return interactions[ballIndex].id;

    printf("%d -> %d at %f\n", ballIndex, interactions[ballIndex].id - 4, minTime);
}

// TODO: is this uniformly distributed?
static inline float RandomRange(float min, float max)
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

double currentSimTime;

int main(void)
{
    GLFWwindow* window = InitializeWindow(WIDTH, HEIGHT, false);
    if (window == NULL) return -1;

    if (glewInit() != GLEW_OK)
    {
        printf("Error initializing glew!\n");
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("cglm version: %d.%d.%d\n",
        CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH);

    InputInitialize(window);
    PolygonInitialize();

    //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    // CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);
    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    Line* bounds = PolygonLines(4);

    for (int i = 0; i < 4; i++)
    {
        glm_vec3_copy((vec3)CLR_ORANGE, (bounds + i)->color);
        (bounds + i)->length = i % 2 == 0 ? 40 : 20;
    }

    glm_vec2_copy((vec2) { -20, 10 }, bounds[0].a);
    glm_vec2_copy((vec2) { 1, 0 }, bounds[0].b);

    glm_vec2_copy((vec2) { 20, -10 }, bounds[1].a);
    glm_vec2_copy((vec2) { 0, 1 }, bounds[1].b);

    glm_vec2_copy((vec2) { -20, -10 }, bounds[2].a);
    glm_vec2_copy((vec2) { 1, 0 }, bounds[2].b);

    glm_vec2_copy((vec2) { -20, -10 }, bounds[3].a);
    glm_vec2_copy((vec2) { 0, 1 }, bounds[3].b);

    Interaction interactions[NUM_BALLS] = { 0 };
    Ball balls[NUM_BALLS] = { 0 };

    for (int i = 0; i < NUM_BALLS; i++)
    {
        interactions[i].time = -1;

        balls[i].circle = PolygonCircle((vec2) { 0, 0 }, 2, (vec3)CLR_MEDIUMSEAGREEN);
        balls[i].ghost = PolygonCircle((vec2) { 0, 0 }, 2, (vec3)CLR_PALEGREEN);
        balls[i].line = PolygonLine((vec2) { 0, 0 }, (vec2) { 0, 1 }, (vec3)CLR_WHITE);
        balls[i].time = 0.0f;
        glm_vec2_copy((vec2) { RandomRange(-2, 2), RandomRange(-2, 2) }, balls[i].position);
        glm_vec2_copy((vec2) { RandomRange(-10, 10), RandomRange(-10, 10) }, balls[i].velocity);
    }

    double startTime = glfwGetTime();
    currentSimTime = startTime;
    double lastFPSUpdate = startTime;

    for (int ballIndex = 0; ballIndex < NUM_BALLS; ballIndex++)
    {
        UpdateBallCollisions(ballIndex, balls, interactions, bounds);
    }


    while (!glfwWindowShouldClose(window))
    {
        double currentFrameTime = glfwGetTime();
        currentSimTime = currentFrameTime - startTime;
        deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        if (lastFPSUpdate + 0.5 < currentFrameTime)
        {
            char fpsText[30];
            snprintf(fpsText, 30, "Viewer | Render: %3.2f ms", deltaTime * 1000.0);

            glfwSetWindowTitle(window, fpsText);
            lastFPSUpdate += 0.5;
        }

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

        // Pass 1: check if balls have reached their collision time ->
        // then move them to their exact collision point
        for (int ballIndex = 0; ballIndex < NUM_BALLS; ballIndex++)
        {
            // Collision time is still in the future, skip
            if (interactions[ballIndex].time > currentSimTime) continue;

            glm_vec2_muladds(balls[ballIndex].velocity,
                interactions[ballIndex].time - balls[ballIndex].time,
                balls[ballIndex].position); // move ball to new position

            //printf("Ball hit: %.4f s | Sim time: %.4f s | Diff %f s\n",
            //    interactions[i].time, currentSimTime, currentSimTime - interactions[i].time);
        }

        // Pass 2: For balls that have collided, compute new velocities and update
        // Also compute time of new collision and update state
        for (int ballIndex = 0; ballIndex < NUM_BALLS; ballIndex++)
        {
            // Collision time is still in the future, skip
            if (interactions[ballIndex].time > currentSimTime) continue;

            vec2 normal;
            int otherBallIndex = -1;

            if (interactions[ballIndex].id < 4)
            {
                if (interactions[ballIndex].id % 2 == 0) // horizontal lines, reflect vertical
                {
                    normal[0] = 0;
                    normal[1] = 1;
                }
                else // vertical lines, reflect horizontal
                {
                    normal[0] = 1;
                    normal[1] = 0;
                }
            }
            else
            {
                otherBallIndex = interactions[ballIndex].id - 4;

                // LogVec2(balls[ballIndex].position);
                // LogVec2(balls[otherBallIndex].position);

                glm_vec2_sub(balls[ballIndex].position, balls[otherBallIndex].position,
                    normal); // compute line segment between two circles

                glm_vec2_scale(normal,
                    1.0f / (balls[ballIndex].circle->radius + balls[otherBallIndex].circle->radius),
                    normal); // normalize normal

                float length = glm_vec2_norm(normal);
                if (fabs(length - 1.0f) > 0.001f) printf("Normal length error! ");
                printf("Normal length: %f\n", length);
                // Normal length error means that the balls aren't actually next to
                // each other when the collision processing is triggered.
                // This indicates an error with the collision algorithm.
                //paused = 1;
            }

            glm_vec2_reflect(balls[ballIndex].velocity, normal, balls[ballIndex].velocity);
        }

        // Pass 3: Update ball collision time and next collision state
        for (int ballIndex = 0; ballIndex < NUM_BALLS; ballIndex++)
        {
            float interactionTime = interactions[ballIndex].time;
            if (interactionTime > currentSimTime) continue;
            balls[ballIndex].time = interactionTime;

            int collidedIndex = UpdateBallCollisions(ballIndex, balls, interactions, bounds);
            //printf("Ball %d to collide at %f\n", ballIndex, interactionTime);
            if (collidedIndex < 4) continue;

            // is a ball
            int collidedBallIndex = collidedIndex - 4;
            float newInteractionTime = interactions[ballIndex].time;
            
            // check the other ball if it has a later collision time
            // if so, we will collide with this ball before it hits its
            // other target. update the time state accordingly
            if (interactions[collidedBallIndex].time > newInteractionTime)
            {
                //printf("Updated ball\n");
                //printf("Prev: %f; ", interactions[collidedBallIndex].time);
                interactions[collidedBallIndex].time = newInteractionTime;
                interactions[collidedBallIndex].id = ballIndex + 4;
                //printf("Updated to: %f\n", newInteractionTime);
            }
        }

        // Pass 4: Update all balls
        for (int i = 0; i < NUM_BALLS; i++)
        {
            if (!paused)
            {
                UpdateBall(balls + i, currentSimTime, interactions[i].time);
            }
        }

        PolygonRenderPolygons();
        InputReset();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // TODO: Delete things??

    glfwTerminate();
    return 0;
}

static void ProcessInput(GLFWwindow* window)
{
    float speed = zoom / 2.0f;
    vec3 movement = { 0.0f };

    InputGetAxes(window, movement); // TODO: passing vec3 as a vec2?

    if (InputKeyPressed(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (InputKeyPressed(window, GLFW_KEY_SPACE))
    {
        currentSimTime += deltaTime * 0.5f;
        printf("time: %f\n", currentSimTime);
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}