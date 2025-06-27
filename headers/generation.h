#pragma once
#include "helpers.h"
#include "frustrum.h"
#include <vector>
#include <tbb/concurrent_queue.h>
#include <tbb/task_group.h>
#include "stb_image.h"
class generator {

public:

generator (Program* ShaderProgram) {

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("./textures/atlas.png", &width, &height, &nrChannels, 0);

    if (data)
    {
        std::cout << "nrChannels: " << nrChannels << std::endl;
        // Note: If your atlas has transparency (alpha channel), use GL_RGBA
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }

    glUniform1i(glGetUniformLocation(ShaderProgram->get_id(), "textureSampler"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    stbi_image_free(data);
}

void start_generation_tasks (const std::unordered_set<glm::ivec3, IVec3Hash>& required) {

    for (const auto& pos : required) {
        if (!active_chunks.count(pos)) { 
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            if (m_pending_generation.insert(pos).second) { 
                tasks_in_flight.fetch_add(1, std::memory_order_relaxed);
                task_group.run([this, pos, required]() { 
                    if (!required.count(pos)) return;
                    generate_chunk(pos);
                    tasks_in_flight.fetch_sub(1, std::memory_order_relaxed);
                });
            }
        }
    }
}

void prune_unnecessary_chunks (const std::unordered_set<glm::ivec3, IVec3Hash>& required) {
    // delete all chunks in memory that are not needed
    std::erase_if(active_chunks, [&](const auto& pair) {
        const glm::ivec3& pos = pair.first;
        return !required.count(pos);
    });
}

void process_finished_mesh (const std::unordered_set<glm::ivec3, IVec3Hash>& required) {

    const auto budget = std::chrono::milliseconds(20);
    auto start_time = std::chrono::high_resolution_clock::now();

    chunkData mesh;
    while (true) {
        auto now = std::chrono::high_resolution_clock::now();
        if (now - start_time > budget) {
            return; // if popping for 5 ms straight, stop. 
        }
        // pop all data from finished_mesh_queue
        if (!finished_mesh_queue.try_pop(mesh)) continue;

        if (!required.count(mesh.pos)) continue;

        auto chunk_ptr = std::make_unique<chunkData>();
        chunk_ptr->pos = std::move(mesh.pos);
        chunk_ptr->vertices = std::move(mesh.vertices);
        chunk_ptr->normals = std::move(mesh.normals);
        chunk_ptr->textures = std::move(mesh.textures);

        set_vao_vbo(*chunk_ptr);

        active_chunks[chunk_ptr->pos] = std::move(chunk_ptr); 

        {
            std::lock_guard<std::mutex> lock(m_pending_mutex);
            m_pending_generation.erase(mesh.pos);
        }
    }
}

void print_task_count() {
    std::cout << "tasks running: " << tasks_in_flight << std::endl;
}

void draw_all (Program* ShaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    
    glUseProgram(ShaderProgram->get_id());
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); 

    // Set uniforms that are the same for all chunks ONCE before the loop
    glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Bind the texture atlas once
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->texture);
    glUniform1i(glGetUniformLocation(ShaderProgram->get_id(), "textureSampler"), 0);

    // Get the location of the model uniform ONCE before the loop for efficiency
    GLint modelLoc = glGetUniformLocation(ShaderProgram->get_id(), "model");

    Frustum frustum;
    frustum.update(projection * view);

    int chunksDrawn = 0;

    // std::unordered_set<glm::ivec3, IVec3Hash> viewable_chunks;
    // occlusion_culling(viewable_chunks);

    for (const auto& pair : active_chunks) {
        const chunkData* data = pair.second.get();
        if (data->vertices.size() == 0) continue;
        // if (!viewable_chunks.contains(pair.first)) continue;

        glm::vec3 min = glm::vec3(data->pos) * (float)CHUNK_LENGTH;
        glm::vec3 max = min + glm::vec3(CHUNK_LENGTH, CHUNK_LENGTH, CHUNK_LENGTH);

        if (!frustum.isBoxVisible(min, max)) {
            continue; // Skip this chunk, it's not visible!
        }

        chunksDrawn++;

        glm::mat4 model = glm::mat4(1.0f);
        // The key 'pos' is the chunk's grid coordinate (e.g., (1, 0, 2)).
        // We multiply by CHUNK_LENGTH to get the real world coordinate (e.g., (16, 0, 32)).
        model = glm::translate(model, min);

        // 2. Send this chunk-specific model matrix to the shader.
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(data->vao);
        glDrawArrays(GL_TRIANGLES, 0, data->vertices.size() / 3);
    }
    // std::cout << "chunks drawn: " << chunksDrawn << std::endl;
}

private:

unsigned int texture;

std::atomic<int> tasks_in_flight{0};

std::unordered_set<glm::ivec3, IVec3Hash> m_pending_generation;
std::mutex m_pending_mutex;

// final info passed to GPU
std::unordered_map<glm::ivec3, std::unique_ptr<chunkData>, IVec3Hash> active_chunks;

tbb::task_group task_group;
tbb::concurrent_queue<chunkData> finished_mesh_queue;   

void set_vao_vbo (chunkData& mesh) {
    glGenVertexArrays(1, &mesh.vao);
    glBindVertexArray(mesh.vao);

    glGenBuffers(1, &mesh.vbo_pos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_pos);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(mesh.vertices[0]), &mesh.vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &mesh.vbo_norm);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_norm);
    glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(mesh.normals[0]), &mesh.normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &mesh.vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, mesh.textures.size() * sizeof(mesh.textures[0]), &mesh.textures[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
}

void calculate_mesh (chunkData& chunk) {
    generator_helper::calculate_mesh(chunk);
}

void generate_chunk (glm::ivec3 pos) {
    chunkData chunk;
    chunk.pos = pos;
    calculate_mesh(chunk); // this is the heavy stuff
    finished_mesh_queue.push(std::move(chunk));
}

void occlusion_culling (std::unordered_set<glm::ivec3, IVec3Hash>& viewable_chunks) {
    // start from the camera, shoot rays 
    
}
};