#pragma once

#include <cglm\cglm.h>

typedef struct Camera
{
    vec3 position;
    vec3 right;
    vec3 up;
    vec3 forward;
} Camera;

void CameraViewMatrix(vec3 position, vec3 forward, vec3 up, mat4 dest);
void CameraTranslate(Camera* camera, vec3 translation);
void CameraTranslateRelative(Camera* camera, vec3 translation);