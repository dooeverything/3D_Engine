#version 330 core
out vec4 FragColor;

in vec2 texCoords;

uniform sampler2D map;

void main()
{
    float color = texture2D(map, texCoords).r;
    FragColor = vec4(vec3(color), 1.0);
}