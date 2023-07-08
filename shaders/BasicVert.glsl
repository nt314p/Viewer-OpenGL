#version 330 core

layout(location = 0) in vec3 position;
//layout(location = 1) in vec3 color;

uniform float angle;
uniform mat4 mvpMatrix;

out vec3 vColor;

void main()
{
   //vec2 rotatedPos = mat2(cos(angle), -sin(angle), sin(angle), cos(angle)) * position;
   gl_Position = mvpMatrix * vec4(position, 1.0);
   vColor = sin(position/2)/2 + 0.5;
};