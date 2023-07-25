#version 330 core

struct Circle
{
    float radius;
    vec2 position;
    vec3 color;
};

layout(location = 0) in vec3 position;

uniform Circle circles[10];
uniform mat4 mvpMatrix;

out vec3 vColor;

// gl_InstanceID

void main()
{
    Circle c = circles[gl_InstanceID];
    vec3 pos = position * c.radius + vec3(c.position, 0.0);
    gl_Position = mvpMatrix * vec4(position + vec3(gl_InstanceID * 4.0, 0.0, 0.0), 1.0);
    vColor = c.color;
};