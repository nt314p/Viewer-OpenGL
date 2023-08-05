#pragma once

#define GLEW_STATIC
#include <GL\glew.h>

int ShaderGetUniformId(unsigned int shaderId, const char* name);
int ShaderGetUniformBlockIndex(unsigned int shaderId, const char* name);
//void ShaderLoad(const char* filePath, char* buffer);
unsigned int ShaderCompile(unsigned int type, const char* filePath);
unsigned int ShaderCreateFromIds(unsigned int vertexShaderId, unsigned int fragmentShaderId);
unsigned int ShaderCreate(const char* vertexShaderFilePath, const char* fragmentShaderFilePath);