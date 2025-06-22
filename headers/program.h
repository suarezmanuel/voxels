#pragma once 

#include "helpers.h"
#include "shader.h"

class Program {

private:
    unsigned int id = -1;
public:
    Program(std::string vshader_string, std::string fshader_string) {
        Shader* VertexShader = new Shader(vshader_string, GL_VERTEX_SHADER, "VERTEX");
        Shader* FragmentShader = new Shader(fshader_string, GL_FRAGMENT_SHADER, "FRAGMENT");
        this->id = glCreateProgram();

        glAttachShader(this->id, VertexShader->get_id());
        glAttachShader(this->id, FragmentShader->get_id());
        glLinkProgram(this->id);

        shader_helper::checkCompileErrors(this->id, "PROGRAM");
        glDeleteShader(VertexShader->get_id());
        glDeleteShader(FragmentShader->get_id());
        delete VertexShader;
        delete FragmentShader;
    }

    unsigned int get_id() {
        return id;
    }
};