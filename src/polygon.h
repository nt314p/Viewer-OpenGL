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

typedef struct Line
{
    vec3 color;
    float length;
    vec2 a;
    vec2 b;
} Line;

// Initializes polygons; must be called before other functions
void PolygonInitialize();

// Initializes a circle and returns its pointer
Circle* PolygonCircle(vec2 position, float radius, vec3 color);

// Allocates `count` contiguous circles and returns a pointer
Circle* PolygonCircles(unsigned int count);

// Initializes a rectangle and returns its pointer
Rect* PolygonRect(vec2 position, float width, float height, vec3 color);

// Allocates `count` contiguous rectangles and returns a pointer
Rect* PolygonRects(unsigned int count);

// Initializes a line and returns its pointer
Line* PolygonLine(vec2 a, vec2 b, vec3 color);

// Allocates `count` contiguous lines and returns a pointer
Line* PolygonLines(unsigned int count);

void PolygonUpdateViewPerspectiveMatrix(mat4 vpMatrix);
void PolygonRenderPolygons();