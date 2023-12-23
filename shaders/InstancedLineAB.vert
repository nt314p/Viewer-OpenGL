#version 330 core

layout(location = 0) in vec2 position;

struct Line
{
    vec4 color;
    vec2 a;
    vec2 b;
};

layout (std140) uniform Lines
{
    Line lines[2048];
};

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

void main()
{
    Line l = lines[gl_InstanceID];
    vec2 vertPos = (gl_VertexID & 1) == 1 ? l.a : l.b;
    gl_Position = vpMatrix * vec4(vertPos, 0.0, 1.0);
    vColor = l.color.rgb;
};