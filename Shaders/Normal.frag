#version 330 core
out vec4 frag_color;

in vec2 texCoords;
in mat4 inverse_proj;
in mat4 out_view;

uniform sampler2D map;
uniform vec3 light_pos;
uniform vec2 inverse_tex;

vec3 uvToView(vec2 uv, float z)
{
    vec2 pos = uv*2.0f - 1.0f;
    vec4 clip = vec4(pos, z, 1.0);
    vec4 view = inverse_proj * clip;
    return view.xyz/view.w;
}

const vec3 light_dir = vec3(-0.2f, -1.0f, -0.3f);
void main()
{
    float depth = texture(map, texCoords).r;
    // float blue = texture(map, texCoords).b;

    if(depth == 0.0)
    {
        frag_color = vec4(0.0);
        return;
    }

    if(depth == 1.0)
    {
        frag_color = vec4(texCoords, 1.0, 1.0);
        return;
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


    vec3 n = normalize(cross(ddx, ddy));
    vec3 l =  normalize(vec4(-light_dir, 0.0).xyz);
    vec3 v = normalize(v_pos);
    
    float amb = 0.5f;
    vec3 ambient = vec3(1.0, 1.0, 1.0) * amb * vec3(0.65, 0.85, 0.90);

    float diff = max(dot(n, l), 0.0);
    vec3 diffuse = vec3(0.8, 0.8, 0.8) * diff * vec3(0.65, 0.85, 0.90);

    vec3 reflect_dir = 2 * dot(n, l) * n - l;
    vec3 h = normalize(l + v);
	float spec = pow(max(dot(v, h), 0.0), 32.0);	
    vec3 specular = vec3(0.5, 0.5, 0.5) * spec * vec3(0.65, 0.85, 0.90);

    vec3 color = ambient + diffuse + specular;
    frag_color = vec4(color, 1.0);

}