#pragma once

int ReadModel(const char* filePath, char* buffer);
void GetModelBufferCounts(char* data, int length, int counts[2]);
void ParseModel(char* data, int length, float* vertices, unsigned int vertexCount, 
    unsigned int* faces, unsigned int faceCount);