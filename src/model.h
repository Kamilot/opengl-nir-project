

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "shader.h"
#include "mesh.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <bits/unordered_map.h>


glm::mat4 aiToGlmMatrix(const aiMatrix4x4& from) {
    glm::mat4 to;
    to[0][0] = from.a1;
    to[1][0] = from.a2;
    to[2][0] = from.a3;
    to[3][0] = from.a4;
    to[0][1] = from.b1;
    to[1][1] = from.b2;
    to[2][1] = from.b3;
    to[3][1] = from.b4;
    to[0][2] = from.c1;
    to[1][2] = from.c2;
    to[2][2] = from.c3;
    to[3][2] = from.c4;
    to[0][3] = from.d1;
    to[1][3] = from.d2;
    to[2][3] = from.d3;
    to[3][3] = from.d4;
    return to;
}

glm::vec3 aiToGlmVec3(const aiVector3D& vector) {
    return {vector.x, vector.y, vector.z};
}

glm::vec3 aiToGlmVec3(const aiColor4D& vector) {
    return {vector.r, vector.g, vector.b};
}

glm::vec2 aiToGlmVec2(const aiVector3D& vector) {
    return {vector.x, vector.y};
}

glm::quat aiToGlmQuat(const aiQuaternion& quaternion) {
    return {quaternion.w, quaternion.x, quaternion.y, quaternion.z};
}

class MotionCaptureData {
public:
    MotionCaptureData(const std::string& filename) {
        parseBVH(filename);
    }

    glm::vec3 get_position(const std::string& bone_name, double time) {
        int frame = static_cast<int>(std::floor(time / frame_time));
        const auto& positions_vector = positions[bone_name];
        if (positions_vector.size() == 1) {
            return positions_vector[0];
        }
        frame %= positions_vector.size();
        if (frame == positions_vector.size() - 1) {
            frame = 0;
        }
        float mix_ratio = fmod(time, frame_time) / frame_time;
        glm::vec3 prev_position = positions_vector[frame];
        glm::vec3 next_position = positions_vector[frame + 1];
        return prev_position * (1 - mix_ratio) + next_position * mix_ratio;
    }

    glm::quat get_rotation(const std::string& bone_name, double time) {
        int frame = static_cast<int>(std::floor(time / frame_time));
        const auto& rotations_vector = rotations[bone_name];
        frame %= rotations_vector.size();
        if (frame == rotations_vector.size() - 1) {
            frame = 0;
        }
        float mix_ratio = fmod(time, frame_time) / frame_time;
        glm::quat prev_rotation = rotations_vector[frame];
        glm::quat next_rotation = rotations_vector[frame + 1];
        return glm::slerp(prev_rotation, next_rotation, mix_ratio);
    }

private:
    void parseBVH(const std::string& filename) {
        std::ifstream bvh_file(filename);
        std::string help_string;
        bvh_file >> help_string;
        assert(help_string == "HIERARCHY");
        while (help_string != "MOTION") {
            bvh_file >> help_string;
            if (help_string == "ROOT" || help_string == "JOINT") {
                std::string bone_name;
                bvh_file >> bone_name;
                bone_list.push_back(bone_name);
                bvh_file >> help_string;
                bvh_file >> help_string;
                assert(help_string == "OFFSET");
                float x, y, z;
                bvh_file >> x >> y >> z;
                positions[bone_name].push_back({x * SCALE, y * SCALE, z * SCALE}); // blender coordinates
                int channels;
                bvh_file >> help_string >> channels;
                assert(help_string == "CHANNELS");
                for (int i = 0; i < channels; ++i) {
                    bvh_file >> help_string;
                }
            }
        }
        bvh_file >> help_string >> num_frames_;
        assert(help_string == "Frames:");
        std::cout << "Frames: " << num_frames_ << "\n";
        bvh_file >> help_string >> help_string >> frame_time;
        assert(help_string == "Time:");
        std::cout << "Frame time: " << frame_time << "\n";
        for (int i = 0; i < num_frames_; ++i) {
            for (int j = 0; j < bone_list.size(); ++j) {
                if (j == 0) {
                    float x, y, z;
                    bvh_file >> x >> y >> z;
                    glm::vec3 pos(x * SCALE, y * SCALE, z * SCALE);
                    if (i == 0) {
                        positions[bone_list[j]][0] = pos;
                    } else {
                        positions[bone_list[j]].push_back(pos);
                    }
                    float z_rot, y_rot, x_rot;
                    bvh_file >> z_rot >> y_rot >> x_rot;
                    // std::cout << z_rot << " " << y_rot << " " << x_rot << "\n";
                    glm::mat4 rotation = glm::mat4(1.0f)
                            * glm::rotate(glm::radians(z_rot), glm::vec3(0.0f, -1.0f, 0.0f))
                                         * glm::rotate(glm::radians(y_rot), glm::vec3(0.0f, 0.0f, 1.0f))
                                         * glm::rotate(glm::radians(x_rot), glm::vec3(1.0f, 0.0f, 0.0f));
                    rotations[bone_list[j]].push_back(glm::quat_cast(rotation));
                } else {
                    float z_rot, x_rot, y_rot;
                    bvh_file >> z_rot >> x_rot >> y_rot;
                    glm::mat4 rotation =
                            glm::rotate(glm::radians(z_rot), glm::vec3(0.0f, 0.0f, 1.0f))
                                         * glm::rotate(glm::radians(x_rot), glm::vec3(1.0f, 0.0f, 0.0f))
                                         * glm::rotate(glm::radians(y_rot), glm::vec3(0.0f, 1.0f, 0.0f));
                    rotations[bone_list[j]].push_back(glm::quat_cast(rotation));
                }
            }
        }

        bvh_file.close();
    }

    double SCALE = 0.028;
    std::vector<std::string> bone_list;
    std::unordered_map<std::string, std::vector<glm::vec3>> positions;
    std::unordered_map<std::string, std::vector<glm::quat>> rotations;
    double frame_time;
    int num_frames_;
};

class VertexBoneAttribute {
    const static int MAX_BONES = 4;
public:
    glm::ivec4 bones;
    glm::vec4 weights;

    void AddBone(int bone_id, float weight) {
        int min_weight_bone = 0;
        for (int i = 1; i < MAX_BONES; ++i) {
            if (weights[i] < weights[min_weight_bone]) {
                min_weight_bone = i;
            }
        }
        if (weight > weights[min_weight_bone]) {
            bones[min_weight_bone] = bone_id;
            weights[min_weight_bone] = weight;
        }
    }

    void NormalizeWeights() {
        float total_weight = 0;
        for (int i = 0; i < MAX_BONES; ++i) {
            total_weight += weights[i];
        }
        if (total_weight != 0) {
            for (int i = 0; i < MAX_BONES; ++i) {
                weights[i] /= total_weight;
            }
        }
    }
};

class BonesAttributes : public VertexAttributes {
public:
    BonesAttributes(const std::vector<VertexBoneAttribute>& vertex_bones) :
            vertex_bones_(vertex_bones) {};

    BonesAttributes(std::vector<VertexBoneAttribute>&& vertex_bones) :
            vertex_bones_(std::move(vertex_bones)) {};

    void initAttributes() override {
        glGenBuffers(1, &VBO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertex_bones_.size() * sizeof(VertexBoneAttribute), &vertex_bones_[0],
                     GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // bone ids
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexBoneAttribute), (void*) 0);
        // bone weights
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexBoneAttribute),
                              (void*) offsetof(VertexBoneAttribute, weights));
    }

    void unloadAttributes() override {
        glDeleteBuffers(1, &VBO);
    }

private:
    std::vector<VertexBoneAttribute> vertex_bones_;
    unsigned int VBO;
};

struct AnimationBoneKeyframe {
    glm::vec3 position;
    glm::quat rotation;

    double time;
};

struct Bone {
    // Todo: support different offsets for different meshes.
    std::string name;
    glm::mat4 offset; // From world space to node space in initial position.
    glm::mat4 global_transform; // From node space to world space in current position in animation.
    glm::mat4 default_tranform; // Default node transform.
    glm::mat4 rotation_fix; // For motion capture rotations

    void init() {
        glm::vec3 shift(offset * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
        glm::mat4 base = glm::mat4(glm::mat3(offset));
//        std::cout << glm::to_string(offset) << "\n";
//        std::cout << glm::to_string(base) << "\n";
//        std::cout << glm::to_string(shift) << "\n";
        shift = glm::mat3(glm::inverse(base)) * shift;
        glm::mat4 straight_offset = glm::translate(shift);
//        std::cout << glm::to_string(offset * glm::inverse(straight_offset)) << "\n";
//        glm::mat4 rotation;
        rotation_fix = offset * glm::inverse(straight_offset);
        //rotation_fix * rotation * glm::inverse(rotation_fix)
    }

    void updateGlobalTransform(glm::mat4 parent_transform, double time) {
        if (keyframes_.size() == 0) {
            global_transform = parent_transform * default_tranform;
            return;
        }
        double animation_start = keyframes_[0].time;
        double animation_length = keyframes_.back().time - animation_start;
        double animation_time = fmod(time, animation_length) + animation_start;
        while (animation_time < keyframes_[current_keyframe_].time ||
               animation_time >= keyframes_[current_keyframe_ + 1].time) {
            current_keyframe_ += 1;
            if (current_keyframe_ == keyframes_.size() - 1) {
                current_keyframe_ = 0;
            }
        }
        const AnimationBoneKeyframe& previous_keyframe = keyframes_[current_keyframe_];
        const AnimationBoneKeyframe& next_keyframe = keyframes_[current_keyframe_ + 1];
        float mix_ratio = static_cast<float>((animation_time - previous_keyframe.time) /
                                             (next_keyframe.time - previous_keyframe.time));

        glm::vec3 position = (1 - mix_ratio) * previous_keyframe.position + mix_ratio * next_keyframe.position;
        glm::quat rotation = glm::slerp(previous_keyframe.rotation, next_keyframe.rotation, mix_ratio);
        global_transform =
                parent_transform * glm::translate(glm::mat4(1.0), position) * glm::mat4_cast(rotation);
    }

    void updateGlobalTranformFromMotionCapture(glm::mat4 parent_transform, double time, MotionCaptureData* data) {
        glm::vec3 position = keyframes_[0].position;// data->get_position(name, time);
        glm::mat4 rotation;
        if (false) {//name != "hip" && name != "lThigh" && name != "lShin") {
            rotation = glm::mat4_cast(keyframes_[0].rotation);
        } else {
            rotation =  rotation_fix * glm::mat4_cast(data->get_rotation(name, time)) * glm::inverse(rotation_fix);
        }
        global_transform =
                parent_transform * glm::translate(glm::mat4(1.0), position) * rotation;
    }

    void addKeyframe(const AnimationBoneKeyframe& keyframe) {
        keyframes_.push_back(keyframe);
    }

private:
    std::vector<AnimationBoneKeyframe> keyframes_;
    unsigned int current_keyframe_; // Last rendered keyframe for quick lookup.
};

struct SkeletonNode {
    int bone_index;
    glm::mat4 node_transform; // For non-bone skeleton nodes
    std::vector<std::unique_ptr<SkeletonNode>> children;
};

class AnimatedModel {
    const int BONE_NOT_FOUND = -1;
public:
    AnimatedModel(const std::string& path, MotionCaptureData* motion_capture_data) {
        loadModel(path);
        motion_capture_data_ = motion_capture_data;
    }

    bool walkNodes(const aiNode* node, int offset) {
        if (true) {
            for (int i = 0; i < offset; ++i) {
                std::cout << "|";
            }
            std::cout << node->mName.data << " " << node->mNumChildren << " " << node->mNumMeshes << "\n";
            // std::cout << glm::to_string(aiToGlmMatrix(node->mTransformation)) << "\n";
        }
        for (int i = 0; i < node->mNumChildren; ++i) {
            walkNodes(node->mChildren[i], offset + 1);
        }
        return true;
    }

    void debugPrintout() {
        walkNodes(scene->mRootNode, 0);

        int mat_index = 0;
        aiString testString;
        scene->mMaterials[mat_index]->Get(AI_MATKEY_NAME, testString);
        std::cout << testString.data << std::endl;
        std::cout << scene->mMaterials[mat_index]->GetTextureCount(aiTextureType_DIFFUSE) << std::endl;
        if (scene->mMaterials[mat_index]->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), testString) == AI_SUCCESS)
            std::cout << testString.data << std::endl;
        else
            std::cout << "FAIL" << std::endl;

        aiColor3D testColor;
        if (scene->mMaterials[mat_index]->Get(AI_MATKEY_COLOR_DIFFUSE, testColor) == AI_SUCCESS)
            std::cout << testColor.r << " " << testColor.g << " " << testColor.b << std::endl;
        else
            std::cout << "FAIL" << std::endl;

        for (int i = 0; i < scene->mNumMaterials; ++i) {
            for (int j = 0; j < scene->mMaterials[i]->mNumProperties; ++j) {
                std::cout << i << ' ' << j << " " << scene->mMaterials[i]->mProperties[j]->mKey.data << "\n";
            }
        }

        std::cout << bones_.size() << " bones\n";
        for (auto& bone : bones_) {
            std::cout << bone.name << "\n";
            std::cout << glm::to_string(bone.rotation_fix) << "\n";
        }

        /*scene->mMaterials[1]->Get(AI_MATKEY_TEXBLEND(aiTextureType_DIFFUSE, 1), blend_power);
        std::cout << blend_power << std::endl;

        scene->mMaterials[1]->Get(AI_MATKEY_TEXBLEND(aiTextureType_DIFFUSE, 2), blend_power);
        std::cout << blend_power << std::endl;

        scene->mMaterials[1]->Get(AI_MATKEY_TEXBLEND(aiTextureType_DIFFUSE, 3), blend_power);
        std::cout << blend_power << std::endl;*/
    }

    void draw(ShaderProgram shader, double time) {
        std::vector<glm::mat4> final_transforms(bones_.size());
        calculateBoneTransforms(skeleton_.get(), time, final_transforms, glm::mat4(1.0f));
        shader.setMat4v("jointTransforms", final_transforms);

        for (const auto& mesh: meshes_) {
            mesh->draw(shader);
        }
    }

private:
    void calculateBoneTransforms(SkeletonNode* node, double time, std::vector<glm::mat4>& final_tranforms,
                                 glm::mat4 parent_transform) {
        glm::mat4 next_parent_transform = parent_transform;
        if (node->bone_index != BONE_NOT_FOUND) {
            Bone& bone = bones_[node->bone_index];
            bone.updateGlobalTransform(parent_transform, time);
            // bone.updateGlobalTranformFromMotionCapture(parent_transform, time, motion_capture_data_);
            final_tranforms[node->bone_index] = bone.global_transform * bone.offset;
            next_parent_transform = bone.global_transform;
        } else {
            next_parent_transform = parent_transform * node->node_transform;
        }

        for (const auto& child : node->children) {
            calculateBoneTransforms(child.get(), time, final_tranforms, next_parent_transform);
        }
    }

    int getBoneId(const std::string& node_name, bool create_bone = true) {
        // Armature name strip.
        size_t pos = node_name.rfind("_");
        std::string bone_name;
        if (pos == std::string::npos) {
            bone_name = node_name;
        } else {
            bone_name = node_name.substr(pos + 1, node_name.size() - pos - 1);
        }

        auto it = bone_to_idx_.find(bone_name);
        int bone_index;
        if (it != bone_to_idx_.end()) {
            bone_index = it->second;
        } else {
            if (create_bone) {
                bone_index = bones_.size();
                bone_to_idx_[bone_name] = bone_index;
                bones_.emplace_back();
                bones_.back().name = bone_name;
            } else {
                return BONE_NOT_FOUND;
            }
        }
        return bone_index;
    }

    // Returns true if there are nodes correlating to bones in subtree.
    bool buildSkeleton(const aiNode* ai_node, SkeletonNode* skeleton_node) {
        int bone_id = getBoneId(ai_node->mName.data, false);
        skeleton_node->bone_index = bone_id;
        skeleton_node->node_transform = aiToGlmMatrix(ai_node->mTransformation);
        int num_children = ai_node->mNumChildren;
        bool bone_node = false;
        if (bone_id != BONE_NOT_FOUND) {
            bone_node = true;
            bones_[bone_id].default_tranform = skeleton_node->node_transform;
        }

        for (int i = 0; i < num_children; ++i) {
            SkeletonNode* new_node = new SkeletonNode();
            bool bones_in_subtree = buildSkeleton(ai_node->mChildren[i], new_node);
            if (bones_in_subtree) {
                skeleton_node->children.emplace_back(new_node);
                bone_node = true;
            } else {
                delete new_node;
            }
        }
        return bone_node;
    }

    void loadModel(const std::string& path) {
//        Assimp::Importer importer;
//        const aiScene*
        scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                        aiProcess_FlipUVs);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // Todo: remove
        std::ofstream temp_file("temp.txt");
        int num_bones = 0;
        for (int mesh_index = 0; mesh_index < scene->mNumMeshes; ++mesh_index) {
            const aiMesh* mesh = scene->mMeshes[mesh_index];
            int num_vertices = mesh->mNumVertices;

            std::vector<Vertex> vertices(num_vertices);
            std::vector<VertexBoneAttribute> bone_data(num_vertices);
            std::vector<unsigned int> indices;

            if (!scene->mMeshes[mesh_index]->HasTextureCoords(0))
                std::cout << "Mesh " << mesh_index << " has no texture coordinates" << std::endl;

            for (int vertex_id = 0; vertex_id < num_vertices; ++vertex_id) {
                vertices[vertex_id].position = aiToGlmVec3(mesh->mVertices[vertex_id]);
                vertices[vertex_id].normal = aiToGlmVec3(mesh->mNormals[vertex_id]);

                // Todo: Add support for multiple texture coordinates.
                if (scene->mMeshes[mesh_index]->HasTextureCoords(0))
                    vertices[vertex_id].tex_coords = aiToGlmVec2(mesh->mTextureCoords[0][vertex_id]);
            }

            for (int face_id = 0; face_id < mesh->mNumFaces; ++face_id) {
                if (mesh->mFaces[face_id].mNumIndices != 3) {
                    std::cout << "Ignoring non-triangle face\n";
                    continue;
                }
                for (int i = 0; i < 3; ++i) {
                    indices.push_back(mesh->mFaces[face_id].mIndices[i]);
                }
            }

            // Load bones and bone weights for vertices.
            int mesh_bones = mesh->mNumBones;
            for (int i = 0; i < mesh_bones; ++i) {
                const aiBone* bone = mesh->mBones[i];
                int bone_index = getBoneId(bone->mName.data);
                // Todo: update to have different offsets for different meshes.
                bones_[bone_index].offset = aiToGlmMatrix(bone->mOffsetMatrix);
                bones_[bone_index].init();
                for (int j = 0; j < bone->mNumWeights; ++j) {
                    int vertex_id = bone->mWeights[j].mVertexId;
                    bone_data[vertex_id].AddBone(bone_index, bone->mWeights[j].mWeight);
                }
            }

            // Normalize bone weights to sum up to 1
            for (int i = 0; i < num_vertices; ++i) {
                bone_data[i].NormalizeWeights();
            }

            std::string dir = path.substr(0, path.rfind('/') + 1);

            Material* material;
            const aiMaterial* ai_material = scene->mMaterials[mesh->mMaterialIndex];
            aiString texture_path;
            if (ai_material->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texture_path) == AI_SUCCESS) {
                // Todo: import specular and shininess
                material = new DiffuseMapMaterial(dir + "textures/" + std::string(texture_path.data),
                                                  glm::vec3(1.0f, 1.0f, 1.0f),
                                                  32.0f);
            } else {
                aiColor4D ai_color;
                ai_material->Get(AI_MATKEY_COLOR_DIFFUSE, ai_color);
                glm::vec3 color = aiToGlmVec3(ai_color);
                material = new DiffuseMapMaterial(color, glm::vec3(1.0f, 1.0f, 1.0f), 32.0f);
            }

            meshes_.emplace_back(new Mesh({new PositionalAttributes(std::move(vertices)),
                                           new BonesAttributes(std::move(bone_data))},
                                          indices,
                                          material));
        }


        global_inverse_transform_ = glm::inverse(aiToGlmMatrix(scene->mRootNode->mTransformation));

        // Todo: support several animations.
        const aiAnimation* animation = scene->mAnimations[0];
        for (int i = 0; i < animation->mNumChannels; ++i) {
            const aiNodeAnim* channel = animation->mChannels[i];
            int bone_id = getBoneId(channel->mNodeName.data);
            assert(bone_id != BONE_NOT_FOUND);
            assert(channel->mNumPositionKeys == channel->mNumRotationKeys);
            for (int j = 0; j < channel->mNumPositionKeys; ++j) {
                bones_[bone_id].addKeyframe({aiToGlmVec3(channel->mPositionKeys[j].mValue),
                                             aiToGlmQuat(channel->mRotationKeys[j].mValue),
                                             channel->mPositionKeys[j].mTime});
            }
        }
        // Todo: remove
        temp_file.close();
        skeleton_.reset(new SkeletonNode);
        assert(buildSkeleton(scene->mRootNode, skeleton_.get()) && "No bone structure information found");
    }


    std::vector<std::unique_ptr<Mesh>> meshes_;
    glm::mat4 global_inverse_transform_;
    std::unordered_map<std::string, int> bone_to_idx_;
    std::vector<Bone> bones_;
    std::unique_ptr<SkeletonNode> skeleton_;
    MotionCaptureData* motion_capture_data_;

    // Todo: move back into init function. Exposed for testing purposes
    const aiScene* scene;
    Assimp::Importer importer;
};

#endif


