#include "renderer.h"
#include <stdio.h>
#include <string.h>

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
    GLCall(glGenBuffers(1, &vertexBuffer->bufferId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->bufferId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
}

void VertexBufferBind(VertexBuffer* vertexBuffer)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->bufferId));
}

void VertexBufferUnbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBufferDelete(VertexBuffer* vertexBuffer)
{
    GLCall(glDeleteBuffers(1, &vertexBuffer->bufferId));
}

void IndexBufferInitialize(IndexBuffer* indexBuffer, unsigned int* data, unsigned int count)
{
    indexBuffer->count = count;
    GLCall(glGenBuffers(1, &indexBuffer->bufferId));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->bufferId));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW));
}

void IndexBufferBind(IndexBuffer* indexBuffer)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer->bufferId));
}

void IndexBufferUnbind()
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBufferDelete(IndexBuffer* indexBuffer)
{
    GLCall(glDeleteBuffers(1, &indexBuffer->bufferId));
}

void UniformBufferInitialize(UniformBuffer* uniformBuffer, void* data, unsigned int size, GLenum usageHint)
{
    uniformBuffer->data = data;
    uniformBuffer->size = size;
    GLCall(glGenBuffers(1, &uniformBuffer->bufferId));
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
    GLCall(glBufferData(GL_UNIFORM_BUFFER, size, data, usageHint));
}

void UniformBufferUpdate(UniformBuffer* uniformBuffer)
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
    GLCall(void* mappedBuffer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    memcpy(mappedBuffer, uniformBuffer->data, uniformBuffer->size);
    GLCall(glUnmapBuffer(GL_UNIFORM_BUFFER));
}

void UniformBufferBind(UniformBuffer* uniformBuffer)
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
}

void UniformBufferBindPoint(UniformBuffer* uniformBuffer, unsigned int index)
{
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, index, uniformBuffer->bufferId));
}

void UniformBufferUnbind()
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void UniformBufferDelete(UniformBuffer* uniformBuffer)
{
    GLCall(glDeleteBuffers(1, &uniformBuffer->bufferId));
}