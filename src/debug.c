#define GLEW_STATIC
#include <GL\glew.h>
#include <cglm\cglm.h>
#include <stdio.h>
#include "debug.h"

void GLClearErrors()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}

int GLLogCall(const char* function, const char* file, int line)
{
    GLenum error = glGetError();
    while (error != GL_NO_ERROR)
    {
        printf("OpenGL error (0x%x): %s in %s:%d\n", error, function, file, line);
        error = glGetError();
        return 0;
    }

    return 1;
}

void LogVec2(vec2 v)
{
    printf("(%f, %f)\n", v[0], v[1]);
}

void LogVec3(vec3 v)
{
    printf("(%f, %f, %f)\n", v[0], v[1], v[2]);
}