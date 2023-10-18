// References:
// 1. https://learnopengl.com/PBR/IBL/Specular-IBL
// 2. https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// 3. http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

#version 450 core
out vec4 frag_color;

in vec3 frag_pos;

#define M_PI 3.1415926535897932384626433832795

uniform samplerCube map;

void main()
{
    vec3 n = normalize(frag_pos);
    vec3 color = vec3(0.0);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, n));
    up = normalize(cross(n,right));

    float d = 0.0245;
    float n_samples = 0.0;

    for(float i=0.0; i<2.0*M_PI; i+=d)
    {
        for(float j=0.0; j<0.5*M_PI; j+=d)
        {
            float x = sin(j)*cos(i);
            float y = sin(j)*sin(i);
            float z = cos(j);

            vec3 sample_coord = vec3(x*right + y*up + z*n);
            color += texture(map, sample_coord).rgb * cos(j) * sin(j);
            n_samples++;
        }
    }
    color = M_PI * color * (1.0 / float(n_samples));
    frag_color = vec4(color, 1.0);
}