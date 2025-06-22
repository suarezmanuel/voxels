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
#include "camera.h"

int speed_index = 0;
std::vector<float> speeds = {1,10,50};

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
            cameraPos += glm::vec3(front.x, 0, front.z) * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= glm::vec3(front.x, 0, front.z) * velocity;
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

    void createBlock(float x, float y, float z, float l, int row, int col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {

        std::vector<float> v = {
            // Back Face
            x,   y,   z,
            x+l, y,   z,
            x+l, y+l, z,
            x+l, y+l, z,
            x,   y+l, z,
            x,   y,   z,

            // Front Face
            x,   y,   z+l,
            x+l, y,   z+l,
            x+l, y+l, z+l,
            x+l, y+l, z+l,
            x,   y+l, z+l,
            x,   y,   z+l,

            // Left Face
            x,  y+l, z+l, 
            x,  y+l, z, 
            x,  y,   z, 
            x,  y,   z, 
            x,  y,   z+l, 
            x,  y+l, z+l, 

            // Right Face
            x+l, y+l, z+l, 
            x+l, y+l, z, 
            x+l, y,   z, 
            x+l, y,   z, 
            x+l, y,   z+l, 
            x+l, y+l, z+l, 

            // Bottom Face
            x,   y, z, 
            x+l, y, z, 
            x+l, y, z+l, 
            x+l, y, z+l, 
            x,   y, z+l, 
            x,   y, z, 

            // Top Face
            x,   y+l, z, 
            x+l, y+l, z, 
            x+l, y+l, z+l, 
            x+l, y+l, z+l, 
            x,   y+l, z+l, 
            x,   y+l, z, 
        };

        std::vector<float> n = {
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,
            0.0f,  0.0f, -1.0f,

            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,

           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,

            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,

            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,
            0.0f, -1.0f,  0.0f,

            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f
        };

        float len = 1.0/16.0; // textures are 16x16
        float off = 0.5;

        std::vector<float> t = {

            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,

            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,

            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,

            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,

            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,

            col*len, row*len,
            (col+off/2)*len, (row)*len,
            (col+off/2)*len, (row+off)*len,
            (col+off/2)*len, (row+off)*len,
            (col+0.0f)*len, (row+off)*len,
            (col)*len, (row)*len,

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
}