#define GLEW_STATIC
#include <GL\glew.h>
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