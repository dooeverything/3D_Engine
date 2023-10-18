// References:
// 1. https://learnopengl.com/PBR/IBL/Specular-IBL
// 2. https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// 3. http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

#version 450 core
out vec4 frag_color;
in vec2 frag_texCoord;

#define M_PI 3.1415926535897932384626433832795

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
    float a = pow(roughness, 2);
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

float geometry(float cos_theta, float roughness)
{
    float k = pow((roughness),2) / 2.0;
    float d = cos_theta*(1-k) + k;
    return cos_theta / d;
}

vec2 integratedBRDF(float x, float y)
{
    // x : N dot V
    // y : roughness 

    vec3 n = vec3(0.0, 0.0, 1.0);
    vec3 v;
    v.x = sqrt(1.0f - x*x);
    v.y = 0;
    v.z = x;

    float sum1 = 0.0;
    float sum2 = 0.0;

    const uint num_samples = 1024u;
    for(uint i=0; i<num_samples; ++i)
    {
        vec2 xi = hammersleyPoint(i, num_samples);
        vec3 h = importanceSample(xi, n, y);
        vec3 l = normalize(2 * dot(v, h) * h - v);

        if(max(l.z, 0.0) > 0)
        {
            float G = geometry(max(dot(n,l),0.0), y) * geometry(max(dot(n,v),0.0), y);
            G = (G * max(dot(v,h), 0.0)) / (max(h.z, 0.0) * x);
            float Fc = pow( (1.0 - max(dot(v,h), 0.0)), 5);
            sum1 += (1.0-Fc) * G;
            sum2 += Fc * G;
        }
    }

    return vec2(sum1/float(num_samples), sum2/float(num_samples));
}

void main()
{
    vec2 color = integratedBRDF(frag_texCoord.x, frag_texCoord.y);
    frag_color = vec4(color, 0.0, 1.0); //color;
}