#include "polygon.h"

#define CircleVertexCount 60
vec3 UnitCircleVertices[CircleVertexCount + 2];
VertexBuffer UnitCircleVertexBuffer;
bool IsUnitCircleVerticesInitialized = false;

// Specialized triangulation for circles?
// https://www.humus.name/index.php?page=News&ID=228

void PolygonCircle(Polygon* polygon, float radius) {
    // Add two vertices. One for the center and the other for the final vertex
    // that must overlap with the first vertex on the circle
    if (!IsUnitCircleVerticesInitialized) {
        memset(UnitCircleVertices, 0, sizeof(UnitCircleVertices));
        for (int i = 0; i <= CircleVertexCount; i++) {
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

        IsUnitCircleVerticesInitialized = true;
    }

    polygon->vertexCount = CircleVertexCount;
    polygon->vertices = UnitCircleVertices;
    polygon->vertexBuffer = UnitCircleVertexBuffer;

    glm_mat4_identity(polygon->transform);
    glm_mat4_scale(polygon->transform, radius);
}

void PolygonDraw(Polygon* polygon) {
    VertexBufferBind(&polygon->vertexBuffer);
    GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, (polygon->vertexCount + 2)));
    // bugged if not drawing a circle because there shouldn't be +2
}

void PolygonFree(Polygon* polygon) {
    free(polygon->vertices); // yeah this is completely bugged if you free a circle
}