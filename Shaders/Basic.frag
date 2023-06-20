#version 330 core
out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;

struct DirLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform DirLight dir_light;
uniform vec3 object_color;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 light_dir)
{
	// Ambient
	float amb = 0.2f;
	vec3 ambient = light.ambient * amb;

	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff;

	// Specular
	// vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	// float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);	
	// vec3 specular = light.specular * spec;

	return (ambient + diffuse);
}


void main()
{
	// Phong Lighting Model -> light Intensity * (Specular + diffuse + ambient)
	vec3 norm = normalize(frag_norm);
	vec3 light_dir = normalize(-dir_light.direction);
	vec3 result = CalcDirLight(dir_light, norm, light_dir) * object_color;

	frag_color = vec4(result, 1.0);
}