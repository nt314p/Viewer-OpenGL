#include "model_parser.h"
#include "renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <cglm\cglm.h>

// Returns the length of the file
int ReadModel(const char* filePath, char* buffer)
{
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Error opening file at path: %s\n", filePath);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);

    fseek(file, 0, SEEK_SET);
    long actualLength = fread(buffer, 1, length, file);
    buffer[actualLength] = '\0';
    fclose(file);

    return actualLength;
}

// VAO must be bound before call
int ModelToBuffers(const char* filePath, VertexBuffer* vb, IndexBuffer* ib) {
    char* buffer = malloc(8 * 1024); // allocate 8K

    int modelLength = ReadModel(filePath, buffer);
    if (modelLength == -1) return -1;

    int modelCounts[2];
    GetModelBufferCounts(buffer, modelLength, modelCounts);
    printf("V: %d; F: %d\n", modelCounts[0], modelCounts[1]);

    unsigned int vertexCount = modelCounts[0];
    unsigned int faceCount = modelCounts[1];
    float* vertices = malloc(vertexCount * sizeof(vec3));
    unsigned int* faces = malloc(faceCount * 3 * sizeof(unsigned int));

    ParseModel(buffer, modelLength, vertices, vertexCount, faces, faceCount);
    free(buffer);

    VertexBufferInitialize(vb, vertices, vertexCount * sizeof(vec3));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
    GLCall(glEnableVertexAttribArray(0));

    IndexBufferInitialize(ib, faces, faceCount * 3);
    
    return 0;
}

// counts array
// Index 0: vertex counts
// Index 1: face counts
void GetModelBufferCounts(char* data, int length, int counts[2])
{
    int vertexCount = 0;
    int faceCount = 0;
    for (int i = 0; i < length; i++)
    {
        char curr = data[i];
        if (curr == '#') {
            while (data[i] != '\n') { i++; }
            continue;
        }
        if (curr == 'v') vertexCount++;
        if (curr == 'f') faceCount++;
    }

    counts[0] = vertexCount;
    counts[1] = faceCount;
}

// Returns a pointer to the end the parsed values
char* ParseVertex(char* start, float vertex[3])
{
    char* end;
    vertex[0] = (float)strtod(start, &end);
    start = end;
    vertex[1] = (float)strtod(start, &end);
    start = end;
    vertex[2] = (float)strtod(start, &end);
    return end;
}

// Returns a pointer to the end the parsed values
char* ParseFace(char* start, unsigned int face[3])
{
    char* end;
    face[0] = (int)strtol(start, &end, 10) - 1;
    start = end;
    face[1] = (int)strtol(start, &end, 10) - 1;
    start = end;
    face[2] = (int)strtol(start, &end, 10) - 1;
    return end;
}

void ParseModel(char* data, int length, float* vertices, unsigned int vertexCount,
    unsigned int* faces, unsigned int faceCount)
{
    int vertexIndex = 0;
    int faceIndex = 0;

    char* end = data + length;
    while (data < end - 1)
    {
        char curr = *data;
        char next = *(data + 1);

        if (curr == 'v' && next == ' ')
        {
            data++;
            data = ParseVertex(data, vertices + vertexIndex);
            vertexIndex += 3;
        }

        if (curr == 'f' && next == ' ')
        {
            data++;
            data = ParseFace(data, faces + faceIndex);
            faceIndex += 3;
        }

        data++;
    }
}