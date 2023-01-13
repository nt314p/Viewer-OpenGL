#include "renderer.h"
#include "shader.h"
#include <cglm\cglm.h>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <stdio.h>
#include <malloc.h>

const int WIDTH = 640;
const int HEIGHT = 480;

void ProcessInput(GLFWwindow* window);
void MouseCallback(GLFWwindow* window, double x, double y);

float deltaTime;
float lastFrame;

vec3 cameraTarget = { 0.0f };
vec3 cameraDirection, cameraRight, cameraUp;
vec3 up = { 0.0f, 1.0f, 0.0f };

vec3 cameraPos = { 0.0f, 0.0f,  3.0f };
vec3 cameraFront = { 0.0f, 0.0f, -1.0f };
vec3 cameraUp = { 0.0f, 1.0f,  0.0f };
vec3 direction;

int main(void)
{
    glm_vec3_sub(cameraPos, cameraTarget, cameraDirection);
    glm_cross(up, cameraDirection, cameraRight);
    glm_normalize(cameraRight);
    glm_cross(cameraDirection, cameraRight, cameraUp);

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(WIDTH, HEIGHT, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
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

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ProcessInput(window);

        mat4 model, view, projection, mvpMatrix;

        vec3 target;

        glm_vec3_add(cameraPos, cameraFront, target);
        glm_lookat(cameraPos, target, cameraUp, view);

        glm_mat4_identity(model);
        glm_rotate(model, angle, (vec3) { 1.0f, 1.0f, 0.0f });
        glm_perspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f, projection);
        glm_mat4_mul(view, model, mvpMatrix);
        glm_mat4_mul(projection, mvpMatrix, mvpMatrix);

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


}

void ProcessInput(GLFWwindow* window)
{
    float speed = 2.0f;
    vec3 movement = { 0.0f };
    vec3 cross;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        glm_vec3_cross(cameraFront, cameraUp, cross);
        glm_normalize(cross);
        glm_vec3_add(movement, cross, movement);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        glm_vec3_cross(cameraFront, cameraUp, cross);
        glm_normalize(cross);
        glm_vec3_sub(movement, cross, movement);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        glm_vec3_add(movement, cameraFront, movement);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        glm_vec3_sub(movement, cameraFront, movement);
    }

    glm_vec3_muladds(movement, speed * deltaTime, cameraPos);
}