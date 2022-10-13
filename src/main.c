#include "renderer.h"
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <cglm\cglm.h>
#include <stdio.h>
#include <malloc.h>

#define WIDTH 640
#define HEIGHT 480

float x = 0;
float z = -3.0f;

// Loads the file at filePath and returns a pointer to the string contents
static char* LoadShader(const char* filePath)
{
    FILE* file = fopen(filePath, "r");

    if (file == NULL)
    {
        printf("Error opening file at path: %s", filePath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    char* buffer = malloc(length + 1);

    fseek(file, 0, SEEK_SET);
    long actualLength = fread(buffer, 1, length, file);
    buffer[actualLength] = '\0';

    fclose(file);
    return buffer;
}

static unsigned int CompileShader(unsigned int type, const char* source)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, NULL, message);
        printf("Failed to compile %s shader!\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment");
        printf(message);
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const char* vertexShader, const char* fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) x += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) x -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) z += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) z -= 0.1f;
}

int main(void)
{
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

    char* vertexShader = LoadShader("shaders/BasicVert.glsl");
    char* fragmentShader = LoadShader("shaders/BasicFrag.glsl");

    unsigned int shader = CreateShader(vertexShader, fragmentShader);

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

    float angle = 0;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        mat4 model, view, projection, mvpMatrix;
        glm_mat4_identity(model);
        glm_rotate(model, angle, (vec3) { 1.0f, 1.0f, 0.0f });
        glm_mat4_identity(view);
        glm_translate(view, (vec3) { -x, 0.0f, z });
        glm_perspective(glm_rad(45.0f), ((float)WIDTH) / HEIGHT, 0.1f, 100.0f, projection);
        glm_mat4_mul(view, model, mvpMatrix);
        glm_mat4_mul(projection, mvpMatrix, mvpMatrix);

        angle += 0.01;
        GLCall(glUniform1f(location, angle));

        GLCall(glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, mvpMatrix[0]));

        /* Render here */
        GLCall(glClearColor(0.1f, 0.15f, 0.2f, 1.0f));
        GLCall(glClear(GL_COLOR_BUFFER_BIT));

        GLCall(glBindVertexArray(vao));
        IndexBufferBind(&ib);

        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shader);
    free(vertexShader);
    free(fragmentShader);

    glfwTerminate();
    return 0;
}