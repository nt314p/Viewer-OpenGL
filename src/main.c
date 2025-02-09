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
#include "priority_queue.h"

#define NUM_BALLS 3

static const int WIDTH = 1280;
static const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window, float deltaTime);

static float zoom = 12.0f;
static float timeFactor = 1.0f;

static int paused = 0;

struct Ball
{
    Circle* circle; // underlying circle render object
    Circle* ghost; // ghost circle render object
    Line2D* line; // velocity indicator
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
        glm_vec2_add(ball->position, ball->velocity, ball->line->b);
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

// Returns the index of the object that the ball at `ballIndex` will 
// now collide with the soonest
static int UpdateBallCollisions(int ballIndex, Ball* balls, Interaction* interactions, Line2D* bounds)
{
    float minTime = FLT_MAX;
    vec2 times;

    // We divide the collision checking into two stages:
    // 1. Line check (static)
    // 2. Ball check (dynamic)

    // Line collisions
    for (int boundIndex = 0; boundIndex < 4; boundIndex++)
    {
        vec2 dir;
        glm_vec2_sub(bounds[boundIndex].b, bounds[boundIndex].a, dir);
        glm_vec2_normalize(dir);

        bool collided = CircleLineCollisionTime(
            balls[ballIndex].position,
            balls[ballIndex].velocity,
            balls[ballIndex].circle->radius,
            bounds[boundIndex].a,
            dir,
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
        float diffTime = balls[ballIndex].time - balls[otherBallIndex].time;
        glm_vec2_muladds(balls[otherBallIndex].velocity, diffTime, otherBallPos);

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

        // the other ball will collide with something else sooner
        // reject this time
        if (times[0] >= interactions[otherBallIndex].time)
        {
            printf("Rejected collision time because other ball would collide sooner\n");
            continue;
        }

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

GLFWwindow* SimInitWindow(int width, int height, const char* title, int isFullscreen);

GLFWwindow* window;

Interaction interactions[NUM_BALLS] = { 0 };
Ball balls[NUM_BALLS] = { 0 };
Line2D* bounds;
PriorityQueue pq;

double startTime;
double lastFPSUpdate;
double currentSimTime;

GLFWwindow* Initialize()
{
    window = SimInitWindow(WIDTH, HEIGHT, "Viewer", false);
    if (window == NULL)
    {
        printf("Error!\n"); // TODO: handle this better
    }

    printf("OpenGL version: %s\n", glGetString(GL_VERSION));
    printf("cglm version: %d.%d.%d\n",
        CGLM_VERSION_MAJOR, CGLM_VERSION_MINOR, CGLM_VERSION_PATCH);

    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    vec2 tl = { -20, 10 };
    vec2 tr = { 20, 10 };
    vec2 bl = { -20, -10};
    vec2 br = { 20, -10};
    bounds = PolygonLine2D(tl, tr, CLR_ORANGE);
    PolygonLine2D(br, bl, CLR_ORANGE);
    PolygonLine2D(tl, bl, CLR_ORANGE);
    PolygonLine2D(tr, br, CLR_ORANGE);
    // Slightly cursed code assumes that subsequent calls return contiguous elements

    for (int i = 0; i < NUM_BALLS; i++)
    {
        interactions[i].time = -1;

        balls[i].circle = PolygonCircle((vec2) { 0, 0 }, 2, CLR_MEDIUMSEAGREEN);
        balls[i].ghost = PolygonCircle((vec2) { 0, 0 }, 2, CLR_PALEGREEN);
        balls[i].line = PolygonLine2D((vec2) { 0, 0 }, (vec2) { 0, 1 }, CLR_WHITE);
        balls[i].time = 0.0f;
        glm_vec2_copy((vec2) { RandomRange(-2, 2), RandomRange(-2, 2) }, balls[i].position);
        glm_vec2_copy((vec2) { RandomRange(-10, 10), RandomRange(-10, 10) }, balls[i].velocity);
    }

    vec2 positions[] = { { 1.532090, 1.910642 }, {0.407544, -1.051607}, {0.508621, -0.398999} };
    vec2 velocities[] = { { -5.274514, -1.284524 }, {-7.015290, 6.093630}, {-9.227271, 4.256416} };

    for (int i = 0; i < NUM_BALLS; i++)
    {
        //LogVec2(balls[i].position);
        //LogVec2(balls[i].velocity);
        glm_vec2_copy(positions[i], balls[i].position);
        glm_vec2_copy(velocities[i], balls[i].velocity);
    }

    for (int ballIndex = 0; ballIndex < NUM_BALLS; ballIndex++)
    {
        UpdateBallCollisions(ballIndex, balls, interactions, bounds);
    }

    Interaction inact[10];
    PriorityQueueCreate(&pq, inact, 10);

    for (int i = 0; i < NUM_BALLS; i++)
    {
        PriorityQueuePush(&pq, interactions[i]);
    }

    startTime = glfwGetTime();
    lastFPSUpdate = startTime;

    //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    return window;
}

void Update(float deltaTime)
{
    double currentFrameTime = glfwGetTime();
    currentSimTime = currentFrameTime - startTime;

    if (lastFPSUpdate + 0.5 < currentFrameTime)
    {
        char fpsText[30];
        snprintf(fpsText, 30, "Viewer | Render: %3.2f ms", deltaTime * 1000.0);

        glfwSetWindowTitle(window, fpsText);
        lastFPSUpdate += 0.5;
    }

    ProcessInput(window, deltaTime);

    vec2 scrollDelta;
    InputScrollDelta(scrollDelta);

    zoom += scrollDelta[1];
    zoom = fmax(zoom, 0.1f);
    CameraZoom(zoom);

    vec2 mouseCoords;
    InputMouseCoords(mouseCoords);

    vec3 worldMouseCoords;
    CameraViewToWorldPoint(mouseCoords, worldMouseCoords);
}

static void ProcessInput(GLFWwindow* window, float deltaTime)
{
    float speed = zoom / 2.0f;
    vec3 movement = { 0.0f };

    InputGetAxes(window, movement); // TODO: passing vec3 as a vec2?

    if (InputKeyPressed(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (InputKeyPressed(window, GLFW_KEY_COMMA))
    {
        currentSimTime -= deltaTime * 0.4f;
    }

    if (InputKeyPressed(window, GLFW_KEY_PERIOD))
    {
        currentSimTime += deltaTime * 3;
    }

    if (InputKeyPressed(window, GLFW_KEY_SPACE))
    {
        paused = !paused;
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}