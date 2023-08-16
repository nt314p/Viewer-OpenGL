#pragma once

#include <GLFW\glfw3.h>
#include <cglm\cglm.h>

void InputInitialize(GLFWwindow* window);
void InputMouseDelta(vec2 mouseDelta);
void InputMouseCoords(vec2 mouseCoords);
void InputScrollDelta(vec2 scrollDelta);
void InputGetAxes(GLFWwindow* window, vec2 axisInput);

int InputKeyPressed(GLFWwindow* window, int key);
