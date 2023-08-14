#define GLEW_STATIC
#include <GL\glew.h>
#include <stdio.h>
#include <malloc.h>
#include "shader.h"
#include "debug.h"

static long GetFileLength(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    if (file == NULL)
        return 0;

    fseek(file, 0, SEEK_END);
    return ftell(file);
}

// Loads the file at filePath into the buffer passed in
// Call GetFileLength first to know how large the buffer should be
static void ShaderLoad(const char* filePath, char* buffer)
{
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Error opening file at path: %s\n", filePath);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);

    fseek(file, 0, SEEK_SET);
    long actualLength = fread(buffer, 1, length, file);
    buffer[actualLength] = '\0';

    fclose(file);
}

int ShaderGetUniformId(unsigned int shaderId, const char* name)
{
    // uniform id or index
    GLCall(int uniformId = glGetUniformLocation(shaderId, name));
    return uniformId;
}

// Fetches the uniform block index (specific to a shader program)
int ShaderGetUniformBlockIndex(unsigned int shaderId, const char* name)
{
    GLCall(int blockIndex = glGetUniformBlockIndex(shaderId, name));
    return blockIndex;
}

// Binds a uniform binding point with a uniform index within the specified shader program
void ShaderUniformBlockBinding(unsigned int shaderId, unsigned int blockIndex,
    unsigned int bindingPoint)
{
    GLCall(glUniformBlockBinding(shaderId, blockIndex, bindingPoint));
}

// Binds a uniform buffer to a uniform block located by name in the specified shader program
void ShaderBindUniformBuffer(unsigned int shaderId, const char* name,
    UniformBuffer* uniformBuffer)
{
    GLCall(int blockIndex = glGetUniformBlockIndex(shaderId, name));
    GLCall(glUniformBlockBinding(shaderId, blockIndex, uniformBuffer->bindingPoint));
}


unsigned int ShaderCompile(unsigned int type, const char* filePath)
{
    long fileLength = GetFileLength(filePath);
    char* buffer = malloc(fileLength + 1);
    ShaderLoad(filePath, buffer);

    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, (const char* const*)&buffer, NULL);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    free(buffer);
    if (result != GL_FALSE)
        return id;

    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char* message = (char*)alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, NULL, message);
    printf("Failed to compile %s shader!\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
    printf(message);
    glDeleteShader(id);
    return 0;
}

// Creates a shader program with a vertex and fragment shader
// Deletes the passed in vertex and fragment shader
// Returns the id of the shader
unsigned int ShaderCreateFromIds(unsigned int vertexShaderId, unsigned int fragmentShaderId)
{
    unsigned int programId = glCreateProgram();

    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glValidateProgram(programId);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return programId;
}

unsigned int ShaderCreate(const char* vertexShaderFilePath, const char* fragmentShaderFilePath)
{
    unsigned int vertexShaderId = ShaderCompile(GL_VERTEX_SHADER, vertexShaderFilePath);
    unsigned int fragmentShaderId = ShaderCompile(GL_FRAGMENT_SHADER, fragmentShaderFilePath);

    return ShaderCreateFromIds(vertexShaderId, fragmentShaderId);
}

void ShaderUse(unsigned int shaderId)
{
    GLCall(glUseProgram(shaderId));
}