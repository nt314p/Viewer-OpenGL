#include "shader.h"
#include <stdio.h>
#include <malloc.h>

static long GetFileLength(const char* filePath)
{
    FILE* file = fopen(filePath, "r");
    if (file == NULL) return 0;

    fseek(file, 0, SEEK_END);
    return ftell(file);
}

// Loads the file at filePath into the buffer passed in
// Call GetFileLength first to know how large the buffer should be
static void LoadShader(const char* filePath, char* buffer)
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

unsigned int CompileShader(unsigned int type, const char* filePath)
{
    long fileLength = GetFileLength(filePath);
    char* buffer = malloc(fileLength + 1);
    LoadShader(filePath, buffer);

    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, (const char* const*) &buffer, NULL);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    free(buffer);
    if (result != GL_FALSE) return id;

    int length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    char* message = (char*)alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, NULL, message);
    printf("Failed to compile %s shader!\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
    printf(message);
    glDeleteShader(id);
    return 0;
}

unsigned int CreateShader(unsigned int vertexShaderId, unsigned int fragmentShaderId)
{
    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShaderId);
    glAttachShader(program, fragmentShaderId);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vertexShaderId);
    glDeleteShader(fragmentShaderId);

    return program;
}