#pragma once

#define GLEW_STATIC
#include <GL\glew.h>

int GetShaderUniformId(unsigned int shaderId, const char* name);
//void LoadShader(const char* filePath, char* buffer);
unsigned int CompileShader(unsigned int type, const char* filePath);
unsigned int CreateShader(unsigned int vertexShaderId, unsigned int fragmentShaderId);