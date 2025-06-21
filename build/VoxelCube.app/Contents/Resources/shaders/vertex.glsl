#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 aTexCoord;

// out vec3 color;
out vec3 faceNormal;
out vec3 fragPos;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    fragPos = (model * vec4(aPos, 1.0)).xyz;
    faceNormal = mat3(transpose(inverse(model))) * inNormal;
    texCoord = aTexCoord;
}