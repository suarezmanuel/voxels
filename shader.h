#pragma once 

#include "helpers.h"

class Shader {

private:
    unsigned int id = -1;
public:
    Shader(const std::string& source, int type_enum, const std::string& type) {
        std::ifstream is(source);
        const std::string shader_source{std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>()};
        const char* shader_content = shader_source.c_str();

        // 5. Build and compile our shader program
        unsigned int shader = glCreateShader(type_enum);
        glShaderSource(shader, 1, &shader_content, NULL);
        glCompileShader(shader);
        shader_helper::checkCompileErrors(shader, type);
        id = shader;
    }

    unsigned int get_id() {
        return id;
    }
};