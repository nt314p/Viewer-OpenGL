#version 330 core

layout(location = 0) in vec2 position;


struct Circle
{
    vec3 color;
    float padding;
    vec2 position;
    float radius;
    float padding2;
};

layout (std140) uniform Circles
{
    Circle circles[2000];
};

layout (std140) uniform Matrices
{
    mat4 vpMatrix;
};

out vec3 vColor;

// gl_InstanceID

void main()
{
    Circle c = circles[gl_InstanceID];
    vec3 pos = vec3(position, 0.0) * c.radius + vec3(c.position, 0.0);
    gl_Position = vpMatrix * vec4(pos, 1.0);
    vColor = c.color;
};