#include "renderer.h"
#include <stdio.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>

void GLClearErrors()
{
    while (glGetError() != GL_NO_ERROR);
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

void VertexBufferInitialize(VertexBuffer* vertexBuffer, void* data, unsigned int size)
{
    GLCall(glGenBuffers(1, &vertexBuffer->rendererId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->rendererId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

void VertexBufferBind(VertexBuffer* vertexBuffer)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->rendererId));
}

void VertexBufferUnbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBufferDelete(VertexBuffer* vertexBuffer)
{
    GLCall(glDeleteBuffers(1, &vertexBuffer->rendererId));
}

void IndexBufferInitialize(IndexBuffer* indexBuffer, unsigned int* data, unsigned int count)
{
    indexBuffer->count = count;
    GLCall(glGenBuffers(1, &indexBuffer->rendererId));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->rendererId));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

void IndexBufferBind(IndexBuffer* indexBuffer)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->rendererId));
}

void IndexBufferUnbind()
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBufferDelete(IndexBuffer* indexBuffer)
{
    GLCall(glDeleteBuffers(1, &indexBuffer->rendererId));
}