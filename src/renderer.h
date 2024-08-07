#pragma once

#define GLEW_STATIC
#include <GL\glew.h>

typedef struct VertexArray
{
    unsigned int vertexBufferId;
    unsigned int vertexBufferSize;
    void* vertexBufferData;

    unsigned int indexBufferId;
    unsigned int indexBufferCount; // If zero, the index buffer has not been initialized
    unsigned int* indexBufferData;

    unsigned int id;
} VertexArray;

typedef struct UniformBuffer
{
    unsigned int bufferId;
    unsigned int size;
    unsigned int bindingPoint;
    void* data;
} UniformBuffer;

typedef struct StorageBuffer
{
    unsigned int bufferId;
    unsigned int size;
    unsigned int bindingPoint;
    void* data;
} StorageBuffer;

void VertexArrayInitialize(VertexArray* vertexArray);
void VertexArrayBind(VertexArray* vertexArray);
void VertexArrayUnbind();
void VertexArrayDelete(VertexArray* vertexArray);

void VertexBufferInitialize(VertexArray* vertexArray, void* data, unsigned int size, GLenum usage);
void VertexBufferUpdate(VertexArray* vertexArray);
void VertexBufferUpdateRange(VertexArray* vertexArray, unsigned int offset, unsigned int size);
void VertexBufferBind(VertexArray* vertexArray);
void VertexBufferUnbind();
void VertexBufferDelete(VertexArray* vertexArray);

void IndexBufferInitialize(VertexArray* vertexArray, unsigned int* data, unsigned int count, GLenum usage);
void IndexBufferBind(VertexArray* vertexArray);
void IndexBufferUnbind();
void IndexBufferDelete(VertexArray* vertexArray);

void UniformBufferInitialize(UniformBuffer* uniformBuffer, void* data, unsigned int size, GLenum usageHint);
void UniformBufferUpdate(UniformBuffer* uniformBuffer);
void UniformBufferUpdateRange(UniformBuffer* uniformBuffer, unsigned int offset, unsigned int size);
void UniformBufferBind(UniformBuffer* uniformBuffer);
void UniformBufferUnbind();
void UniformBufferDelete(UniformBuffer* uniformBuffer);

void VertexAttribPointerFloats(unsigned int index, int size);