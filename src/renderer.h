#pragma once

#define GLEW_STATIC
#include <GL\glew.h>

#define ASSERT(x) if(!(x)) __builtin_trap();
#define GLCall(x) GLClearErrors();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearErrors();
int GLLogCall(const char* function, const char* file, int line);

// TODO: are the data and count fields for vertex and index buffers necessary

typedef struct VertexBuffer
{
    unsigned int bufferId;
    void* data;
} VertexBuffer;

typedef struct IndexBuffer
{
    unsigned int bufferId;
    unsigned int count; // how many indices are contained in the buffer
    void* data;
} IndexBuffer;

typedef struct UniformBuffer
{
    unsigned int bufferId;
    unsigned int size;
    unsigned int bindingPoint;
    void* data;
} UniformBuffer;

void VertexArrayInitialize(unsigned int* vertexArrayId);
void VertexArrayBind(unsigned int vertexArrayId);
void VertexArrayUnbind();
void VertexArrayDelete(unsigned int vertexArrayId);

void VertexBufferInitialize(VertexBuffer* vertexBuffer, void* data, unsigned int size);
void VertexBufferBind(VertexBuffer* vertexBuffer);
void VertexBufferUnbind();
void VertexBufferDelete(VertexBuffer* vertexBuffer);

void IndexBufferInitialize(IndexBuffer* indexBuffer, unsigned int* data, unsigned int count);
void IndexBufferBind(IndexBuffer* indexBuffer);
void IndexBufferUnbind();
void IndexBufferDelete(IndexBuffer* indexBuffer);

void UniformBufferInitialize(UniformBuffer* uniformBuffer, void* data, unsigned int size, GLenum usageHint);
void UniformBufferUpdate(UniformBuffer* uniformBuffer);
void UniformBufferBind(UniformBuffer* uniformBuffer);
void UniformBufferBindPoint(UniformBuffer* uniformBuffer, unsigned int bindingPoint);
void UniformBufferUnbind();
void UniformBufferDelete(UniformBuffer* uniformBuffer);

void VertexAttribPointerFloats(unsigned int index, int size);