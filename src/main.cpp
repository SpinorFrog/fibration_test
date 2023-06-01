#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "computeshader.h"
#include "textures.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera.h"

//backend processes (window and i/o)
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

//window size
const unsigned int SCR_WIDTH = 2560;
const unsigned int SCR_HEIGHT = 1440;

Camera cam(0.0f, 0.0f, 0.0f);
bool firstMouse = true;

//view matrix
glm::mat4 viewmatrix(float x_res, float y_res); 

int main(){
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sandbox APP", glfwGetPrimaryMonitor(), NULL);
    cam.setWindow(window);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const int gridX = 700, gridY = 700;
    cShader main("shaders(comp)/update.comp", gridX, gridY);
    Shader shader("shaders(vf)/v_shader.vert","shaders(vf)/f_shader.frag");

    unsigned int render, data, derivative;

    glGenTextures(1, &render);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, render);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, gridX, gridY, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, render, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &data);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, gridX, gridY, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(1, data, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &derivative);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, derivative);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, gridX, gridY, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(2, derivative, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    cShader initial("shaders(comp)/initial.comp", gridX, gridY);
    initial.use();

    const int grid_size = 700; 
    int vSize = grid_size*grid_size*5;
    int iSize = 6*(grid_size-1)*(grid_size-1);

    float *vertices = new float[grid_size*grid_size*5];
    unsigned int *indices = new unsigned int[6*(grid_size-1)*(grid_size-1)];

    if (!vertices)
    {
        std::cout << "Memory allocation failed\n";
    }

    for(int i = 0; i < grid_size; i ++){
        for(int j = 0; j < grid_size; j ++){
            vertices[5*(i*grid_size + j)+0] = (float)(i-grid_size/2);
            vertices[5*(i*grid_size + j)+1] = 0.0f;
            vertices[5*(i*grid_size + j)+2] = (float)(j-grid_size/2);
            vertices[5*(i*grid_size + j)+3] = ((float)i + 0.5f)/((float)grid_size);
            vertices[5*(i*grid_size + j)+4] = ((float)j + 0.5f)/((float)grid_size);
            //std::cout << vertices[i*grid_size + j] << ", " << vertices[i*grid_size + j + 1] << ", " << vertices[i*grid_size + j +2] << ", " << vertices[i*grid_size + j + 3] << ", " << vertices[i*grid_size + j + 4] << std::endl;
        }
    }

    for(int i = 0; i < grid_size - 1; i ++){
        for(int j = 0; j < grid_size - 1; j ++){
            int x = (i+1)*grid_size + (j+1);
            int y = x - 1;
            indices[6*(i*(grid_size-1) + j) + 0] = x;
            indices[6*(i*(grid_size-1) + j) + 1] = x-1;
            indices[6*(i*(grid_size-1) + j) + 2] = x-grid_size;
            indices[6*(i*(grid_size-1) + j) + 3] = y;
            indices[6*(i*(grid_size-1) + j) + 4] = y - grid_size;
            indices[6*(i*(grid_size-1) + j) + 5] = y - grid_size + 1;
        }
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vSize*sizeof(float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize*sizeof(float), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    delete[] vertices;
    delete[] indices;

    glm::mat4 viewspace = viewmatrix(gridX, gridY);
    //initial.use();
    //initial.setMat4("viewmat", viewspace);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 rot = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);
    //projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(60.0f), float(SCR_WIDTH) / float(SCR_HEIGHT), 0.1f, 2000000.0f);
    scale = glm::scale(scale, glm::vec3(1.0f));
    glm::mat4 mvp;
    glEnable(GL_DEPTH_TEST);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glEnable(GL_PROGRAM_POINT_SIZE);  

    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //viewspace = viewmatrix(gridX, gridY);
        initial.use();
        initial.setFloat("time", (float)glfwGetTime());
        //initial.setMat4("viewmat", viewspace);

        view = cam.updateView();
        mvp = projection * view * scale;

        shader.use();
        shader.setMat4("mvp", mvp);
        shader.setFloat("time", (float)glfwGetTime());
        shader.setInt("fibration", 1);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, grid_size*grid_size);

        shader.use();
        shader.setInt("fibration", 0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, grid_size*grid_size);


        //glDrawElements(GL_TRIANGLES, 6*(grid_size-1)*(grid_size-1), GL_UNSIGNED_INT, 0);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // glBindVertexArray(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    cam.updateDirection(xpos, ypos, firstMouse);
}