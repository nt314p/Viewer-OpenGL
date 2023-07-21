#include "camera.h"
#include <string.h>

static vec3 position = { 0.0f, 0.0f, 0.0f };
static vec3 right = { 1.0f, 0.0f, 0.0f };
static vec3 up = { 0.0f, 1.0f, 0.0f };
static vec3 forward = { 0.0f, 0.0f, 1.0f };

static float fov = 45.0f;
static float aspect = 16.0f / 9.0f;
static float near = 0.1f;
static float far = 100.0f;
static float orthoSize = 10.0f;
static mat4 projection;

// Initializes perspective projection parameters and sets
// the projection type to perspective
void CameraUsePerspective(float fovY, float aspectRatio, float nearClip, float farClip)
{
    fov = fovY;
    aspect = aspectRatio;
    near = nearClip;
    far = farClip;
    glm_perspective(fov, aspect, near, far, projection);
}

// Initializes orthographic projection parameters and sets
// the projection type to orthographic
void CameraUseOrthographic(float aspectRatio, float size)
{
    aspect = aspectRatio;
    orthoSize = size;
    glm_ortho_default_s(aspectRatio, size, projection);
}

// Returns P * V where P and V are the perspective and
// view matrices for the camera respectively
// I have no idea if this still works with ortho
// cameras but it seems like it does
void CameraViewPerspectiveMatrix(mat4 dest)
{
    CameraViewMatrix(dest);
    glm_mat4_mul(projection, dest, dest);    
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

// Computes the view matrix of the camera
void CameraViewMatrix(mat4 dest)
{
    dest[0][0] = right[0];
    dest[0][1] = up[0];
    dest[0][2] = forward[0];
    dest[1][0] = right[1];
    dest[1][1] = up[1];
    dest[1][2] = forward[1];
    dest[2][0] = right[2];
    dest[2][1] = up[2];
    dest[2][2] = forward[2];
    dest[3][0] = -glm_vec3_dot(right, position);
    dest[3][1] = -glm_vec3_dot(up, position);
    dest[3][2] = -glm_vec3_dot(forward, position);
    dest[0][3] = dest[1][3] = dest[2][3] = 0.0f;
    dest[3][3] = 1.0f;
}

// Translates the camera in worldspace
void CameraTranslate(vec3 translation)
{
    glm_vec3_add(translation, position, position);
}

// Translates the camera in viewspace/camera space
void CameraTranslateRelative(vec3 translation)
{
    glm_vec3_muladds(right, translation[0], position);
    glm_vec3_muladds(up, translation[1], position);
    glm_vec3_muladds(forward, translation[2], position);
}

// for orthographic camera
void CameraZoom(float size)
{
    glm_ortho_default_s(aspect, size, projection);
}

void CameraRotate(float yaw, float pitch, float roll)
{
    // pitch = a
    // yaw = b
    // roll = g
    float ca = cosf(glm_rad(pitch)); // rotation around x axis
    float sa = sinf(glm_rad(pitch));
    float cb = cosf(glm_rad(yaw)); // rotation around y axis
    float sb = sinf(glm_rad(yaw));
    float cg = cosf(glm_rad(roll)); // rotation around z axis
    float sg = sinf(glm_rad(roll));

    right[0] = cb * cg;
    right[1] = cb * sg;
    right[2] = -sb;

    up[0] = sa * sb * cg - ca * sg;
    up[1] = sa * sb * sg + ca * cg;
    up[2] = sa * cb;

    forward[0] = ca * sb * cg + sa * sg;
    forward[1] = ca * sb * sg - sa * cg;
    forward[2] = ca * cb;
}