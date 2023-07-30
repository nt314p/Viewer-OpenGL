#version 330 core

layout(location = 0) in vec3 position;


struct Circle
{
    vec3 color;
    float radius;
    vec2 position;
    vec2 padding;
};

layout (std140) uniform Circles
{
    Circle circles[];
};

uniform mat4 mvpMatrix;

out vec3 vColor;

// gl_InstanceID

void main()
{
    Circle c = circles[gl_InstanceID];
    vec3 pos = position * c.radius + vec3(c.position, 0.0);
    gl_Position = mvpMatrix * vec4(pos, 1.0);
    vColor = c.color;
};