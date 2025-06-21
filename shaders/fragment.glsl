#version 330 core
out vec4 FragColor;

in vec3 faceNormal;
in vec3 fragPos;
in vec2 texCoord;

uniform vec3 light_position;
uniform sampler2D textureSampler;

void main()
{
    vec4 texColor = texture(textureSampler, texCoord);
    // the dot make things blacker if they are on the opposite facing side
    vec3 dir_to_light = normalize(light_position-fragPos);
    float lambert = dot(faceNormal, dir_to_light);
    // float lambert = 1.0;
    FragColor = vec4(texColor.rgb * lambert, 1.0);
}