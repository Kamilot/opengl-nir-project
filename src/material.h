//
// Created by kamilot on 28.04.18.
//

#ifndef FIRST_TRY_MATERIAL_H
#define FIRST_TRY_MATERIAL_H

#include <glad/glad.h>
#include <stb_image.h>
#include "shader.h"
#include <glm/glm.hpp>

#include <string>

unsigned int loadTexture(const std::string& path) {
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture " << path << std::endl;
    }
    stbi_image_free(data);
    return texture_id;
}

unsigned int createSingleColorTexture(const glm::vec3& color) {
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, glm::value_ptr(color));
    return texture_id;
}

class Material {
public:
    virtual void load(ShaderProgram shaderProgram) = 0;
};

class ColorMaterial: public Material {
public:
    ColorMaterial(glm::vec3 diffuse_color, glm::vec3 specular_color, float shininess)
            : diffuse_color_(diffuse_color), specular_color_(specular_color), shininess_(shininess) {}

    void load(ShaderProgram shaderProgram) override {
        shaderProgram.setVec3("material.diffuse", diffuse_color_);
        shaderProgram.setVec3("material.specular", specular_color_);
        shaderProgram.setFloat("material.shininess", shininess_);
    }

private:
    glm::vec3 diffuse_color_;
    glm::vec3 specular_color_;
    float shininess_;
};

class DiffuseMapMaterial: public Material {
public:
    DiffuseMapMaterial(const std::string& diffuse_texture_path, glm::vec3 specular_color, float shininess)
            :specular_color_(specular_color), shininess_(shininess) {
        diffuse_texture_ = loadTexture(diffuse_texture_path);
    }

    DiffuseMapMaterial(const glm::vec3& diffuse_color, glm::vec3 specular_color, float shininess)
            :specular_color_(specular_color), shininess_(shininess) {
        diffuse_texture_ = createSingleColorTexture(diffuse_color);
    }

    void load(ShaderProgram shaderProgram) override {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_texture_);
        shaderProgram.setInt("material.diffuse", 0);
        shaderProgram.setVec3("material.specular", specular_color_);
        shaderProgram.setFloat("material.shininess", shininess_);
    }

private:
    unsigned int diffuse_texture_;
    glm::vec3 specular_color_;
    float shininess_;
};

class DiffuseSpecularMapMaterial: public Material {
public:
    DiffuseSpecularMapMaterial(std::string diffuse_texture_path, std::string specular_texture_path, float shininess)
            :shininess_(shininess) {
        diffuse_texture_ = loadTexture(diffuse_texture_path);
        specular_texture_ = loadTexture(specular_texture_path);
    }

    void load(ShaderProgram shaderProgram) override {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuse_texture_);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specular_texture_);
        shaderProgram.setInt("material.diffuse", 0);
        shaderProgram.setInt("material.specular", 1);
        shaderProgram.setFloat("material.shininess", shininess_);
    }

private:
    unsigned int diffuse_texture_;
    unsigned int specular_texture_;
    float shininess_;
};

#endif //FIRST_TRY_MATERIAL_H
