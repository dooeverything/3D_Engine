// References:
// 1. https://learnopengl.com/PBR/IBL/Specular-IBL
// 2. https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// 3. http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

#version 410 core

out vec4 frag_color;
in vec3 frag_pos;

uniform samplerCube map;
uniform float roughness;

#define M_PI 3.1415926535897932384626433832795

float normalDistribution(vec3 n, vec3 h, float roughness)
{
    float a = pow(roughness, 4);
    float d = pow(max(dot(n,h), 0.0), 2) * (a-1.0) + 1.0;
    return a/(M_PI * pow(d,2));
}

float radicalInverseVDC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0x00FF00FFu) >> 8u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0x0F0F0F0Fu) >> 4u); 
    bits = ((bits & 0x33333333u) << 2u) | ((bits * 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    return float(bits) * 2.3283064365386963e-10; 
}

vec2 hammersleyPoint(uint i, uint N)
{
    return vec2(float(i)/float(N), radicalInverseVDC(i));
}

vec3 importanceSample(vec2 xi, vec3 n, float roughness)
{
    float a = pow(roughness, 4);

    float phi = 2 * M_PI * xi.x;
    float cos_theta = sqrt((1-xi.y) / (1+(a*a -1)*xi.y));
    float sin_theta = sqrt(1 - cos_theta*cos_theta);

    vec3 h;
    h.x = sin_theta * cos(phi);
    h.y = sin_theta * sin(phi);
    h.z = cos_theta;

    vec3 up = abs(n.z) < 0.999 ? vec3(0.0,0.0,1.0) : vec3(1.0,0.0,0.0);
    vec3 tangent = normalize(cross(up, n));
    vec3 bitangent = cross(n, tangent);

    vec3 sample_vec = tangent * h.x + bitangent * h.y + n * h.z;
    return normalize(sample_vec);
}

void main()
{
    vec3 n = normalize(frag_pos);
    vec3 r = n;
    vec3 v = r;

    const uint num_samples = 1024u;
    vec3 color = vec3(0.0);
    float total_weight = 0.0;
    
    for(uint i=0; i<num_samples; ++i)
    {
        vec2 xi = hammersleyPoint(i, num_samples);
        vec3 h = importanceSample(xi, n, roughness);
        vec3 l =  normalize(2 * dot(v, h) * h - v);

        if(max(dot(n,l), 0.0) > 0)
        {
            float D = normalDistribution(n, h, roughness);
            float pdf = D * max(dot(n,h),0.0) / (4.0*max(dot(v,h),0.0)+0.0001);
            float resolution = 4096;
            float sa_texel = 4.0 * M_PI / (6.0 * resolution * resolution);
            float sa_sample = 1.0 / (float(num_samples) * pdf + 0.0001);

            float level = roughness == 0.0 ? 0.0 : 0.5 * log2(sa_sample/sa_texel);

            color += textureLod(map, l, level).rgb * max(dot(n,l), 0.0);
            total_weight += max(dot(n,l),0.0);
        }
    }

    color = color / total_weight;
    frag_color = vec4(color, 1.0);
}


