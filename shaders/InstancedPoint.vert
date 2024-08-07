#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inPointSize;
layout(location = 2) in vec3 inColor;

out vec3 vColor;

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

void main() 
{
    gl_Position = vpMatrix * vec4(inPosition, 1.0);
    gl_PointSize = inPointSize;
    vColor = inColor;
};
