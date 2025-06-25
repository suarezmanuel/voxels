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
#define CHUNK_LENGTH 32
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

    float len_x = 1.0/64.0; // textures are 16x16
    float len_y = 1.0/32.0; // textures are 16x16
    float off = 0.5;

    void genTopFaceSexy(float x, float y, float z, float lenx, float leny, float lenz, int tex_row, int tex_col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Top Face
            x,      y+leny, z, 
            x+lenx, y+leny, z, 
            x+lenx, y+leny, z+lenz, 
            x+lenx, y+leny, z+lenz, 
            x,      y+leny, z+lenz, 
            x,      y+leny, z, 
        };
        std::vector<float> n = {
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f,
            0.0f,  1.0f,  0.0f
        };

        float u0 = tex_col * len_x; // Left edge (u)
        float v0 = tex_row * len_y; // Bottom edge (v)
        float u1 = u0 + len_x;      // Right edge (u)
        float v1 = v0 + len_y;      // Top edge (v)

        std::vector<float> t = {
            // Tri 1
            u1, v0, // Corresponds to vertex 1 (bottom-left)
            u0, v0, // Corresponds to vertex 2 (bottom-right)
            u0, v1, // Corresponds to vertex 3 (top-right)
            // Tri 2
            u0, v1, // Corresponds to vertex 4 (top-right)
            u1, v1, // Corresponds to vertex 5 (top-left)
            u1, v0, // Corresponds to vertex 6 (bottom-left)
        };


        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genBotFaceSexy(float x, float y, float z, float lenx, float leny, float lenz, int tex_row, int tex_col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Bottom Face
            x,   y, z, 
            x+lenx, y, z, 
            x+lenx, y, z+lenz, 
            x+lenx, y, z+lenz, 
            x,   y, z+lenz, 
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

        float u0 = tex_col * len_x; // Left edge (u)
        float v0 = tex_row * len_y; // Bottom edge (v)
        float u1 = u0 + len_x;      // Right edge (u)
        float v1 = v0 + len_y;      // Top edge (v)

        std::vector<float> t = {
            // Tri 1
            u0, v0, // Corresponds to vertex 1 (bottom-left)
            u1, v0, // Corresponds to vertex 2 (bottom-right)
            u1, v1, // Corresponds to vertex 3 (top-right)
            // Tri 2
            u1, v1, // Corresponds to vertex 4 (top-right)
            u0, v1, // Corresponds to vertex 5 (top-left)
            u0, v0, // Corresponds to vertex 6 (bottom-left)
        };

        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genLeftFaceSexy(float x, float y, float z, float lenx, float leny, float lenz, int tex_row, int tex_col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Left Face
            x,  y+leny, z+lenz, 
            x,  y+leny, z, 
            x,  y,   z, 
            x,  y,   z, 
            x,  y,   z+lenz, 
            x,  y+leny, z+lenz, 
        };

        std::vector<float> n = {
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
           -1.0f,  0.0f,  0.0f,
        };

        float u0 = tex_col * len_x; // Left edge (u)
        float v0 = tex_row * len_y; // Bottom edge (v)
        float u1 = u0 + len_x;      // Right edge (u)
        float v1 = v0 + len_y;      // Top edge (v)

        std::vector<float> t = {
            // Tri 1
            u0, v1, // Corresponds to vertex 1 (bottom-left)
            u1, v1, // Corresponds to vertex 2 (bottom-right)
            u1, v0, // Corresponds to vertex 3 (top-right)
            // Tri 2
            u1, v0, // Corresponds to vertex 4 (top-right)
            u0, v0, // Corresponds to vertex 5 (top-left)
            u0, v1, // Corresponds to vertex 6 (bottom-left)
        };

        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genRightFaceSexy(float x, float y, float z, float lenx, float leny, float lenz, int tex_row, int tex_col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Right Face
            x+lenx, y+leny, z+lenz, 
            x+lenx, y+leny, z, 
            x+lenx, y,   z, 
            x+lenx, y,   z, 
            x+lenx, y,   z+lenz, 
            x+lenx, y+leny, z+lenz, 
        };

        std::vector<float> n = {
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
            1.0f,  0.0f,  0.0f,
        };

        float u0 = tex_col * len_x; // Left edge (u)
        float v0 = tex_row * len_y; // Bottom edge (v)
        float u1 = u0 + len_x;      // Right edge (u)
        float v1 = v0 + len_y;      // Top edge (v)

        // grows downwards
        std::vector<float> t = {
            u0, v1, 
            u1, v1,
            u1, v0,

            u1, v0,
            u0, v0,
            u0, v1,
        };
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genFrontFaceSexy(float x, float y, float z, float lenx, float leny, float lenz, int tex_row, int tex_col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
        std::vector<float> v = {
            // Front Face
            x,   y,   z+lenz,
            x+lenx, y,   z+lenz,
            x+lenx, y+leny, z+lenz,
            x+lenx, y+leny, z+lenz,
            x,   y+leny, z+lenz,
            x,   y,   z+lenz
        };

        std::vector<float> n = {
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,  1.0f,
        };

        float u0 = tex_col * len_x; // Left edge (u)
        float v0 = tex_row * len_y; // Bottom edge (v)
        float u1 = u0 + len_x;      // Right edge (u)
        float v1 = v0 + len_y;      // Top edge (v)

        std::vector<float> t = {
            // Tri 1
            u0, v0, // Corresponds to vertex 1 (bottom-left)
            u1, v0, // Corresponds to vertex 2 (bottom-right)
            u1, v1, // Corresponds to vertex 3 (top-right)
            // Tri 2
            u1, v1, // Corresponds to vertex 4 (top-right)
            u0, v1, // Corresponds to vertex 5 (top-left)
            u0, v0, // Corresponds to vertex 6 (bottom-left)
        };
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
    }

    void genBackFaceSexy(float x, float y, float z, float lenx, float leny, float lenz, int tex_row, int tex_col, std::vector<float>& vertices, std::vector<float>& normals, std::vector<float>& textures) {
    
        std::vector<float> v = {
            // Back Face
            x,   y,   z,
            x+lenx, y,   z,
            x+lenx, y+leny, z,
            x+lenx, y+leny, z,
            x,   y+leny, z,
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

        float u0 = tex_col * len_x; // Left edge (u)
        float v0 = tex_row * len_y; // Bottom edge (v)
        float u1 = u0 + len_x;      // Right edge (u)
        float v1 = v0 + len_y;      // Top edge (v)

        std::vector<float> t = {
            // Tri 1
            u0, v0, // Corresponds to vertex 1 (bottom-left)
            u1, v0, // Corresponds to vertex 2 (bottom-right)
            u1, v1, // Corresponds to vertex 3 (top-right)
            // Tri 2
            u1, v1, // Corresponds to vertex 4 (top-right)
            u0, v1, // Corresponds to vertex 5 (top-left)
            u0, v0, // Corresponds to vertex 6 (bottom-left)
        };
        vertices.insert(vertices.end(), v.begin(), v.end());
        normals.insert (normals.end(),  n.begin(), n.end());
        textures.insert(textures.end(), t.begin(), t.end());
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

        int chunk_len_2 = CHUNK_LENGTH+2;
        std::vector<bool> arr (chunk_len_2 * chunk_len_2  * chunk_len_2 , 0);

        auto is_filled = [&arr, chunk_len_2](uint8_t x, uint8_t y, uint8_t z) {
            return arr[z * chunk_len_2 * chunk_len_2 + y * chunk_len_2 + x] == 1;
        };

        f32 f = 1.0f / 32.0f; // the smaller the more coarse
        uint8_t* val = (uint8_t*) malloc(4 * sizeof(uint8_t));

        for (int z=0; z < chunk_len_2; z++) {
            for (int y=0; y < chunk_len_2; y++) {
                for (int x=0; x < chunk_len_2; x += 4) {

                    perlinNoiseSIMD_4x(((x-1) + chunk.pos.x * CHUNK_LENGTH + 10000) * f, ((y-1) + chunk.pos.y * CHUNK_LENGTH + 10000) * f, ((z-1) + chunk.pos.z * CHUNK_LENGTH + 10000) * f, f, val);
                    
                    for (int i = 0; i < 4; i++) {
                        if (x+i >= chunk_len_2) break; // were done here
                        if (val[i] >= PERLIN_THRESHOLD)
                            arr[z * chunk_len_2 * chunk_len_2 + y * chunk_len_2 + x+i] = 1;
                    }
                }
            }
        }

        int tex_row = 13-1;
        int tex_col = 15-1;

        // for (int z=1; z <= CHUNK_LENGTH; z++) {
        //     for (int y=1; y <= CHUNK_LENGTH; y++) {
        //         for (int x=1; x <= CHUNK_LENGTH; x++) {
        //             if (!is_filled(x,y,z)) continue;
                    // if (!is_filled(x+1,y,z)) 
                    //     genRightFace (x-1,y-1,z-1,1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                    // if (!is_filled(x-1,y,z))
                    //     genLeftFace  (x-1,y-1,z-1,1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                    // // if (!is_filled(x,y+1,z))
                    //     genTopFace   (x-1,y-1,z-1,1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                    // if (!is_filled(x,y-1,z))
                    //     genBotFace   (x-1,y-1,z-1,1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                    // if (!is_filled(x,y,z+1))
                    //     genFrontFace (x-1,y-1,z-1,1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                    // if (!is_filled(x,y,z-1))
                    //     genBackFace  (x-1,y-1,z-1,1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
        //         }
        //     }
        // }


        for (int y=1; y <= CHUNK_LENGTH; y++) {

            std::vector<bool> top_faces_occ (CHUNK_LENGTH * CHUNK_LENGTH, false);
            std::vector<bool> top_faces_filled (CHUNK_LENGTH * CHUNK_LENGTH, false);

            auto is_top_filled = [&top_faces_filled](int x, int z) {
                return top_faces_filled[(x-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            auto set_top_filled = [&top_faces_filled](int x, int z) {
                top_faces_filled[(x-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto set_top_occ = [&top_faces_occ](int x, int z) {
                top_faces_occ[(x-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto is_top_occ = [&top_faces_occ](int x, int z) {
                return top_faces_occ[(x-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            for (int z=1; z <= CHUNK_LENGTH; z++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    if (is_filled(x, y, z))
                        set_top_filled(x,z);
                }
            }

            for (int z=1; z <= CHUNK_LENGTH; z++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    // if above filled or you not filled
                    if (is_filled(x,y+1,z) || !is_filled(x,y,z) || is_top_occ(x,z)) continue;

                    int length_x = 1;
                    int length_z = 1;
                    bool stretch;
                    bool skip;
                    do {
                        stretch = false;  
                        if (x+length_x < CHUNK_LENGTH && z+length_z-1 < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_z; i++) {
                                if (is_filled(x+length_x,y+1,z+i) || !is_top_filled(x+length_x,z+i) || is_top_occ(x+length_x,z+i))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_z; i++)
                                    set_top_occ(x+length_x,z+i);    
                                length_x++;
                            }
                        }
                        if (x+length_x-1 < CHUNK_LENGTH && z+length_z < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_x; i++) {
                                if (is_filled(x+i,y+1,z+length_z) || !is_top_filled(x+i,z+length_z) || is_top_occ(x+i,z+length_z))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_x; i++)
                                    set_top_occ(x+i,z+length_z);
                                length_z++;
                            }
                        }
                    } while (stretch);

                    genTopFaceSexy (x-1,y-1,z-1,length_x, 1, length_z,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                }
            }
        }

        for (int y=1; y <= CHUNK_LENGTH; y++) {

            std::vector<bool> bot_faces_occ (CHUNK_LENGTH * CHUNK_LENGTH, false);
            std::vector<bool> bot_faces_filled (CHUNK_LENGTH * CHUNK_LENGTH, false);

            auto is_bot_filled = [&bot_faces_filled](int x, int z) {
                return bot_faces_filled[(x-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            auto set_bot_filled = [&bot_faces_filled](int x, int z) {
                bot_faces_filled[(x-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto set_bot_occ = [&bot_faces_occ](int x, int z) {
                bot_faces_occ[(x-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto is_bot_occ = [&bot_faces_occ](int x, int z) {
                return bot_faces_occ[(x-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            for (int z=1; z <= CHUNK_LENGTH; z++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    if (is_filled(x, y, z))
                        set_bot_filled(x,z);
                }
            }

            for (int z=1; z <= CHUNK_LENGTH; z++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    // if above filled or you not filled
                    if (is_filled(x,y-1,z) || !is_filled(x,y,z) || is_bot_occ(x,z)) continue;

                    int length_x = 1;
                    int length_z = 1;
                    bool stretch;
                    bool skip;
                    do {
                        stretch = false;  
                        if (x+length_x < CHUNK_LENGTH && z+length_z-1 < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_z; i++) {
                                if (is_filled(x+length_x,y-1,z+i) || !is_bot_filled(x+length_x,z+i) || is_bot_occ(x+length_x,z+i))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_z; i++)
                                    set_bot_occ(x+length_x,z+i);    
                                length_x++;
                            }
                        }
                        if (x+length_x-1 < CHUNK_LENGTH && z+length_z < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_x; i++) {
                                if (is_filled(x+i,y-1,z+length_z) || !is_bot_filled(x+i,z+length_z) || is_bot_occ(x+i,z+length_z))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_x; i++)
                                    set_bot_occ(x+i,z+length_z);
                                length_z++;
                            }
                        }
                    } while (stretch);

                    genBotFaceSexy (x-1,y-1,z-1,length_x, 1, length_z,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                }
            }
        }

        for (int z=1; z <= CHUNK_LENGTH; z++) {

            std::vector<bool> bot_faces_occ (CHUNK_LENGTH * CHUNK_LENGTH, false);
            std::vector<bool> bot_faces_filled (CHUNK_LENGTH * CHUNK_LENGTH, false);

            auto is_bot_filled = [&bot_faces_filled](int x, int y) {
                return bot_faces_filled[(y-1) * CHUNK_LENGTH + (x-1)] == true;
            };

            auto set_bot_filled = [&bot_faces_filled](int x, int y) {
                bot_faces_filled[(y-1) * CHUNK_LENGTH + (x-1)] = true;
            };

            auto set_bot_occ = [&bot_faces_occ](int x, int y) {
                bot_faces_occ[(y-1) * CHUNK_LENGTH + (x-1)] = true;
            };

            auto is_bot_occ = [&bot_faces_occ](int x, int y) {
                return bot_faces_occ[(y-1) * CHUNK_LENGTH + (x-1)] == true;
            };

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    if (is_filled(x, y, z))
                        set_bot_filled(x,y);
                }
            }

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    // if above filled or you not filled
                    if (is_filled(x,y,z+1) || !is_filled(x,y,z) || is_bot_occ(x,y)) continue;

                    int length_x = 1;
                    int length_y = 1;
                    bool stretch;
                    bool skip;
                    do {
                        stretch = false;  
                        if (x+length_x < CHUNK_LENGTH && y+length_y-1 < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_y; i++) {
                                if (is_filled(x+length_x,y+i,z+1) || !is_bot_filled(x+length_x,y+i) || is_bot_occ(x+length_x,y+i))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_y; i++)
                                    set_bot_occ(x+length_x,y+i);    
                                length_x++;
                            }
                        }
                        if (x+length_x-1 < CHUNK_LENGTH && y+length_y < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_x; i++) {
                                if (is_filled(x+i,y+length_y,z+1) || !is_bot_filled(x+i,y+length_y) || is_bot_occ(x+i,y+length_y))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_x; i++)
                                    set_bot_occ(x+i,y+length_y);
                                length_y++;
                            }
                        }
                    } while (stretch);

                    genFrontFaceSexy (x-1,y-1,z-1,length_x, length_y, 1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                }
            }
        }

        for (int z=1; z <= CHUNK_LENGTH; z++) {

            std::vector<bool> bot_faces_occ (CHUNK_LENGTH * CHUNK_LENGTH, false);
            std::vector<bool> bot_faces_filled (CHUNK_LENGTH * CHUNK_LENGTH, false);

            auto is_bot_filled = [&bot_faces_filled](int x, int y) {
                return bot_faces_filled[(y-1) * CHUNK_LENGTH + (x-1)] == true;
            };

            auto set_bot_filled = [&bot_faces_filled](int x, int y) {
                bot_faces_filled[(y-1) * CHUNK_LENGTH + (x-1)] = true;
            };

            auto set_bot_occ = [&bot_faces_occ](int x, int y) {
                bot_faces_occ[(y-1) * CHUNK_LENGTH + (x-1)] = true;
            };

            auto is_bot_occ = [&bot_faces_occ](int x, int y) {
                return bot_faces_occ[(y-1) * CHUNK_LENGTH + (x-1)] == true;
            };

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    if (is_filled(x, y, z))
                        set_bot_filled(x,y);
                }
            }

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int x=1; x <= CHUNK_LENGTH; x++) {
                    // if above filled or you not filled
                    if (is_filled(x,y,z-1) || !is_filled(x,y,z) || is_bot_occ(x,y)) continue;

                    int length_x = 1;
                    int length_y = 1;
                    bool stretch;
                    bool skip;
                    do {
                        stretch = false;  
                        if (x+length_x < CHUNK_LENGTH && y+length_y-1 < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_y; i++) {
                                if (is_filled(x+length_x,y+i,z-1) || !is_bot_filled(x+length_x,y+i) || is_bot_occ(x+length_x,y+i))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_y; i++)
                                    set_bot_occ(x+length_x,y+i);    
                                length_x++;
                            }
                        }
                        if (x+length_x-1 < CHUNK_LENGTH && y+length_y < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_x; i++) {
                                if (is_filled(x+i,y+length_y,z-1) || !is_bot_filled(x+i,y+length_y) || is_bot_occ(x+i,y+length_y))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_x; i++)
                                    set_bot_occ(x+i,y+length_y);
                                length_y++;
                            }
                        }
                    } while (stretch);

                    genBackFaceSexy (x-1,y-1,z-1,length_x, length_y, 1,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                }
            }
        }
    
        for (int x=1; x <= CHUNK_LENGTH; x++) {

            std::vector<bool> top_faces_occ (CHUNK_LENGTH * CHUNK_LENGTH, false);
            std::vector<bool> top_faces_filled (CHUNK_LENGTH * CHUNK_LENGTH, false);

            auto is_top_filled = [&top_faces_filled](int y, int z) {
                return top_faces_filled[(y-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            auto set_top_filled = [&top_faces_filled](int y, int z) {
                top_faces_filled[(y-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto set_top_occ = [&top_faces_occ](int y, int z) {
                top_faces_occ[(y-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto is_top_occ = [&top_faces_occ](int y, int z) {
                return top_faces_occ[(y-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int z=1; z <= CHUNK_LENGTH; z++) {
                    if (is_filled(x, y, z))
                        set_top_filled(y,z);
                }
            }

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int z=1; z <= CHUNK_LENGTH; z++) {
                    // if above filled or you not filled
                    if (is_filled(x+1,y,z) || !is_filled(x,y,z) || is_top_occ(y,z)) continue;

                    int length_y = 1;
                    int length_z = 1;
                    bool stretch;
                    bool skip;
                    do {
                        stretch = false;  
                        if (y+length_y < CHUNK_LENGTH && z+length_z-1 < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_z; i++) {
                                if (is_filled(x+1,y+length_y,z+i) || !is_top_filled(y+length_y,z+i) || is_top_occ(y+length_y,z+i))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_z; i++)
                                    set_top_occ(y+length_y,z+i);    
                                length_y++;
                            }
                        }
                        if (y+length_y-1 < CHUNK_LENGTH && z+length_z < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_y; i++) {
                                if (is_filled(x+1,y+i,z+length_z) || !is_top_filled(y+i,z+length_z) || is_top_occ(y+i,z+length_z))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_y; i++)
                                    set_top_occ(y+i,z+length_z);
                                length_z++;
                            }
                        }
                    } while (stretch);

                    genRightFaceSexy (x-1,y-1,z-1,1,length_y,length_z,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                }
            }
        }

        for (int x=1; x <= CHUNK_LENGTH; x++) {

            std::vector<bool> top_faces_occ (CHUNK_LENGTH * CHUNK_LENGTH, false);
            std::vector<bool> top_faces_filled (CHUNK_LENGTH * CHUNK_LENGTH, false);

            auto is_top_filled = [&top_faces_filled](int y, int z) {
                return top_faces_filled[(y-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            auto set_top_filled = [&top_faces_filled](int y, int z) {
                top_faces_filled[(y-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto set_top_occ = [&top_faces_occ](int y, int z) {
                top_faces_occ[(y-1) * CHUNK_LENGTH + (z-1)] = true;
            };

            auto is_top_occ = [&top_faces_occ](int y, int z) {
                return top_faces_occ[(y-1) * CHUNK_LENGTH + (z-1)] == true;
            };

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int z=1; z <= CHUNK_LENGTH; z++) {
                    if (is_filled(x, y, z))
                        set_top_filled(y,z);
                }
            }

            for (int y=1; y <= CHUNK_LENGTH; y++) {
                for (int z=1; z <= CHUNK_LENGTH; z++) {
                    // if above filled or you not filled
                    if (is_filled(x-1,y,z) || !is_filled(x,y,z) || is_top_occ(y,z)) continue;

                    int length_y = 1;
                    int length_z = 1;
                    bool stretch;
                    bool skip;
                    do {
                        stretch = false;  
                        if (y+length_y < CHUNK_LENGTH && z+length_z-1 < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_z; i++) {
                                if (is_filled(x-1,y+length_y,z+i) || !is_top_filled(y+length_y,z+i) || is_top_occ(y+length_y,z+i))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_z; i++)
                                    set_top_occ(y+length_y,z+i);    
                                length_y++;
                            }
                        }
                        if (y+length_y-1 < CHUNK_LENGTH && z+length_z < CHUNK_LENGTH) {
                            skip = false;
                            for (int i = 0; i < length_y; i++) {
                                if (is_filled(x-1,y+i,z+length_z) || !is_top_filled(y+i,z+length_z) || is_top_occ(y+i,z+length_z))
                                    skip = true;
                            }
                            if (!skip) {
                                stretch = true;
                                for (int i = 0; i < length_y; i++)
                                    set_top_occ(y+i,z+length_z);
                                length_z++;
                            }
                        }
                    } while (stretch);

                    genLeftFaceSexy (x-1,y-1,z-1,1,length_y,length_z,tex_row,tex_col,chunk.vertices,chunk.normals,chunk.textures);
                }
            }
        }
    }
}