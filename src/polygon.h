#pragma once

#include <cglm\cglm.h>
#include "renderer.h"
#include <string.h>

typedef struct Polygon_
{
    VertexBuffer vertexBuffer;
    mat4 transform;
    vec2* vertices; // this should be heap allocated only
    int vertexCount;
} Polygon_;

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

typedef struct Line
{
    vec3 color;
} Line;

void PolygonInitialize();
void PolygonBindUnitCircle();
void PolygonBindUnitSquare();
void PolygonCircle(Polygon_* polygon, float radius);
void PolygonDraw(Polygon_* polygon);
void PolygonFree(Polygon_* polygon);