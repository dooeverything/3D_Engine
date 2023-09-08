#version 330 core
out vec4 frag_color;

in vec2 texCoords;

uniform vec3 light_pos;
uniform sampler2D map;
uniform vec2 inverse_tex;

vec3 uvToView(vec2 uv, float z)
{
    vec2 pos = uv*2.0f - 1.0f;
    vec4 clip = vec4(pos, z, 1.0);
    vec3 view = inverse(projection) * clip;
    return view.xyz/view.w;
}

void main()
{
    float depth = texture2D(map, texCoords).r;

    if(depth == 0.0)
    {
        frag_color = vec4(0.0);
        return;
    }

    if(depth == 1.0)
    {
        frag_color = vec4(1.0);
    }

    vec3 v_pos = uvToView(texCoords, depth);

    vec3 t1 = v_pos - uvToView(texCoords - vec2(inverse_tex.x, 0.0), texture(map, texCoords - vec2(inverse_tex.x, 0.0)).r);
    vec3 t2 = uvToView(texCoords + vec2(inverse_tex.x, 0.0), texture(map, texCoords + vec2(inverse_tex.x, 0.0)).r) - v_pos;
    
    vec3 t3 = v_pos - uvToView(texCoords - vec2(0.0, inverse_tex.y), texture(map, texCoords - vec2(0.0, inverse_tex.y)).r);
    vec3 t4 = uvToView(texCoords + vec2(0.0, inverse_tex.y), texture(map, texCoords + vec2(0.0, inverse_tex.y)).r) - v_pos;

    vec3 ddx = t1;
    vec3 ddy = t3;

    if(abs(t2.z) < abs(t1.z))
    {
        ddx = t2;
    }

    if(abs(t4.z) < abs(t3.z))
    {
        ddy = t4;
    }

    vec3 normal = normalize(cross(ddx, ddy));
    frag_color = vec4(normal, 1.0);

    // vec3 n = texture2D(map, texCoords).rgb;
    // vec3 l = normalize(light_pos - pos);
    // float color = dot(n, l)*0.5 + 0.5;

    // if(color == 0.0)
    // {
    //     frag_color = vec4(0.0);
    // }
    // else
    //     frag_color = vec4(vec3(color), 1.0);
}