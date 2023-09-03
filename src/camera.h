#pragma once

#include <cglm\cglm.h>

void CameraViewPerspectiveMatrix(mat4 dest);
void CameraUseOrthographic(float aspectRatio, float size);
void CameraUsePerspective(float fovY, float aspectRatio, float nearClip, float farClip);
void CameraViewMatrix(mat4 dest);
void CameraTranslate(vec3 translation);
void CameraTranslateRelative(vec3 translation);
void CameraZoom(float size);
void CameraRotate(float yaw, float pitch, float roll);
void CameraViewToWorldPoint(vec2 viewPoint, vec3 worldPoint);