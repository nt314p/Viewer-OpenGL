#include <string.h>
#include <cglm\cglm.h>
#include "polygon.h"
#include "shader.h"
#include "renderer.h"
#include "debug.h"

// The maximum depth used for the circle generation algorithm
// A higher level yields a better circle approximation
// See https://www.humus.name/index.php?page=News&ID=228
#define MaxCircleGenLevel 4 
#define CircleVertexCount (3 * (1 << MaxCircleGenLevel))
#define CircleTriangles (3 * ((1 << MaxCircleGenLevel) - 1) + 1)
static const int VerticesPerLine = 2;
static const int VerticesPerRect = 4;

static const char* BasicFragShaderPath = "shaders/BasicFrag.frag";
static const char* RectVertShaderPath = "shaders/InstancedRect.vert";
static const char* CircleVertShaderPath = "shaders/InstancedCircle.vert";
static const char* LineVertShaderPath = "shaders/InstancedLine.vert";

static VertexArray UnitCircleVertexArray; 
// static VertexBuffer UnitCircleVertexBuffer;
// static unsigned int UnitCircleVertexArrayId;

static vec2 UnitSquareVertices[4];
//static VertexBuffer UnitSquareVertexBuffer;
static VertexArray UnitSquareVertexArray;

static bool IsInitialized = false;

static const int MaxCircleCount = 2048;
static const int MaxRectCount = 2048;
static const int MaxLineCount = 2048;

static Circle* circles;
static Rect* rects;
static Line* lines;

static UniformBuffer circlesBuffer;
static UniformBuffer rectsBuffer;
static UniformBuffer linesBuffer;

static unsigned int numCircles;
static unsigned int numRects;
static unsigned int numLines;

static mat4 vpMatrix;
static UniformBuffer vpMatrixUB;

static unsigned int circleShaderId;
static unsigned int lineShaderId;
static unsigned int rectShaderId;

static void InitializeUnitCircle()
{
    vec2 vertLookup[CircleVertexCount];

    for (int i = 0; i < CircleVertexCount; i++)
    {
        float angle = (2 * M_PI / CircleVertexCount) * i;

        vertLookup[i][0] = cosf(angle);
        vertLookup[i][1] = sinf(angle);
    }

    // printf("CircleVertexCount: %d\n", CircleVertexCount);
    // printf("CircleTriangles: %d\n", CircleTriangles);

    vec2* vertData = malloc(sizeof(vec2) * CircleTriangles * 3);
    int globalIndex = 0;

    // render n = 4 levels
    // vertex count = 3 * 2^n = 3 * 2^4 = 48

    for (int n = 0; n <= MaxCircleGenLevel; n++) {
        int triangleCount = n == 0 ? 1 : 3 * (1 << (n - 1));
        int stride = 1 << (MaxCircleGenLevel - n);
        // printf("triangleCount: %d\n", triangleCount);
        // printf("stride: %d\n", stride);

        for (int i = 0; i < triangleCount; i++)
        {
            int vIndex = i * stride * 2; // base vertex index in lookup

            glm_vec2_copy(vertLookup[vIndex], vertData[globalIndex]);
            glm_vec2_copy(vertLookup[vIndex + stride], vertData[globalIndex + 1]);
            glm_vec2_copy(vertLookup[(vIndex + 2 * stride) % CircleVertexCount], vertData[globalIndex + 2]);

            // printf("%d, %d, %d\n", vertexIndex, vertexIndex + stride, (vertexIndex + 2 * stride) % CircleVertexCount);
            globalIndex += 3;
        }
    }

    VertexArrayInitialize(&UnitCircleVertexArray);
    VertexArrayBind(&UnitCircleVertexArray);

    VertexBufferInitialize(&UnitCircleVertexArray, vertData, 
        sizeof(vec2) * CircleTriangles * 3, GL_STATIC_DRAW);

    VertexAttribPointerFloats(0, 2);

    VertexArrayUnbind();
    free(vertData);
}

// Initializes a unit square (side length one) centered at the origin
static void InitializeUnitSquare()
{
    glm_vec2_copy((vec2) { 0.5f, 0.5f }, UnitSquareVertices[0]);
    glm_vec2_copy((vec2) { -0.5f, 0.5f }, UnitSquareVertices[1]);
    glm_vec2_copy((vec2) { -0.5f, -0.5f }, UnitSquareVertices[2]);
    glm_vec2_copy((vec2) { 0.5f, -0.5f }, UnitSquareVertices[3]);

    VertexArrayInitialize(&UnitSquareVertexArray);
    VertexArrayBind(&UnitSquareVertexArray);

    VertexBufferInitialize(&UnitSquareVertexArray, UnitSquareVertices, 
        sizeof(UnitSquareVertices), GL_STATIC_DRAW);

    VertexAttribPointerFloats(0, 2);

    VertexArrayUnbind();
}

void PolygonInitialize()
{
    InitializeUnitCircle();
    InitializeUnitSquare();

    // Initialize primitive buffers
    int circlesSize = sizeof(Circle) * MaxCircleCount;
    int rectsSize = sizeof(Rect) * MaxRectCount;
    int linesSize = sizeof(Line) * MaxLineCount;

    circles = malloc(circlesSize);
    rects = malloc(rectsSize);
    lines = malloc(linesSize);

    memset(circles, 0, circlesSize);
    memset(rects, 0, rectsSize);
    memset(lines, 0, linesSize);

    UniformBufferInitialize(&circlesBuffer, circles, circlesSize, GL_DYNAMIC_DRAW);
    UniformBufferInitialize(&rectsBuffer, rects, rectsSize, GL_DYNAMIC_DRAW);
    UniformBufferInitialize(&linesBuffer, lines, linesSize, GL_DYNAMIC_DRAW);

    rectShaderId = ShaderCreate(RectVertShaderPath, BasicFragShaderPath);
    circleShaderId = ShaderCreate(CircleVertShaderPath, BasicFragShaderPath);
    lineShaderId = ShaderCreate(LineVertShaderPath, BasicFragShaderPath);

    UniformBufferInitialize(&vpMatrixUB, vpMatrix, sizeof(mat4), GL_DYNAMIC_DRAW);
    ShaderBindUniformBuffer(rectShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(circleShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(lineShaderId, "Matrices", &vpMatrixUB);

    ShaderBindUniformBuffer(circleShaderId, "Circles", &circlesBuffer);
    ShaderBindUniformBuffer(rectShaderId, "Rectangles", &rectsBuffer);
    ShaderBindUniformBuffer(lineShaderId, "Lines", &linesBuffer);

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

Circle* PolygonCircles(unsigned int count)
{
    Circle* cs = circles + numCircles;
    numCircles += count;
    return cs;
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

Rect* PolygonRects(unsigned int count)
{
    Rect* rs = rects + numRects;
    numRects += count;
    return rs;
}

Line* PolygonLine(vec2 a, vec2 b, vec3 color)
{
    Line* l = lines + numLines;
    glm_vec2_copy(a, l->a);
    glm_vec2_sub(b, a, l->b);
    float length = glm_vec2_norm(l->b);
    l->length = length;

    l->b[0] /= length;
    l->b[1] /= length;

    glm_vec3_copy(color, l->color);
    numLines++;
    return l;
}

Line* PolygonLines(unsigned int count)
{
    Line* ls = lines + numLines;
    numLines += count;
    return ls;
}

static void DrawCircles()
{
    UniformBufferUpdateRange(&circlesBuffer, 0, sizeof(Circle) * numCircles);
    ShaderUse(circleShaderId);
    VertexArrayBind(&UnitCircleVertexArray);
    GLCall(glDrawArraysInstanced(GL_TRIANGLES, 0, CircleTriangles * 3, numCircles));
}

static void DrawRects()
{
    UniformBufferUpdateRange(&rectsBuffer, 0, sizeof(Rect) * numRects);
    ShaderUse(rectShaderId);
    VertexArrayBind(&UnitSquareVertexArray);
    GLCall(glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, VerticesPerRect, numRects));
}

static void DrawLines()
{
    UniformBufferUpdateRange(&linesBuffer, 0, sizeof(Line) * numLines);
    ShaderUse(lineShaderId);
    VertexArrayBind(&UnitSquareVertexArray); // why do we bind unit square for lines??
    GLCall(glDrawArraysInstanced(GL_LINES, 0, VerticesPerLine, numLines));
}

void PolygonRenderPolygons()
{
    UniformBufferUpdate(&vpMatrixUB);

    if (numRects > 0)
    {
        DrawRects();
    }

    if (numCircles > 0)
    {
        DrawCircles();
    }

    if (numLines > 0)
    {
        DrawLines();
    }
}

void PolygonUpdateViewPerspectiveMatrix(mat4 m)
{
    glm_mat4_copy(m, vpMatrix);
}