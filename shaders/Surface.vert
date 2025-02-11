#version 330 core

layout(location = 0) in float height;
layout(location = 1) in vec3 color;

uniform vec3 origin;
uniform float scale;
uniform int n;

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

void main()
{
    float x = mod(float(gl_VertexID), float(n));
    float y = floor(float(gl_VertexID) / float(n));

    vec2 coord = vec2(x, y) * scale;
    vec3 pos = vec3(coord.x, height, coord.y) + origin; // Assume y is up

    gl_Position = vpMatrix * vec4(pos, 1.0);
    vColor = vec3(x / n, y / n, 1.0);
}