#version 330 core
out vec4 frag_color;

in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
} fs_in;

struct Light
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material
{
	vec3 color;
    float metallic;
    float roughness;
	float ao;
};

uniform Light light;
uniform Material mat;
uniform vec3 view_pos;
uniform vec3 light_pos;
uniform float colors[1000];

in float x;
in float y;
in float z;

vec3 CalcDirLight(Light light, vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-light.direction);

	// Ambient
	float amb = 0.5f;
	vec3 ambient = light.ambient * amb;
	
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff;

	// Specular
	vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	vec3 h = normalize(light_dir + view_dir);
	float spec = pow(max(dot(view_dir, h), 0.0), 32.0f);	
	vec3 specular = light.specular * spec;

	return (ambient + diffuse + specular ) * mat.color;
}


void main()
{
    vec3 norm = normalize(fs_in.frag_norm);
	vec3 view_dir = normalize(view_pos - fs_in.frag_pos);
	vec3 light_dir = normalize(light_pos - fs_in.frag_pos);

    vec3 result = CalcDirLight(light, norm, view_dir);
    result = result / (result + vec3(1.0)); // HDR
    result = pow(result, vec3(1.0/2.2)); // Gamma correction
    
	int grid_index = int(x + 10.0 * z+ 100.0 *y);
	float grid_color = colors[grid_index];
	
	if(grid_color > 0.2)
	{
		grid_color = 1.0;
	}

	frag_color = vec4(result, 1.0); // color[index].x, color[index].y, color[index].z
}