#pragma once

#include <cglm\cglm.h>
#include "renderer.h"
#include <string.h>

typedef struct Polygon {
    VertexBuffer vertexBuffer;
    mat4 transform;
    vec3* vertices; // this should be heap allocated only
    int vertexCount;
} Polygon;

void PolygonInitialize();
void PolygonBindUnitCircle();
void PolygonCircle(Polygon* polygon, float radius);
void PolygonDraw(Polygon* polygon);
void PolygonFree(Polygon* polygon);