#version 330 core

struct Line2D
{
    vec4 color;
    vec2 a; // endpoint a
    vec2 b; // endpoint b
};

layout (std140) uniform Lines
{
    Line2D lines[2048];
};

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

void main()
{
    Line2D l = lines[gl_InstanceID];
    vec2 vertPos = mix(l.a, l.b, float(gl_VertexID & 1));
    gl_Position = vpMatrix * vec4(vertPos, 0.0, 1.0);
    vColor = l.color.rgb;
};