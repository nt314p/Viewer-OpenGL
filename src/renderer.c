#include <stdio.h>
#include <string.h>
#include "renderer.h"
#include "debug.h"

static unsigned int NextUniformBufferBindingPoint = 0;

static unsigned int GetNextUniformBufferBindingPoint()
{
    return NextUniformBufferBindingPoint++;
}

void VertexArrayInitialize(VertexArray* vertexArray)
{
    memset(vertexArray, 0, sizeof(VertexArray));
    GLCall(glGenVertexArrays(1, &vertexArray->id));
}

void VertexArrayBind(VertexArray* vertexArray)
{
    GLCall(glBindVertexArray(vertexArray->id));
}

void VertexArrayUnbind()
{
    GLCall(glBindVertexArray(0));
}

// Deletes the vertex array but does not alter the associated buffers
void VertexArrayDelete(VertexArray* vertexArray)
{
    GLCall(glDeleteVertexArrays(1, &vertexArray->id));
}

void VertexBufferInitialize(VertexArray* vertexArray, void* data, unsigned int size, GLenum usage)
{
    unsigned int* vertexBufferId = &vertexArray->vertexBufferId;
    GLCall(glGenBuffers(1, vertexBufferId));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, *vertexBufferId));
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, usage));

    // TODO: make an optional init method that marks whether the 
    // data and size are still valid beyond this call.
    // For use cases where the vertex buffer is initialized from
    // temp data.

    vertexArray->vertexBufferData = data;
    vertexArray->vertexBufferSize = size;
}

void VertexBufferUpdate(VertexArray* vertexArray)
{
    VertexBufferBind(vertexArray);
    GLCall(void* mappedBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    memcpy(mappedBuffer, vertexArray->vertexBufferData, vertexArray->vertexBufferSize);
    GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
}

void VertexBufferUpdateRange(VertexArray* vertexArray, unsigned int offset, unsigned int size)
{
    VertexBufferBind(vertexArray);
    GLCall(void* mappedBuffer = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
    memcpy(mappedBuffer, (char*)(vertexArray->vertexBufferData) + offset, size);
    GLCall(glUnmapBuffer(GL_ARRAY_BUFFER));
}

void VertexBufferBind(VertexArray* vertexArray)
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, vertexArray->vertexBufferId));
}

void VertexBufferUnbind()
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBufferDelete(VertexArray* vertexArray)
{
    GLCall(glDeleteBuffers(1, &vertexArray->vertexBufferId));
}

void IndexBufferInitialize(VertexArray* vertexArray, unsigned int* data, unsigned int count, GLenum usage)
{
    unsigned int* indexBufferId = &vertexArray->indexBufferId;
    GLCall(glGenBuffers(1, indexBufferId));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indexBufferId));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, usage));

    vertexArray->indexBufferData = data;
    vertexArray->indexBufferCount = count;
}

void IndexBufferBind(VertexArray* vertexArray)
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexArray->indexBufferId));
}

void IndexBufferUnbind()
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBufferDelete(VertexArray* vertexArray)
{
    GLCall(glDeleteBuffers(1, &vertexArray->indexBufferId));
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

void UniformBufferUpdateRange(UniformBuffer* uniformBuffer, unsigned int offset,
    unsigned int size)
{
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer->bufferId));
    GLCall(void* mappedBuffer = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY));
    memcpy(mappedBuffer, (char*)(uniformBuffer->data) + offset, size);
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