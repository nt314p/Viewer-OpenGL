#version 330 core

layout(location = 0) in vec2 position;

struct Circle
{
    vec4 color;
    vec2 position;
    float radius;
    float padding2;
};

layout (std140) uniform Circles
{
    Circle circles[2048];
};

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

void main()
{
    Circle c = circles[gl_InstanceID];
    vec3 pos = vec3(position * c.radius + c.position, 0.0);
    gl_Position = vpMatrix * vec4(pos, 1.0);
    vColor = c.color.rgb;
};