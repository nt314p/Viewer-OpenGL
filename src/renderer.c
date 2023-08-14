#include <stdio.h>
#include <string.h>
#include "renderer.h"
#include "debug.h"

static unsigned int NextUniformBufferBindingPoint = 0;

static unsigned int GetNextUniformBufferBindingPoint()
{
    return NextUniformBufferBindingPoint++;
}

void VertexArrayInitialize(unsigned int* vertexArrayId)
{
    GLCall(glGenVertexArrays(1, vertexArrayId));
}

void VertexArrayBind(unsigned int vertexArrayId)
{
    GLCall(glBindVertexArray(vertexArrayId));
}

void VertexArrayUnbind()
{
    GLCall(glBindVertexArray(0));
}

void VertexArrayDelete(unsigned int vertexArrayId)
{
    GLCall(glDeleteVertexArrays(1, &vertexArrayId));
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
    unsigned int bindingPoint = GetNextUniformBufferBindingPoint();
    uniformBuffer->bindingPoint = bindingPoint;
    GLCall(glGenBuffers(1, &uniformBuffer->bufferId));
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
    GLCall(glBufferData(GL_UNIFORM_BUFFER, size, data, usageHint));
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer->bufferId));
}

void UniformBufferUpdate(UniformBuffer* uniformBuffer)
{
    // TODO: investigate glMapBuffer vs glBufferSubData performance

    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
    GLCall(void* mappedBuffer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    memcpy(mappedBuffer, uniformBuffer->data, uniformBuffer->size);
    GLCall(glUnmapBuffer(GL_UNIFORM_BUFFER));
}

void UniformBufferBind(UniformBuffer* uniformBuffer)
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
}

void UniformBufferUnbind()
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void UniformBufferDelete(UniformBuffer* uniformBuffer)
{
    GLCall(glDeleteBuffers(1, &uniformBuffer->bufferId));
}

// Enables and configures an attribute at the specified index
// The attribute is a series of floats (number specified by size)
void VertexAttribPointerFloats(unsigned int index, int size)
{
    GLCall(glEnableVertexAttribArray(index));
    GLCall(glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, 0, 0));
}