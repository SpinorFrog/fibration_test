#include <glad/glad.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/*const char *LoadCShader(const std::string &file){
    std::fstream shaderFile;
    unsigned int shaderFileLength;

    shaderFile.open(file);

    if (shaderFile.fail())
    {
        throw std::runtime_error("COULD NOT FIND SHADER FILE");
    }

    shaderFile.seekg(0, shaderFile.end);
    shaderFileLength = shaderFile.tellg();
    //std::cout << shaderFileLength << std::endl;
    shaderFile.seekg(0, shaderFile.beg);

    GLchar *shaderCode = new GLchar[shaderFileLength];
    shaderFile.read(shaderCode, shaderFileLength);

    shaderFile.close();

    shaderCode[shaderFileLength] = '\0';

    //std::cout << shaderCode << std::endl;

	return shaderCode;
    /*std::string computeCode;
        std::fstream cShaderFile;
        int vShaderLines = 1;

        //open file tester
        cShaderFile.open(file, std::ios::in);
        std::string cline;
        while(getline(cShaderFile, cline)){
            vShaderLines ++;
        }
        cShaderFile.close(); 
        
        //read files
        cShaderFile.open(file, std::ios::in);
        std::string cline2;
        while(getline(cShaderFile, cline2)){
            vShaderLines --;
            if(vShaderLines > 1){
                computeCode += cline2 + "\n";
            }
            else{
                computeCode += cline2 + "\0";    
            }
        }
        cShaderFile.close();
    const char *cCode = computeCode.c_str();

    //std::cout <<cShaderCode << std::endl;

    //GLchar *ccode = const_cast<GLchar *>(cShaderCode);

    //std::cout << ccode << std::endl;

    return cCode;
}*/

class cShader
{
public:
    unsigned int ID;
    int wgWidth;
    int wgHeight;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    cShader(const std::string &file, int wid, int hig)
    {
        wgWidth = wid;
        wgHeight = hig;

        //const char *cShaderCode = LoadCShader(computePath);
        std::ifstream shaderFile (file, std::ifstream::binary);
        int shaderFileLength;

        //shaderFile.open(file);

        if (shaderFile.fail())
        {
            throw std::runtime_error("COULD NOT FIND SHADER FILE");
        }

        shaderFile.seekg(0, shaderFile.end);
        shaderFileLength = (int)shaderFile.tellg();
        //std::cout << shaderFileLength << std::endl;
        shaderFile.seekg(0, shaderFile.beg);

        char *shaderCode = new char[shaderFileLength];
        shaderFile.read(shaderCode, shaderFileLength);

        shaderFile.close();

        shaderCode[shaderFileLength] = '\0';

        //std::cout << shaderCode << std::endl;
        
        unsigned int compute = glCreateShader(GL_COMPUTE_SHADER);

        glShaderSource(compute, 1, &shaderCode, NULL);

        glCompileShader(compute);
        checkCompileErrors(compute, file);
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, compute);
        glDeleteShader(compute);
    }

    void link_shader(const std::string &new_shader_path){
        //const GLchar *new_code = LoadCShader(new_shader_path);
        std::fstream shaderFile;
        unsigned int shaderFileLength;

        shaderFile.open(new_shader_path);

        if (shaderFile.fail())
        {
            throw std::runtime_error("COULD NOT FIND SHADER FILE");
        }

        shaderFile.seekg(0, shaderFile.end);
        shaderFileLength = shaderFile.tellg();
        //std::cout << shaderFileLength << std::endl;
        shaderFile.seekg(0, shaderFile.beg);

        GLchar *new_code = new GLchar[shaderFileLength];
        shaderFile.read(new_code, shaderFileLength);

        shaderFile.close();

        new_code[shaderFileLength] = '\0';

        unsigned int new_shader = glCreateShader(GL_COMPUTE_SHADER);

        glShaderSource(new_shader, 1, &new_code, NULL);
        glCompileShader(new_shader);

        checkCompileErrors(new_shader, "NEW CODE");
        glAttachShader(ID, new_shader);
        glDeleteShader(new_shader);
    }

    void compile_shader(){
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
    }

    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
        glDispatchCompute(16, 16, 16);
        // make sure writing to image has finished before read
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
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

    void setVec4(const std::string &name, float value1, float value2, float value3, float value4) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), value1, value2, value3, value4);
    }

    void setMat3(const std::string &name, glm::mat3 &value) const
    { 
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value)); 
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