#version 330 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 inColor;
layout (location = 1) in vec3 inNormal;

// out vec3 color;
out vec3 faceNormal;
out vec4 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    fragPos = gl_Position;
    // faceNormal = normalize(inNormal);
    faceNormal = inNormal; 
}