#include "polygon.h"
#include "debug.h"

#define CircleVertexCount 50
static vec2 UnitCircleVertices[CircleVertexCount + 2];
static VertexBuffer UnitCircleVertexBuffer;
static unsigned int UnitCircleVertexArrayId;

static vec2 UnitSquareVertices[4];
static VertexBuffer UnitSquareVertexBuffer;
static unsigned int UnitSquareVertexArrayId;

bool IsInitialized = false;

// Specialized triangulation for circles?
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
    IsInitialized = true;
}

void PolygonCircle(Polygon* polygon, float radius)
{
    polygon->vertexCount = CircleVertexCount;
    polygon->vertices = UnitCircleVertices;
    polygon->vertexBuffer = UnitCircleVertexBuffer;

    glm_mat4_identity(polygon->transform);
    glm_mat4_scale(polygon->transform, radius);
}

// Binds the unit circle vertex buffer
void PolygonBindUnitCircle()
{
    VertexArrayBind(UnitCircleVertexArrayId);
}

// Binds the unit square vertex buffer
void PolygonBindUnitSquare()
{
    VertexArrayBind(UnitSquareVertexArrayId);
}

void PolygonDraw(Polygon* polygon)
{
    VertexBufferBind(&polygon->vertexBuffer);
    GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, (polygon->vertexCount + 2)));
    // bugged if not drawing a circle because there shouldn't be +2
}

void PolygonFree(Polygon* polygon)
{
    free(polygon->vertices); // yeah this is completely bugged if you free a circle
}