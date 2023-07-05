#version 330 core
out vec4 frag_color;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};


in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
	vec2 frag_texCoord;
} fs_in;

uniform sampler2D texture_map;
uniform Material mat;
uniform int has_texture;

vec3 CalcDirLight(vec3 normal, vec3 view_dir)
{
    vec3 direction = vec3(4.0, 4.0, 4.0);
	vec3 light_dir = normalize(direction);

	// Ambient
	float amb = 0.8f;
    vec3 light_ambient = vec3(1.0, 1.0, 1.0);
	vec3 ambient = light_ambient * amb * mat.ambient;
	
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
    vec3 light_diffuse = vec3(0.8, 0.8, 0.8);
	vec3 diffuse = light_diffuse * diff * mat.diffuse;

	// Specular
	vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	vec3 h = normalize(light_dir + view_dir);
	float spec = pow(max(dot(view_dir, h), 0.0), mat.shininess);
    vec3 light_specular = vec3(0.5, 0.5, 0.5);	
	vec3 specular = light_specular * spec * mat.specular;

	return (ambient + diffuse + specular);
}

vec3 CalcDirLightTexture(vec3 normal, vec3 view_dir, vec3 color)
{	
    vec3 direction = vec3(4.0, 4.0, 4.0);
	vec3 light_dir = normalize(direction);

	// Ambient
    vec3 light_ambient = vec3(0.8, 0.8, 0.8);
	vec3 ambient = light_ambient * color;

	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
    vec3 light_diffuse = vec3(0.5, 0.5, 0.5);
	vec3 diffuse = light_diffuse * diff * color;

	// Specular
	vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), mat.shininess);	
    vec3 light_specular = vec3(1.0, 1.0, 1.0);
	vec3 specular = light_specular * spec * mat.specular;

	return (ambient + diffuse + specular);
}

void main()
{
	vec3 norm = normalize(fs_in.frag_norm);

    vec3 view_pos = vec3(0.0, 0.0, 4.0);
	vec3 view_dir = normalize(view_pos - fs_in.frag_pos);

    vec3 light_pos = vec3(3.0, 3.0, 3.0);
	vec3 light_dir = normalize(light_pos - fs_in.frag_pos);
	float cos_theta = dot(light_dir, norm);

	if(has_texture == 0)
	{
		// Texture map of outline
		vec3 result = CalcDirLight(norm, view_dir);
		frag_color = vec4(result, 1.0);

	}
	else
	{
		vec3 texture_color = vec3(texture(texture_map, fs_in.frag_texCoord));
		vec3 result = CalcDirLightTexture(norm, view_dir, texture_color);
		frag_color = vec4(result, 1.0);
	}
}