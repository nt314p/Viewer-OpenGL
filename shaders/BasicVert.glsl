#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

uniform float angle;
uniform mat4 mvpMatrix;

out vec3 vColor;

void main()
{
   //vec2 rotatedPos = mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * position;
   gl_Position = mvpMatrix * vec4(position, 0.0, 1.0);
   vColor = color;
};