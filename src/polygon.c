#include "polygon.h"

void PolygonCircle(Polygon* polygon, float radius, int vertexCount) {
    polygon->vertexCount = vertexCount;
    polygon->vertices = malloc(sizeof(vec3) * (vertexCount + 2));
    memset(polygon->vertices, 0, sizeof(vec3) * (vertexCount + 2));

    glm_mat4_identity(polygon->transform);
    for (int i = 0; i <= vertexCount; i++) {
        int index = i + 1;
        float angle = (2 * M_PI / vertexCount) * i;

        float x = radius * cosf(angle);
        float y = radius * sinf(angle);
        polygon->vertices[index][0] = x;
        polygon->vertices[index][1] = y;
    }

    VertexBufferInitialize(&polygon->vertexBuffer, polygon->vertices, sizeof(vec3) * (vertexCount + 2));
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
}

void PolygonDraw(Polygon* polygon) {
    VertexBufferBind(&polygon->vertexBuffer);
    GLCall(glDrawArrays(GL_TRIANGLE_FAN, 0, (polygon->vertexCount + 2)));
}

void PolygonFree(Polygon* polygon) {
    free(polygon->vertices);
}