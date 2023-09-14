#version 330 core
layout(location = 0) out float fragmentdepth;

void main()
{
    //Render the depth value to the depth buffer, no need to output the color
    fragmentdepth = gl_FragCoord.z;
}