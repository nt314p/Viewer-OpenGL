#include <GLFW\glfw3.h>
#include <cglm\cglm.h>
#include "input.h"
#include "debug.h"

static vec2 MouseCoords; // In normalized upright axis [-1, 1]
static vec2 MouseDelta; // In pixels per frame
static vec2 ScrollDelta;

static int WindowWidth;
static int WindowHeight;

static void ScrollCallback(GLFWwindow* window, double deltaX, double deltaY)
{
    ScrollDelta[0] = deltaX;
    ScrollDelta[1] = deltaY;
}

static void MouseCallback(GLFWwindow* window, double x, double y)
{
    static float prevX, prevY;
    static bool firstCall = true;

    float xScaled = x / (WindowWidth - 1);
    float yScaled = y /-(WindowHeight - 1); // Assumes height is smaller than width


    xScaled = 2 * xScaled - 1;
    yScaled = 2 * yScaled + 1;

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

    MouseCoords[0] = xScaled; // TODO: what are these units?
    MouseCoords[1] = yScaled;
    MouseDelta[0] = dx;
    MouseDelta[1] = dy;
}

void InputInitialize(GLFWwindow* window)
{
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    // TODO: this assumes that window size doesn't change after init
    glfwGetWindowSize(window, &WindowWidth, &WindowHeight);
}

void InputMouseDelta(vec2 mouseDelta)
{
    glm_vec2_copy(MouseDelta, mouseDelta);
}

void InputMouseCoords(vec2 mouseCoords)
{
    glm_vec2_copy(MouseCoords, mouseCoords);
}

void InputScrollDelta(vec2 scrollDelta)
{
    glm_vec2_copy(ScrollDelta, scrollDelta);
}

// Gets a 2d vector representing axis input
// x [-1, 1]: horizontal input
// y [-1, 1]: vertical input
void InputGetAxes(GLFWwindow* window, vec2 axisInput)
{
    // TODO: store window ref in this file?
    // TODO: custom horizontal and vertical axis keybinds
    axisInput[0] = InputKeyPressed(window, GLFW_KEY_D) - InputKeyPressed(window, GLFW_KEY_A);
    axisInput[1] = InputKeyPressed(window, GLFW_KEY_W) - InputKeyPressed(window, GLFW_KEY_S);
}

// Resets all input values. Should be called at the end of every frame
void InputReset()
{
    glm_vec2_zero(MouseDelta);
    glm_vec2_zero(ScrollDelta);
}

int InputKeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

int InputKeyDown(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_KEY_DOWN;
}