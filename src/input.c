#include <GLFW\glfw3.h>
#include <cglm\cglm.h>
#include "input.h"

static vec2 MouseCoords;
static vec2 MouseDelta;
static vec2 ScrollDelta;

static void ScrollCallback(GLFWwindow* window, double deltaX, double deltaY)
{
    ScrollDelta[0] = deltaX;
    ScrollDelta[1] = deltaY;
}

static void MouseCallback(GLFWwindow* window, double x, double y)
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

    MouseCoords[0] = x;
    MouseCoords[1] = y;
    MouseDelta[0] = dx;
    MouseDelta[1] = dy;
}

void InputInitialize(GLFWwindow* window)
{
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
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