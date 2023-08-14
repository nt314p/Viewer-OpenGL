#pragma once

#define ASSERT(x) if(!(x)) __builtin_trap();
#define GLCall(x) GLClearErrors();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearErrors();
int GLLogCall(const char* function, const char* file, int line);