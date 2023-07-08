#include "model_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

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