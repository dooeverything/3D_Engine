#version 330 core
out vec4 fragColor;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct Texture
{
	sampler2D texture_diffuse1;
	sampler2D texture_reflection1;
};

struct DirLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 frag_texCoord;
flat in ivec4 boneIDs;
in vec4 weights;
in vec3 pose_pos;

uniform Material mat;
uniform Texture tex;
uniform vec3 view_pos;
uniform DirLight dir_light;

uniform int displayBoneIndex;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-light.direction);

	// Ambient
	vec3 texture_color = vec3(texture(tex.texture_diffuse1, frag_texCoord));
	vec3 ambient = light.ambient * texture_color;
	
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * texture_color;

	// Specular
	// vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	// float spec_factor = pow(max(dot(view_dir, reflect_dir), 0.0), 64.0);	
	// vec3 specular = light.specular * spec_factor;

	return (ambient + diffuse);
}

void main()
{
	bool found = false;

	if(!found)
	{
		vec3 norm = normalize(frag_norm);
		vec3 view_dir = normalize(view_pos - frag_pos);
		
		// Directional Light
		vec3 result = CalcDirLight(dir_light, norm, view_dir);
		fragColor = vec4(result, 1.0);
	}
}