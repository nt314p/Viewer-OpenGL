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
static const char* LineVertShaderPath = "shaders/InstancedLine2D.vert";
static const char* PointVertShaderPath = "shaders/InstancedPoint.vert";
static const char* SurfaceVertShaderPath = "shaders/Surface.vert";

static VertexArray UnitCircleVertexArray;
static VertexArray UnitSquareVertexArray;
static VertexArray PointVertexArray;

static bool IsInitialized = false;

static const int MaxCircleCount = 2048;
static const int MaxRectCount = 2048;
static const int MaxLineCount = 2048;
static const int MaxPointCount = 2048;

static Circle* circles;
static Rect* rects;
static Line2D* line2Ds;
static Point* points;

static UniformBuffer circlesBuffer;
static UniformBuffer rectsBuffer;
static UniformBuffer line2DsBuffer;

static unsigned int numCircles;
static unsigned int numRects;
static unsigned int numLine2Ds;
static unsigned int numPoints;

static mat4 vpMatrix;
static UniformBuffer vpMatrixUB;

static unsigned int circleShaderId;
static unsigned int line2DShaderId;
static unsigned int rectShaderId;
static unsigned int pointShaderId;
static unsigned int surfaceShaderId;

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
    vec2 UnitSquareVertices[4];

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

static void InitializePointVertexArray()
{
    VertexArrayInitialize(&PointVertexArray);
    VertexArrayBind(&PointVertexArray);

    VertexBufferInitialize(&PointVertexArray, points, sizeof(Point) * MaxPointCount, GL_DYNAMIC_DRAW);
    VertexAttribPointerFloats(0, 3); // vec3 position
    VertexAttribPointerFloats(1, 1); // float pointSize
    VertexAttribPointerFloats(2, 3); // vec3 color

    VertexArrayUnbind();
}

void PolygonInitialize()
{
    InitializeUnitCircle();
    InitializeUnitSquare();

    // Initialize primitive buffers
    int circlesSize = sizeof(Circle) * MaxCircleCount;
    int rectsSize = sizeof(Rect) * MaxRectCount;
    int linesSize = sizeof(Line2D) * MaxLineCount;
    int pointsSize = sizeof(Point) * MaxPointCount;

    circles = malloc(circlesSize);
    rects = malloc(rectsSize);
    line2Ds = malloc(linesSize);
    points = malloc(pointsSize);

    memset(circles, 0, circlesSize);
    memset(rects, 0, rectsSize);
    memset(line2Ds, 0, linesSize);
    memset(points, 0, pointsSize);

    InitializePointVertexArray(); // requires that `points` points to valid memory

    UniformBufferInitialize(&circlesBuffer, circles, circlesSize, GL_DYNAMIC_DRAW);
    UniformBufferInitialize(&rectsBuffer, rects, rectsSize, GL_DYNAMIC_DRAW);
    UniformBufferInitialize(&line2DsBuffer, line2Ds, linesSize, GL_DYNAMIC_DRAW);

    rectShaderId = ShaderCreate(RectVertShaderPath, BasicFragShaderPath);
    circleShaderId = ShaderCreate(CircleVertShaderPath, BasicFragShaderPath);
    line2DShaderId = ShaderCreate(LineVertShaderPath, BasicFragShaderPath);
    pointShaderId = ShaderCreate(PointVertShaderPath, BasicFragShaderPath);
    surfaceShaderId = ShaderCreate(SurfaceVertShaderPath, BasicFragShaderPath);

    UniformBufferInitialize(&vpMatrixUB, vpMatrix, sizeof(mat4), GL_DYNAMIC_DRAW);
    ShaderBindUniformBuffer(rectShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(circleShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(line2DShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(pointShaderId, "Matrices", &vpMatrixUB);
    ShaderBindUniformBuffer(surfaceShaderId, "Matrices", &vpMatrixUB);

    ShaderBindUniformBuffer(circleShaderId, "Circles", &circlesBuffer);
    ShaderBindUniformBuffer(rectShaderId, "Rectangles", &rectsBuffer);
    ShaderBindUniformBuffer(line2DShaderId, "Lines", &line2DsBuffer);

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

Line2D* PolygonLine2D(vec2 a, vec2 b, vec3 color)
{
    Line2D* l = line2Ds + numLine2Ds;
    glm_vec2_copy(a, l->a);
    glm_vec2_copy(b, l->b);

    glm_vec3_copy(color, l->color);
    numLine2Ds++;
    return l;
}

Line2D* PolygonLine2Ds(unsigned int count)
{
    Line2D* ls = line2Ds + numLine2Ds;
    numLine2Ds += count;
    return ls;
}

void SurfaceInitialize(Surface* surface, vec3 origin, float scale, vec4* data, uint32_t n)
{
    glm_vec3_copy(origin, surface->origin);
    surface->scale = scale;
    surface->n = n;

    int dataSize = sizeof(vec4) * n * n;
    surface->vertices = data;

    VertexArray* va = &surface->vertexArray;

    VertexArrayInitialize(va);
    VertexArrayBind(va);

    VertexBufferInitialize(va, surface->vertices, dataSize, GL_DYNAMIC_DRAW);
    VertexAttribPointerFloats(0, 1); // height
    VertexAttribPointerFloats(1, 3); // color rgb

    // An n x n grid contains (n - 1) x (n - 1) squares.
    // So we need 2 * (n - 1) * (n - 1) triangles,
    // which have 3 * 2 * (n - 1) * (n - 1) vertices.
    int numVertices = 3 * 2 * (n - 1) * (n - 1);
    unsigned int* indexData = malloc(sizeof(unsigned int) * numVertices);

    /* Triangulation
    (x,y)
     a---b
     |  /|
     | / |
     |/  |
     c---d (x + 1, y + 1)
    */

    int vertIndex = 0;
    for (int x = 0; x < n - 1; x++)
    {
        for (int y = 0; y < n - 1; y++)
        {
            int a = x + y * n;
            int b = a + 1;
            int c = a + n;
            int d = c + 1;
            indexData[vertIndex + 0] = a; // top left tri
            indexData[vertIndex + 1] = b;
            indexData[vertIndex + 2] = c;

            indexData[vertIndex + 3] = b; // bottom right tri
            indexData[vertIndex + 4] = d;
            indexData[vertIndex + 5] = c;
            vertIndex += 6;
        }
    }

    IndexBufferInitialize(va, indexData, numVertices, GL_STATIC_DRAW);
    free(indexData); // TODO: hopefully we don't access this data again

    VertexArrayUnbind();
}

void SurfaceDraw(Surface* surface)
{
    VertexArray* va = &surface->vertexArray;
    VertexBufferUpdate(va);
    ShaderUse(surfaceShaderId);
    VertexArrayBind(va);
    GLCall(glDrawElements(GL_TRIANGLES, va->indexBufferCount, GL_UNSIGNED_INT, 0))
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

static void DrawLine2Ds()
{
    UniformBufferUpdateRange(&line2DsBuffer, 0, sizeof(Line2D) * numLine2Ds);
    ShaderUse(line2DShaderId);
    VertexArrayBind(&UnitSquareVertexArray); // why do we bind unit square for lines??
    GLCall(glDrawArraysInstanced(GL_LINES, 0, VerticesPerLine, numLine2Ds));
}

static void DrawPoints()
{
    VertexBufferUpdateRange(&PointVertexArray, 0, sizeof(Point) * numPoints);
    ShaderUse(pointShaderId);
    VertexArrayBind(&PointVertexArray);
    GLCall(glDrawArrays(GL_POINT, 0, numPoints)); // TODO: `glEnable(GL_VERTEX_PROGRAM_POINT_SIZE)` somewhere
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

    if (numLine2Ds > 0)
    {
        DrawLine2Ds();
    }

    if (numPoints > 0)
    {
        DrawPoints();
    }
}

void PolygonUpdateViewPerspectiveMatrix(mat4 m)
{
    glm_mat4_copy(m, vpMatrix);
}