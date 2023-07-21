#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include "model_parser.h"
#include "polygon.h"
#include <cglm\cglm.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>

const int WIDTH = 1280;
const int HEIGHT = 720;

static void ProcessInput(GLFWwindow* window);
static void MouseCallback(GLFWwindow* window, double x, double y);
static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);

static float deltaTime;
static float lastFrame;
static vec2 mouseCoords;
static vec2 scrollDelta;
static vec2 mouseDelta;

float yaw;
float pitch;

float RandomRange(float min, float max) {
    return rand() * (max - min) / RAND_MAX + min;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Viewer", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);

    if (glewInit() != GLEW_OK)
        printf("Error initializing glew!\n");

    printf("%s\n", glGetString(GL_VERSION));

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    unsigned int vertexShaderId = CompileShader(GL_VERTEX_SHADER, "shaders/BasicVert.glsl");
    unsigned int fragmentShaderId = CompileShader(GL_FRAGMENT_SHADER, "shaders/BasicFrag.glsl");
    unsigned int shaderId = CreateShader(vertexShaderId, fragmentShaderId);

    int mvpMatrixId = GetShaderUniformId(shaderId, "mvpMatrix");

    GLCall(glUseProgram(shaderId));

    //GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    const int numCircles = 2000;
    Polygon* circles = malloc(sizeof(Polygon) * numCircles);
    for (int i = 0; i < numCircles; i++) {
        Polygon* circle = circles + i;
        PolygonCircle(circle, RandomRange(0.5, 2));
        float x = RandomRange(-10, 10);
        float y = RandomRange(-10, 10);
        float z = RandomRange(-10, 10);
        vec4* transform = (circle)->transform;
        glm_translate(transform, (vec3) { x, y, z });

        x = RandomRange(-10, 10);
        y = RandomRange(-10, 10);
        z = RandomRange(-10, 10);
        glm_rotate(transform, RandomRange(0, 2 * M_PI), (vec3) { x, y, z });
    }

    //CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);
    CameraUseOrthographic(((float)WIDTH) / HEIGHT, 10.0f);

    mat4 vpMatrix, mvpMatrix;

    float zoom = 10.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        printf("%d\n", (int)(1 / deltaTime));

        ProcessInput(window);

        //CameraRotate(yaw, pitch, 0.0f);

        zoom += scrollDelta[1];
        zoom = fmax(zoom, 0.1f);
        CameraZoom(zoom);

        CameraViewPerspectiveMatrix(vpMatrix);

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        VertexBufferBind(&circles->vertexBuffer);
        for (int i = 0; i < numCircles; i++) {
            Polygon* circle = circles + i;
            glm_mat4_mul(vpMatrix, circle->transform, mvpMatrix);
            GLCall(glUniformMatrix4fv(mvpMatrixId, 1, GL_FALSE, mvpMatrix[0]));
            PolygonDraw(circle);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderId);

    glfwTerminate();
    return 0;
}

static void ScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    scrollDelta[0] = xOffset;
    scrollDelta[1] = yOffset;
}

void MouseCallback(GLFWwindow* window, double x, double y)
{
    static float prevX, prevY;
    static bool firstCall = true;

    if (firstCall)
    {
        prevX = x;
        prevY = y;
        firstCall = false;
    }

    float dx = x - prevX;
    float dy = -(y - prevY);

    prevX = x;
    prevY = y;

    mouseCoords[0] = x;
    mouseCoords[1] = y;
    mouseDelta[0] = dx;
    mouseDelta[1] = dy;

    yaw -= dx * deltaTime * 6;
    pitch += dy * deltaTime * 6;

    //printf("dx: %f; dy: %f\n", dx, dy);
}

int KeyPressed(GLFWwindow* window, int key);

inline int KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void ProcessInput(GLFWwindow* window)
{
    float speed = 6.0f;
    vec3 movement = { 0.0f };

    if (KeyPressed(window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (KeyPressed(window, GLFW_KEY_D))
    {
        movement[0] += 1;
    }

    if (KeyPressed(window, GLFW_KEY_A))
    {
        movement[0] -= 1;
    }

    if (KeyPressed(window, GLFW_KEY_W))
    {
        movement[1] += 1;
    }

    if (KeyPressed(window, GLFW_KEY_S))
    {
        movement[1] -= 1;
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}