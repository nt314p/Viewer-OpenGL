#pragma once

#include <cglm\cglm.h>
#include "renderer.h"
#include <string.h>

typedef struct Polygon
{
    VertexBuffer vertexBuffer;
    mat4 transform;
    vec2 *vertices; // this should be heap allocated only
    int vertexCount;
} Polygon;

typedef struct Circle
{
    vec3 color;
    float padding;
    vec2 position;
    float radius;
    float padding2;
} Circle;

typedef struct Rectangle
{
    vec3 color;
    float padding;
    vec2 position;
    float width;
    float height;
} Rectangle;

void PolygonInitialize();
void PolygonBindUnitCircle();
void PolygonBindUnitSquare();
void PolygonCircle(Polygon *polygon, float radius);
void PolygonDraw(Polygon *polygon);
void PolygonFree(Polygon *polygon);