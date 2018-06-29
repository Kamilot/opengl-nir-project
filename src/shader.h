//
// Created by kamilot on 10.01.18.
//

#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>

std::string ReadFile(const std::string& filename) {
    std::ifstream infile(filename);
    infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    return { std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
}

class ShaderProgram {
private:
    unsigned int CompileShader(const std::string& filename, unsigned int shaderType) {
        unsigned int shader;
        shader = glCreateShader(shaderType);
        std::string shaderFile = ReadFile(filename);
        const char *shaderFilePtr = shaderFile.c_str();
        glShaderSource(shader, 1, &shaderFilePtr, NULL);
        glCompileShader(shader);

        int success;
        char infoLog[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            success_ = false;
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << filename << "\n"<< infoLog << std::endl;
        }
        return shader;
    }

public:
    ShaderProgram(const std::string& vertexFilepath, const std::string& fragmentFilepath) {
        success_ = true;
        unsigned int vertexShaderId;
        try {
            vertexShaderId = CompileShader(vertexFilepath, GL_VERTEX_SHADER);
        } catch (std::ifstream::failure e) {
            std::cout << "ERROR::VERTEX_SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
            success_ = false;
        }
        unsigned int fragmentShaderId;
        try {
            fragmentShaderId = CompileShader(fragmentFilepath, GL_FRAGMENT_SHADER);
        } catch (std::ifstream::failure e) {
            std::cout << "ERROR::FRAGMENT_SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
            success_ = false;
        }
        if (success_) {
            id_ = glCreateProgram();
            glAttachShader(id_, vertexShaderId);
            glAttachShader(id_, fragmentShaderId);
            glLinkProgram(id_);
            int success;
            char infoLog[512];
            glGetProgramiv(id_, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(id_, 512, NULL, infoLog);
                success_ = false;
                std::cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            }
            glDeleteShader(vertexShaderId);
            glDeleteShader(fragmentShaderId);
        }
    }

    ShaderProgram(unsigned int vertexShaderId, unsigned int fragmentShaderId) {
        id_ = glCreateProgram();
        glAttachShader(id_, vertexShaderId);
        glAttachShader(id_, fragmentShaderId);
        glLinkProgram(id_);
        int success;
        char infoLog[512];
        glGetProgramiv(id_, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(id_, 512, NULL, infoLog);
            success_ = false;
            std::cout << "ERROR::SHADER_PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }

    void use() {
        glUseProgram(id_);
    }

    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(id_, name.c_str()), (int)value);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(id_, name.c_str()), value);
    }

    void setInt(const std::string &name, std::vector<int> values) const {
        if (values.size() == 1) {
            glUniform1iv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        } else if (values.size() == 2) {
            glUniform2iv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        } else if (values.size() == 3) {
            glUniform3iv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        } else if (values.size() == 4) {
            glUniform4iv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        }
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(id_, name.c_str()), value);
    }

    void setFloatVector(const std::string &name, const std::vector<float>& values) const {
        if (values.size() == 1) {
            glUniform1fv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        } else if (values.size() == 2) {
            glUniform2fv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        } else if (values.size() == 3) {
            glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        } else if (values.size() == 4) {
            glUniform4fv(glGetUniformLocation(id_, name.c_str()), 1, &values[0]);
        }
    }

    void setVec3(const std::string &name, const glm::vec3& value) const {
        glUniform3fv(glGetUniformLocation(id_, name.c_str()), 1, glm::value_ptr(value));
    }

    void setMat4(const std::string &name, glm::mat4 value) {
        glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

    void setMat4v(const std::string &name, std::vector<glm::mat4> value) {
        glUniformMatrix4fv(glGetUniformLocation(id_, name.c_str()), value.size(), GL_FALSE, (float*)(&value[0]));
    }

    void setMat3(const std::string &name, glm::mat3 value) {
        glUniformMatrix3fv(glGetUniformLocation(id_, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
    }

private:
    unsigned int id_;
    bool success_;
};



#endif //SHADER_H
