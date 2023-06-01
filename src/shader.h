#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

GLchar *LoadShader(const std::string &file);

class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(std::string vertexPath, std::string fragmentPath)
    {
        // 1. retrieve the vertex/fragment source code from filePath
        /*std::string vertexCode;
        std::string fragmentCode;
        std::fstream vShaderFile;
        std::fstream fShaderFile;
        int vShaderLines = 1;
        int fShaderLines = 1;

        //open file tester
        vShaderFile.open(vertexPath, std::ios::in);
        std::string vline;
        while(getline(vShaderFile, vline)){
            vShaderLines ++;
        }
        vShaderFile.close(); 
        fShaderFile.open(fragmentPath, std::ios::in);
        std::string fline;
        while(getline(fShaderFile, fline)){
            fShaderLines ++;
        }
        fShaderFile.close();
        
        //read files
        vShaderFile.open(vertexPath, std::ios::in);
        std::string vline2;
        while(getline(vShaderFile, vline2)){
            vShaderLines --;
            if(vShaderLines > 1){
                vertexCode += vline2 + "\n";
            }
            else{
                vertexCode += vline2 + "\0";    
            }
        }
        vShaderFile.close();

        fShaderFile.open(fragmentPath, std::ios::in);
        std::string fline2;
        while(getline(fShaderFile, fline2)){
            fShaderLines --;
            if(fShaderLines > 1){
                fragmentCode += fline2 + "\n";
            }
            else{
                fragmentCode += fline2 + "\0";    
            }
        }
        fShaderFile.close();

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();*/
        const GLchar *vShaderCode = LoadShader(vertexPath);
        const GLchar *fShaderCode = LoadShader(fragmentPath);
        
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX");
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT");
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
    }
    int getID(){
        return ID;
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    {
        //std::cout << name << std::endl; 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }

    void setVec2(const std::string &name, float value1, float value2) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), value1, value2);
    }

    void setMat4(const std::string &name, glm::mat4 &value) const
    { 
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); 
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- ----------------------------------------------------- " << std::endl;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- ----------------------------------------------------- " << std::endl;
            }
        }
    }
};
#endif