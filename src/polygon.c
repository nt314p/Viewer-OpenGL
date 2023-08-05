#include "polygon.h"

#define CircleVertexCount 50
vec3 UnitCircleVertices[CircleVertexCount + 2]; // change to vec2?
VertexBuffer UnitCircleVertexBuffer;

vec3 UnitSquareVertices[4];
VertexBuffer UnitSquareVertexBuffer;

bool IsInitialized = false;

// Specialized triangulation for circles?
// https://www.humus.name/index.php?page=News&ID=228

void InitializeUnitCircle()
{
    // Add two vertices. One for the center and the other for the final vertex
    // that must overlap with the first vertex on the circle

    memset(UnitCircleVertices, 0, sizeof(UnitCircleVertices));
    for (int i = 0; i <= CircleVertexCount; i++)
    {
        int index = i + 1;
        float angle = (2 * M_PI / CircleVertexCount) * i;

        float x = cosf(angle);
        float y = sinf(angle);
        UnitCircleVertices[index][0] = x;
        UnitCircleVertices[index][1] = y;
    }

    VertexBufferInitialize(&UnitCircleVertexBuffer, UnitCircleVertices, sizeof(UnitCircleVertices));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
}

// Initializes a unit square in the first quadrant
// with side length one and a corner at the origin
void InitializeUnitSquare()
{
    glm_vec2_copy((vec3) { 0.0f, 0.0f, 0.0f }, UnitSquareVertices[0]);
    glm_vec2_copy((vec3) { 1.0f, 0.0f, 0.0f }, UnitSquareVertices[1]);
    glm_vec2_copy((vec3) { 1.0f, 1.0f, 0.0f }, UnitSquareVertices[2]);
    glm_vec2_copy((vec3) { 0.0f, 1.0f, 0.0f }, UnitSquareVertices[3]);

    VertexBufferInitialize(&UnitSquareVertexBuffer, UnitSquareVertices, sizeof(UnitSquareVertices));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
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
    VertexBufferBind(&UnitSquareVertexBuffer);
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