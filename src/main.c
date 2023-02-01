#include "renderer.h"
#include "shader.h"
#include "camera.h"
#include <cglm\cglm.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>

const int WIDTH = 640;
const int HEIGHT = 480;

static void ProcessInput(GLFWwindow* window);
static void MouseCallback(GLFWwindow* window, double x, double y);

static float deltaTime;
static float lastFrame;
static vec2 mouseCoords;
static vec2 mouseDelta;

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

    if (glewInit() != GLEW_OK)
        printf("Error initializing glew!\n");

    printf("%s\n", glGetString(GL_VERSION));

    float positions[] = {
        -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 1.0f, 0.0f, 1.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    VertexBuffer vb;
    VertexBufferInitialize(&vb, positions, sizeof(positions));

    // position attribute (stride is 5 * sizeof(float))
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0));
    GLCall(glEnableVertexAttribArray(0));

    // color attribute (offset is 2 * sizeof(float))
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(2 * sizeof(float))));
    GLCall(glEnableVertexAttribArray(1));

    IndexBuffer ib;
    IndexBufferInitialize(&ib, indices, 6);

    unsigned int vertexShaderId = CompileShader(GL_VERTEX_SHADER, "shaders/BasicVert.glsl");
    unsigned int fragmentShaderId = CompileShader(GL_FRAGMENT_SHADER, "shaders/BasicFrag.glsl");
    unsigned int shader = CreateShader(vertexShaderId, fragmentShaderId);

    GLCall(int location = glGetUniformLocation(shader, "angle"));
    GLCall(int mvpLocation = glGetUniformLocation(shader, "mvpMatrix"));
    // GLCall(int colorLocation = glGetUniformLocation(shader, "Color"));
    // ASSERT(colorLocation != -1);
    // GLCall(glUniform4f(colorLocation, 0.2f, 0.3f, 0.8f, 1.0f));

    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));

    GLCall(glUseProgram(shader));

    GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));

    float angle = 0;

    CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ProcessInput(window);

        mat4 model, mvpMatrix;

        glm_mat4_identity(model);
        glm_rotate(model, angle, (vec3) { 1.0f, 1.0f, 0.0f });
        CameraViewPerspectiveMatrix(mvpMatrix);
        glm_mat4_mul(mvpMatrix, model, mvpMatrix);

        angle += 0.01;
        GLCall(glUniform1f(location, angle));

        GLCall(glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvpMatrix[0]));

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glBindVertexArray(vao));
        IndexBufferBind(&ib);

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}

float yaw;
float pitch;

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

    CameraRotate(yaw, pitch, 0.0f);
}

void ProcessInput(GLFWwindow* window)
{
    float speed = 4.0f;
    vec3 movement = { 0.0f };

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        movement[0] += 1;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        movement[0] -= 1;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        movement[2] -= 1;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        movement[2] += 1;
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}