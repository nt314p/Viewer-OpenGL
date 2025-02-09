#version 330 core

// Represents a line defined by a position p
// and normalized direction v.

struct Line
{
    vec4 color;
    vec3 a; // endpoint a
    float thickness; // line thickness
    vec3 b; // endpoint b
    float dummy;
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
    vec3 vertPos = mix(l.a, l.b, float(gl_VertexID & 1));
    
    gl_Position = vpMatrix * vec4(vertPos, 1.0);
    vColor = l.color.rgb;
};