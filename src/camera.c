#include "camera.h"
#include <string.h>

Camera DEFAULT_CAMERA = {
    { 0.0f, 0.0f, 0.0f }, // position
    { 1.0f, 0.0f, 0.0f }, // right
    { 0.0f, 1.0f, 0.0f }, // up
    { 0.0f, 0.0f, 1.0f }, // forward
};

// Initializes camera. Definitely not cursed
void CameraInitialize(Camera* camera)
{
    memcpy(camera, &DEFAULT_CAMERA, sizeof(DEFAULT_CAMERA));
}

/*
Camera matrix =
X Y Z | P
------+--
0 0 0 | 1

Let X, Y, Z, and P be column vectors.
X, Y, and Z are the worldspace camera axes
P is the camera position

This matrix represents the rotation and translation
of the camera in worldspace.

View matrix =
X | -P*X
Y | -P*Y
Z | -P*Z
--+-----
0 |  1

Now, X, Y, and Z are row vectors.
The view matrix is the inverse of the camera matrix,
and represents the transformation of coordinates in
worldspace to the view space.

Note that the upper left blocks of the camera and
view matrices are transposes of each other. They
are also inverses of each other. This property
holds because {X, Y, Z} is an orthonormal basis.

P represents the translation of the camera in world
space. We want to find the camera's translation in
view space. So we can compute the dot products P*X,
P*Y, and P*Z.
*/

// Computes the view matrix given a camera position and
// the normalized forward and up directions of the camera
void CameraViewMatrix(Camera* camera, mat4 dest)
{
    dest[0][0] = camera->right[0];
    dest[0][1] = camera->up[0];
    dest[0][2] = camera->forward[0];
    dest[1][0] = camera->right[1];
    dest[1][1] = camera->up[1];
    dest[1][2] = camera->forward[1];
    dest[2][0] = camera->right[2];
    dest[2][1] = camera->up[2];
    dest[2][2] = camera->forward[2];
    dest[3][0] = -glm_vec3_dot(camera->right, camera->position);
    dest[3][1] = -glm_vec3_dot(camera->up, camera->position);
    dest[3][2] = -glm_vec3_dot(camera->forward, camera->position);
    dest[0][3] = dest[1][3] = dest[2][3] = 0.0f;
    dest[3][3] = 1.0f;
}

// Translates the camera in worldspace
void CameraTranslate(Camera* camera, vec3 translation)
{
    glm_vec3_add(translation, camera->position, camera->position);
}

// Translates the camera in viewspace/camera space
void CameraTranslateRelative(Camera* camera, vec3 translation)
{
    glm_vec3_muladds(camera->right, translation[0], camera->position);
    glm_vec3_muladds(camera->up, translation[1], camera->position);
    glm_vec3_muladds(camera->forward, translation[2], camera->position);
}