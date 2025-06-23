#pragma once 

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "camera.h"

#include "../extern/perlin/perlin_sse.h"

int speed_index = 0;
std::vector<float> speeds = {50,100,500};
#define CHUNK_LENGTH 16
#define PERLIN_THRESHOLD 160

struct chunkData {
    uint vao = 0;
    uint vbo_pos = 0;
    uint vbo_norm = 0;
    uint vbo_tex = 0;
    int vertexCount = 0;
    glm::ivec3 pos;
    std::vector<float> vertices, normals, textures;

    ~chunkData() {
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            GLuint buffers_to_delete[] = { vbo_pos, vbo_norm, vbo_tex };
            glDeleteBuffers(3, buffers_to_delete);
        }
    }
};

struct IVec3Hash {
    ssize_t operator()(const glm::ivec3& v) const {
        ssize_t h1 = std::hash<int>{}(v.x);
        ssize_t h2 = std::hash<int>{}(v.y);
        ssize_t h3 = std::hash<int>{}(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

namespace shader_helper {
    // Helper function to check for shader compilation/linking errors
    void checkCompileErrors(unsigned int shader, std::string type) {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM") {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        } else {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success) {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
}

namespace render_helper {
    // This function is called whenever the window is resized
    void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        // On a Retina display, width and height will be double the window size.
        // This makes sure the OpenGL viewport is the correct size in pixels.
        glViewport(0, 0, width, height);
    }

    // Process user input (e.g., close window with ESC key)
    void processInput(GLFWwindow* window, float dt, glm::vec3 &cameraPos, glm::vec3 &front, glm::vec3 &up)    // dt = frame time
    {
        const float velocity = speeds[speed_index] * dt;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::vec3(front.x, 0, front.z)) * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::vec3(front.x, 0, front.z)) * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(front, up)) * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(front, up)) * velocity;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraPos += glm::vec3(0.0, velocity, 0.0);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cameraPos -= glm::vec3(0.0, velocity, 0.0);
        if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) 
            speed_index = (speed_index + 1) % speeds.size();
    }
}

namespace generator_helper {

    float len = 1.0/16.0; // textures are 16x16
    float off = 0.5;

    void genTopFace(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Top Face
            x,   y+l, z, 
            x+l, y+l, z, 
            x+l, y+l, z+l, 
            x+l, y+l, z+l, 
            x,   y+l, z+l, 
            x,   y+l, z, 
        };
        std::vector<float> n = {
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f
        };

        std::vector<float> t = {
            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,
        };

        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genBotFace(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Bottom Face
            x,   y, z, 
            x+l, y, z, 
            x+l, y, z+l, 
            x+l, y, z+l, 
            x,   y, z+l, 
            x,   y, z, 
        };

        std::vector<float> n = {
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
        };

        std::vector<float> t = {
            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,
        };
        
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genLeftFace(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Left Face
            x,  y+l, z+l, 
            x,  y+l, z, 
            x,  y,   z, 
            x,  y,   z, 
            x,  y,   z+l, 
            x,  y+l, z+l, 
        };

        std::vector<float> n = {
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
        };

        std::vector<float> t = {
            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,
        };
        
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genRightFace(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Right Face
            x+l, y+l, z+l, 
            x+l, y+l, z, 
            x+l, y,   z, 
            x+l, y,   z, 
            x+l, y,   z+l, 
            x+l, y+l, z+l, 
        };

        std::vector<float> n = {
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
        };

        std::vector<float> t = {
            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,
        };
        
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genFrontFace(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Front Face
            x,   y,   z+l,
            x+l, y,   z+l,
            x+l, y+l, z+l,
            x+l, y+l, z+l,
            x,   y+l, z+l,
            x,   y,   z+l
        };

        std::vector<float> n = {
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
        };

        std::vector<float> t = {
            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,
        };
        
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genBackFace(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
    
        std::vector<float> v = {
            // Back Face
            x,   y,   z,
            x+l, y,   z,
            x+l, y+l, z,
            x+l, y+l, z,
            x,   y+l, z,
            x,   y,   z
        };

        std::vector<float> n = {
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
        };

        std::vector<float> t = {
            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,
        };
        
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void createBlock(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        genTopFace  (x,y,z,l,row,col,vertices,normals,textures);
        genBotFace  (x,y,z,l,row,col,vertices,normals,textures);
        genLeftFace (x,y,z,l,row,col,vertices,normals,textures);
        genRightFace(x,y,z,l,row,col,vertices,normals,textures);
        genFrontFace(x,y,z,l,row,col,vertices,normals,textures);
        genBackFace (x,y,z,l,row,col,vertices,normals,textures);
    }

    void calculate_required_chunks(std::unordered_set<glm::ivec3, IVec3Hash>& current_required_chunks) {

        int camera_chunk_x = static_cast<int>(floor(cameraPos.x / CHUNK_LENGTH));
        int camera_chunk_y = static_cast<int>(floor(cameraPos.y / CHUNK_LENGTH));
        int camera_chunk_z = static_cast<int>(floor(cameraPos.z / CHUNK_LENGTH));

        for (int k=-ceil(RENDER_DISTANCE/2); k <= ceil(RENDER_DISTANCE/2); k++) { // y
            for (int r=0; r <= ceil(RENDER_DISTANCE/2); r++) { // z
                for (int i=0; i <= 2*r; i++)
                    current_required_chunks.insert({camera_chunk_x - r + i, camera_chunk_y + k, camera_chunk_z - r});

                for (int i=0; i <= 2*r; i++)
                    current_required_chunks.insert({camera_chunk_x - r, camera_chunk_y + k, camera_chunk_z - r + i});

                for (int i=0; i < 2*r; i++)
                    current_required_chunks.insert({camera_chunk_x + r, camera_chunk_y + k, camera_chunk_z + r - i});

                for (int i=0; i < 2*r; i++)
                    current_required_chunks.insert({camera_chunk_x + r - i, camera_chunk_y + k, camera_chunk_z + r});
            }
        }
    }

    void calculate_mesh (chunkData& chunk) {

        f32 f = 1.0f / 32.0f; // the smaller the more coarse
        uint8_t* val = (uint8_t*) malloc(4 * sizeof(uint8_t));

        for (int x=0; x < CHUNK_LENGTH; x += 4) {
            for (int y=0; y < CHUNK_LENGTH; y++) {
                for (int z=0; z < CHUNK_LENGTH; z++) {
                    perlinNoiseSIMD_4x((x + chunk.pos.x * CHUNK_LENGTH + 10000) * f, (y + chunk.pos.y * CHUNK_LENGTH + 10000) * f, (z + chunk.pos.z * CHUNK_LENGTH + 10000) * f, f, val);
                    
                    for (int i = 0; i < 4; i++) {
                        // std::cout << "hello: " << static_cast<int>(val[i]) << std::endl;
                        if (val[i] >= PERLIN_THRESHOLD)
                            generator_helper::createBlock(x+i, y, z, 1, chunk.pos.x%32, chunk.pos.z%64, chunk.vertices, chunk.normals, chunk.textures);
                    }
                }
            }
        }

        // for (int i=0; i < CHUNK_LENGTH; i++) {
        //     for (int j=0; j < CHUNK_LENGTH; j++) {
        //         // the translation is done later
        //         generator_helper::createBlock(i, 0, j, 1, chunk.pos.x%32, chunk.pos.z%64, chunk.vertices, chunk.normals, chunk.textures);
        //     }
        // }
    }
}