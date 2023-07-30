// References:
// 1. https://learnopengl.com/PBR/IBL/Specular-IBL
// 2. https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// 3. http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

#version 410 core
out vec4 frag_color;

in vec3 frag_pos;

uniform sampler2D map;

const vec2 inv_atan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= inv_atan;
    uv += 0.5;
    return uv;
}

void main()
{
    vec2 uv = SampleSphericalMap(normalize(frag_pos));
    vec3 color = texture(map, uv).rgb;
    frag_color = vec4(color, 1.0);
}