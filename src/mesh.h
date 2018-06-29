//
// Created by kamilot on 27.04.18.
//

#ifndef FIRST_TRY_MESH_H
#define FIRST_TRY_MESH_H

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "material.h"

#include <string>
#include <vector>
#include <memory>

using std::vector;
using std::string;

class VertexAttributes {
public:
    virtual void initAttributes() = 0;
    virtual void unloadAttributes() = 0;
};

struct Vertex {
    // position
    glm::vec3 position;
    // normal
    glm::vec3 normal;
    // texCoords
    glm::vec2 tex_coords;
};

class PositionalAttributes: public VertexAttributes {
public:
    PositionalAttributes(const vector<Vertex> &vertices):
            vertices_(vertices) {}

    PositionalAttributes(vector<Vertex> &&vertices):
            vertices_(std::move(vertices)) {}

    void initAttributes() override {
        glGenBuffers(1, &VBO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(Vertex), &vertices_[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    }

    void unloadAttributes() override {
        glDeleteBuffers(1, &VBO);
    }

private:
    std::vector<Vertex> vertices_;
    unsigned int VBO;
};

// Maybe add template argument for Vertex later
class Mesh {
    void initMesh() {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], GL_STATIC_DRAW);

        for (const auto& attribute: attributes_) {
            attribute->initAttributes();
        }

        glBindVertexArray(0);
    }

public:
    Mesh(const std::vector<VertexAttributes*>& attributes, const std::vector<unsigned int>& indices, Material* material):
        indices_(indices), material_(material) {
        attributes_.reserve(attributes.size());
        for (auto attribute: attributes) {
            attributes_.emplace_back(attribute);
        }
        initMesh();
    }

    // render the mesh
    void draw(ShaderProgram shader)
    {
        material_->load(shader);
        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

    ~Mesh() {

        for (const auto& attribute : attributes_) {
            attribute->unloadAttributes();
        }
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &EBO);
    }

private:
    unsigned int VAO, EBO;
    std::vector<std::unique_ptr<VertexAttributes>> attributes_;
    std::vector<unsigned int> indices_;
    std::unique_ptr<Material> material_;
};

Mesh* createCube(float size) {
    std::vector<Vertex> vertices;
    size /= 2;
    Vertex new_vertex;
    // Face 1 of 6
    new_vertex.normal = {-1.0f, 0.0f, 0.0f};

    new_vertex.position = {-size, -size, -size};
    new_vertex.tex_coords = {1.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, -size, size};
    new_vertex.tex_coords = {1.0f, 1.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, size, -size};
    new_vertex.tex_coords = {0.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, size, size};
    new_vertex.tex_coords = {0.0f, 1.0f};
    vertices.push_back(new_vertex);

    // Face 2 of 6
    new_vertex.normal = {0.0f, -1.0f, 0.0f};

    new_vertex.position = {size, -size, -size};
    new_vertex.tex_coords = {1.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, -size, size};
    new_vertex.tex_coords = {1.0f, 1.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, -size, -size};
    new_vertex.tex_coords = {0.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, -size, size};
    new_vertex.tex_coords = {0.0f, 1.0f};
    vertices.push_back(new_vertex);

    // Face 3 of 6
    new_vertex.normal = {1.0f, 0.0f, 0.0f};

    new_vertex.position = {size, size, -size};
    new_vertex.tex_coords = {1.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, size, size};
    new_vertex.tex_coords = {1.0f, 1.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, -size, -size};
    new_vertex.tex_coords = {0.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, -size, size};
    new_vertex.tex_coords = {0.0f, 1.0f};
    vertices.push_back(new_vertex);

    // Face 4 of 6
    new_vertex.normal = {0.0f, 1.0f, 0.0f};

    new_vertex.position = {-size, size, -size};
    new_vertex.tex_coords = {1.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, size, size};
    new_vertex.tex_coords = {1.0f, 1.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, size, -size};
    new_vertex.tex_coords = {0.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, size, size};
    new_vertex.tex_coords = {0.0f, 1.0f};
    vertices.push_back(new_vertex);

    // Face 5 of 6
    new_vertex.normal = {0.0f, 0.0f, 1.0f};

    new_vertex.position = {-size, -size, size};
    new_vertex.tex_coords = {1.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, -size, size};
    new_vertex.tex_coords = {1.0f, 1.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, size, size};
    new_vertex.tex_coords = {0.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, size, size};
    new_vertex.tex_coords = {0.0f, 1.0f};
    vertices.push_back(new_vertex);

    // Face 6 of 6
    new_vertex.normal = {0.0f, 0.0f, -1.0f};

    new_vertex.position = {size, -size, -size};
    new_vertex.tex_coords = {1.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, -size, -size};
    new_vertex.tex_coords = {1.0f, 1.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {size, size, -size};
    new_vertex.tex_coords = {0.0f, 0.0f};
    vertices.push_back(new_vertex);

    new_vertex.position = {-size, size, -size};
    new_vertex.tex_coords = {0.0f, 1.0f};
    vertices.push_back(new_vertex);

    std::vector<unsigned int> indices(36);
    for (unsigned int i = 0; 6 * i < indices.size(); ++i) {
        indices[i * 6] = i * 4;
        indices[i * 6 + 1] = i * 4 + 1;
        indices[i * 6 + 2] = i * 4 + 2;
        indices[i * 6 + 3] = i * 4 + 2;
        indices[i * 6 + 4] = i * 4 + 1;
        indices[i * 6 + 5] = i * 4 + 3;
    }

    return new Mesh({new PositionalAttributes(vertices)},
                     indices,
                     new DiffuseSpecularMapMaterial("resources/textures/container2.png",
                                                    "resources/textures/container2_specular.png",
                                                    32.0f));
    // return new Mesh({new PositionalAttributes(vertices)}, indices,
    //         new DiffuseMapMaterial("resources/textures/container2.png", {1.0f, 1.0f, 1.0f}, 32.0f));
    // return new Mesh({new PositionalAttributes(vertices)}, indices,
    //         new ColorMaterial({1.0f, 0.5f, 0.31f}, {1.0f, 1.0f, 1.0f}, 32.0f));
}


#endif //FIRST_TRY_MESH_H
