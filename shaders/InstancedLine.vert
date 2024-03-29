#version 330 core

// Represents a line defined by a position p
// and normalized direction v.

layout(location = 0) in vec2 position;

struct Line
{
    vec4 color; // 4th component contains the length of the line
    vec2 p; //
    vec2 v; // normalized direction
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
    vec2 vertPos = l.p + ((gl_VertexID & 1) == 1 ? l.color.a * l.v : 0);
    gl_Position = vpMatrix * vec4(vertPos, 0.0, 1.0);
    vColor = l.color.rgb;
};