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
const unsigned int SCR_WIDTH = 3440;
const unsigned int SCR_HEIGHT = 1440;

Camera cam(0.0f, 20.0f, -32.0f);
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

    const int gridX = 128, gridY = 128, gridZ = 128;
    //cShader main("shaders(comp)/update.comp", gridX, gridY);
    //cShader sim("comp_shaders/evolution.glsl", gridX, gridY);
    Shader shader("shaders(vf)/v_shader.vert","shaders(vf)/f_shader.frag");

    unsigned int main_grid, second_grid, new_main_grid, new_second_grid, main_derivative, secondary_derivatives;

    glGenTextures(1, &main_grid);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, main_grid);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridX*4, gridY, gridZ, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, main_grid, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &second_grid);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, second_grid);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridX*3, gridY, gridZ, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(1, second_grid, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &new_main_grid);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, new_main_grid);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridX*4, gridY, gridZ, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(2, new_main_grid, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &new_second_grid);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_3D, new_second_grid);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridX*3, gridY, gridZ, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(3, new_second_grid, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    //3D textures for derivatives
    glGenTextures(1, &main_derivative);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_3D, main_derivative);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridX*4, gridY*3, gridZ, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(4, main_derivative, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glGenTextures(1, &secondary_derivatives);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_3D, secondary_derivatives);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridX*3, gridY*3, gridZ, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(5, secondary_derivatives, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    cShader simulation("comp_shaders/evolution.glsl", gridX, gridY);
    simulation.compile_shader();

    cShader initial("comp_shaders/initial.comp", gridX, gridY);
    initial.compile_shader();

    cShader derivative("comp_shaders/derivative.comp", gridX, gridY);
    derivative.compile_shader();

    initial.use();
    derivative.use();

    const int grid_size = 128; 
    int vSize = grid_size*grid_size*grid_size*6;

    float *vertices = new float[vSize];

    if (!vertices)
    {
        std::cout << "Memory allocation failed\n";
    }

    for(int i = 0; i < grid_size; i ++){
        for(int j = 0; j < grid_size; j ++){
            for(int k = 0; k < grid_size; k ++){
                vertices[6*(i*grid_size*grid_size + j*grid_size + k)+0] = 32.0f*((float)i-grid_size/2)/((float)grid_size);
                vertices[6*(i*grid_size*grid_size + j*grid_size + k)+1] = 32.0f*((float)j-grid_size/2)/((float)grid_size);
                vertices[6*(i*grid_size*grid_size + j*grid_size + k)+2] = 32.0f*((float)k-grid_size/2)/((float)grid_size);
                vertices[6*(i*grid_size*grid_size + j*grid_size + k)+3] = 4*i;
                vertices[6*(i*grid_size*grid_size + j*grid_size + k)+4] = j;
                vertices[6*(i*grid_size*grid_size + j*grid_size + k)+5] = k;
            }
        }
    }

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vSize*sizeof(float), vertices, GL_STATIC_DRAW);

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize*sizeof(float), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    delete[] vertices;

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


        simulation.use();
        derivative.use();

        glCopyImageSubData(new_main_grid, GL_TEXTURE_3D, 0, 0, 0, 0,
                   main_grid, GL_TEXTURE_3D, 0, 0, 0, 0,
                   128*4, 128, 128);

        //viewspace = viewmatrix(gridX, gridY);
        //simulation.use();
        //simulation.setFloat("time", (float)glfwGetTime());
        //initial.setMat4("viewmat", viewspace);

        view = cam.updateView();
        mvp = projection * view * scale;

        shader.use();
        shader.setMat4("mvp", mvp);
        //shader.setFloat("time", (float)glfwGetTime());
        //shader.setInt("fibration", 1);

        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, grid_size*grid_size*grid_size);

        /*shader.use();
        shader.setInt("fibration", 0);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, grid_size*grid_size);*/


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