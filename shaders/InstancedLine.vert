#version 330 core

layout(location = 0) in vec2 position;

layout (std140) uniform LineColors
{
    vec4 colors[4096];
};

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

void main()
{
    gl_Position = vpMatrix * vec4(position, 0.0, 1.0);
    vColor = colors[gl_VertexID >> 1].rgb;
};