#version 330 core
out vec4 frag_color;

in vec2 texCoords;
uniform sampler2D map;

void main()
{
    vec3 color = texture2D(map, texCoords).rgb;

    if(color == 1.0)
    {
        frag_color = vec4(0.0);
    }
    else
    {
        frag_color = vec4(color, 1.0);
    }
}