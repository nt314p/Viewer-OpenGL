#pragma once

#include "renderer.h"

void ShaderUse(unsigned int shaderId);
int ShaderGetUniformId(unsigned int shaderId, const char* name);
int ShaderGetUniformBlockIndex(unsigned int shaderId, const char* name);
void ShaderBindUniformBuffer(unsigned int shaderId, const char* name,
    UniformBuffer* uniformBuffer);
unsigned int ShaderCompile(unsigned int type, const char* filePath);
unsigned int ShaderCreateFromIds(unsigned int vertexShaderId, unsigned int fragmentShaderId);
unsigned int ShaderCreate(const char* vertexShaderFilePath, const char* fragmentShaderFilePath);