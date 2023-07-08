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

const int WIDTH = 800;
const int HEIGHT = 600;

static void ProcessInput(GLFWwindow* window);
static void MouseCallback(GLFWwindow* window, double x, double y);

static float deltaTime;
static float lastFrame;
static vec2 mouseCoords;
static vec2 mouseDelta;

// Returns the length of the file
int ReadModel(const char* filePath, char* buffer)
{
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Error opening file at path: %s\n", filePath);
        return -1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);

    fseek(file, 0, SEEK_SET);
    long actualLength = fread(buffer, 1, length, file);
    buffer[actualLength] = '\0';
    fclose(file);

    return actualLength;
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

    if (glewInit() != GLEW_OK)
        printf("Error initializing glew!\n");

    printf("%s\n", glGetString(GL_VERSION));

    char* buffer = malloc(32 * 1024); // allocate 32K

    int modelLength = ReadModel("elephant.obj", buffer);
    if (modelLength == -1) return -1;

    int modelCounts[2];
    GetModelBufferCounts(buffer, modelLength, modelCounts);
    printf("V: %d; F: %d\n", modelCounts[0], modelCounts[1]);

    unsigned int vertexCount = modelCounts[0];
    unsigned int faceCount = modelCounts[1];
    float* vertices = malloc(vertexCount * 3 * sizeof(float));
    unsigned int* faces = malloc(faceCount * 3 * sizeof(unsigned int));

    ParseModel(buffer, modelLength, vertices, vertexCount, faces, faceCount);

    // for (int i = 0; i < vertexCount * 3; i += 3)
    // {
    //     printf("V: %f %f %f\n", vertices[i], vertices[i + 1], vertices[i + 2]);
    // }

    // for (int i = 0; i < faceCount * 3; i += 3)
    // {
    //     printf("F: %d %d %d\n", faces[i], faces[i + 1], faces[i + 2]);
    // }

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    VertexBuffer vb;
    VertexBufferInitialize(&vb, vertices, vertexCount * 3 * sizeof(float));

    // position attribute (stride is 5 * sizeof(float))
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
    GLCall(glEnableVertexAttribArray(0));

    // color attribute (offset is 2 * sizeof(float))
    // GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
    //     5 * sizeof(float), (void*)(2 * sizeof(float))));
    // GLCall(glEnableVertexAttribArray(1));

    IndexBuffer ib;
    IndexBufferInitialize(&ib, faces, faceCount * 3);

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

    GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));

    float angle = 0;
    free(buffer);
    
    GLCall(glBindVertexArray(vao));

    Polygon circle;
    PolygonCircle(&circle, 3, 1024);

    CameraUsePerspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        ProcessInput(window);

        mat4 model, mvpMatrix;

        glm_mat4_identity(model);
        // glm_rotate(model, angle, (vec3) { 0.0f, 1.0f, 0.0f });
        CameraViewPerspectiveMatrix(mvpMatrix);
        glm_mat4_mul(mvpMatrix, model, mvpMatrix);

        //angle += 0.01;
        GLCall(glUniform1f(location, angle));

        GLCall(glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvpMatrix[0]));

        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glBindVertexArray(vao));
        // VertexBufferBind(&vb);
        // IndexBufferBind(&ib);

        // faceCount * 3
        //GLCall(glDrawElements(GL_TRIANGLES, faceCount * 3, GL_UNSIGNED_INT, NULL));
        
        PolygonDraw(&circle);

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

int KeyPressed(GLFWwindow* window, int key);

inline int KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void ProcessInput(GLFWwindow* window)
{
    float speed = 4.0f;
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
        movement[2] -= 1;
    }

    if (KeyPressed(window, GLFW_KEY_S))
    {
        movement[2] += 1;
    }

    glm_vec3_scale(movement, deltaTime * speed, movement);

    CameraTranslateRelative(movement);
}