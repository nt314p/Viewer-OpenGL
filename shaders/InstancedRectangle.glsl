#version 330 core

layout(location = 0) in vec2 position;

struct Rectangle
{
    vec3 color;
    float padding;
    vec2 position;
    float width;
    float height;
};

layout (std140) uniform Rectangles
{
    Rectangle rectangles[200];
};

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

void main()
{
    Rectangle r = rectangles[gl_InstanceID];
    vec3 pos = vec2(position.x * r.width, position.y * r.height) + vec3(r.position, 0.0);
    gl_Position = vpMatrix * vec4(pos, 1.0);
    vColor = r.color;
};