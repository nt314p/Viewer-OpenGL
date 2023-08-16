#include <string.h>
#include <cglm\cglm.h>
#include "polygon.h"
#include "shader.h"
#include "renderer.h"
#include "debug.h"

#define CircleVertexCount 50

const char* BasicFragShaderPath = "shaders/BasicFrag.frag";
const char* RectVertShaderPath = "shaders/InstancedRect.vert";
const char* CircleVertShaderPath = "shaders/InstancedCircle.vert";
const char* LineVertShaderPath = "shaders/InstancedLine.vert";

static vec2 UnitCircleVertices[CircleVertexCount + 2];
static VertexBuffer UnitCircleVertexBuffer;
static unsigned int UnitCircleVertexArrayId;

static vec2 UnitSquareVertices[4];
static VertexBuffer UnitSquareVertexBuffer;
static unsigned int UnitSquareVertexArrayId;

static bool IsInitialized = false;

static const int MaxCircleCount = 2048;
static const int MaxRectCount = 2048;
// static const int MaxLineCount = 4096;

static Circle* circles;
static Rect* rects;

static UniformBuffer circlesBuffer;
static UniformBuffer rectsBuffer;

static unsigned int numCircles;
static unsigned int numRects;

static mat4 vpMatrix;
static UniformBuffer vpMatrixUB;

static unsigned int circleShaderId;
static unsigned int lineShaderId;
static unsigned int rectShaderId;

// TODO: Specialized triangulation for circles?
// https://www.humus.name/index.php?page=News&ID=228

void InitializeUnitCircle()
{
    // Add two vertices. One for the center and the other for the final vertex
    // that must overlap with the first vertex on the circle

    for (int i = 0; i <= CircleVertexCount; i++)
    {
        int index = i + 1;
        float angle = (2 * M_PI / CircleVertexCount) * i;

        UnitCircleVertices[index][0] = cosf(angle);
        UnitCircleVertices[index][1] = sinf(angle);
    }

    VertexArrayInitialize(&UnitCircleVertexArrayId);
    VertexArrayBind(UnitCircleVertexArrayId);

    VertexBufferInitialize(&UnitCircleVertexBuffer, UnitCircleVertices, sizeof(UnitCircleVertices));
    VertexAttribPointerFloats(0, 2);

    VertexArrayUnbind();
}

// Initializes a unit square in the first quadrant
// with side length one and centered at the origin
void InitializeUnitSquare()
{
    glm_vec2_copy((vec2) { 0.5f, 0.5f }, UnitSquareVertices[0]);
    glm_vec2_copy((vec2) { -0.5f, 0.5f }, UnitSquareVertices[1]);
    glm_vec2_copy((vec2) { -0.5f, -0.5f }, UnitSquareVertices[2]);
    glm_vec2_copy((vec2) { 0.5f, -0.5f }, UnitSquareVertices[3]);

    VertexArrayInitialize(&UnitSquareVertexArrayId);
    VertexArrayBind(UnitSquareVertexArrayId);

    VertexBufferInitialize(&UnitSquareVertexBuffer, UnitSquareVertices, sizeof(UnitSquareVertices));
    VertexAttribPointerFloats(0, 2);

    VertexArrayUnbind();
}

// Initializes vertex buffers for unit polygons
// Circle, square
void PolygonInitialize()
{
    InitializeUnitCircle();
    InitializeUnitSquare();

    int circlesSize = sizeof(Circle) * MaxCircleCount;
    int rectsSize = sizeof(Rect) * MaxRectCount;

    circles = malloc(circlesSize);
    rects = malloc(rectsSize);

    memset(circles, 0, circlesSize);
    memset(rects, 0, rectsSize);

    UniformBufferInitialize(&circlesBuffer, circles, circlesSize, GL_DYNAMIC_DRAW);
    UniformBufferInitialize(&rectsBuffer, rects, rectsSize, GL_DYNAMIC_DRAW);

    rectShaderId = ShaderCreate(RectVertShaderPath, BasicFragShaderPath);
    circleShaderId = ShaderCreate(CircleVertShaderPath, BasicFragShaderPath);
    lineShaderId = ShaderCreate(LineVertShaderPath, BasicFragShaderPath);

    UniformBufferInitialize(&vpMatrixUB, vpMatrix, sizeof(mat4), GL_DYNAMIC_DRAW);
    ShaderBindUniformBuffer(rectShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(circleShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(lineShaderId, "Matrices", &vpMatrixUB);

    ShaderBindUniformBuffer(circleShaderId, "Circles", &circlesBuffer);
    ShaderBindUniformBuffer(rectShaderId, "Rectangles", &rectsBuffer);

    IsInitialized = true;
}

Circle* PolygonCircle(vec2 position, float radius, vec3 color)
{
    Circle* c = circles + numCircles;
    glm_vec2_copy(position, c->position);
    c->radius = radius;
    glm_vec3_copy(color, c->color);
    numCircles++;
    return c;
}

Rect* PolygonRect(vec2 position, float width, float height, vec3 color)
{
    Rect* r = rects + numRects;
    glm_vec2_copy(position, r->position);
    r->width = width;
    r->height = height;
    glm_vec3_copy(color, r->color);
    numRects++;
    return r;
}

static void DrawCircles()
{
    UniformBufferUpdateRange(&circlesBuffer, 0, sizeof(Circle) * numCircles);
    ShaderUse(circleShaderId);
    VertexArrayBind(UnitCircleVertexArrayId);
    GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, CircleVertexCount + 2, numCircles));
}

static void DrawRects()
{
    UniformBufferUpdateRange(&rectsBuffer, 0, sizeof(Rect) * numRects);
    ShaderUse(rectShaderId);
    VertexArrayBind(UnitSquareVertexArrayId);
    GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, numRects));
}

void PolygonRenderPolygons()
{
    UniformBufferUpdate(&vpMatrixUB);

    if (numCircles > 0)
    {
        DrawCircles();
    }

    if (numRects > 0)
    {
        DrawRects();
    }
}

void PolygonUpdateViewPerspectiveMatrix(mat4 m)
{
    glm_mat4_copy(m, vpMatrix);
}