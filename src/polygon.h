#pragma once

#include <cglm\cglm.h>

typedef struct Circle
{
    vec3 color;
    float padding;
    vec2 position;
    float radius;
    float padding2;
} Circle;

typedef struct Rect
{
    vec3 color;
    float padding;
    vec2 position;
    float width;
    float height;
} Rect;

typedef struct LineInternal
{
    vec2 a;
    vec2 b;
} LineInternal;

void PolygonInitialize();
void PolygonBindUnitCircle();
void PolygonBindUnitSquare();

Circle* PolygonCircle(vec2 position, float radius, vec3 color);
Rect* PolygonRect(vec2 position, float width, float height, vec3 color); 
void PolygonRenderPolygons();
void PolygonUpdateViewPerspectiveMatrix(mat4 vpMatrix);