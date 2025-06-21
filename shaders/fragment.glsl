#version 330 core
out vec4 FragColor;

// in vec3 color;
uniform vec3 color;
uniform vec3 light_position;
in vec3 faceNormal;
in vec4 fragPos;

void main()
{
    // the dot make things blacker if they are on the opposite facing side
    vec3 dir_to_light = normalize(light_position-fragPos.xyz);
    // float lambert = dot(faceNormal, dir_to_light);
    float lambert = 1.0;
    FragColor = vec4(abs(faceNormal) * lambert, 1.0);
}