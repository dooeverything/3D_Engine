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

in vec3 fragPos;
in vec3 normal;
in vec2 texCoord;
flat in ivec4 boneIDs;
in vec4 weights;
in vec3 posePos;

uniform Material mat;
uniform Texture tex;
uniform vec3 viewPos;
uniform DirLight dirLight;

uniform int displayBoneIndex;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir =	normalize(-light.direction);

	// Ambient
	vec3 textureColor = vec3(texture(tex.texture_diffuse1, texCoord));
	vec3 ambient = light.ambient * textureColor;
	
	// Diffuse
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * diff * textureColor;

	// Specular
	vec3 reflectDir = 2 * dot(normal, lightDir) * normal - lightDir;
	float specFactor = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);	
	vec3 specular = light.specular * specFactor;

	return (ambient + diffuse + specular);
}

void main()
{
	bool found = false;

	if(!found)
	{
		vec3 norm = normalize(normal);
		vec3 viewDir = normalize(viewPos - fragPos);
		
		// Directional Light
		vec3 result = CalcDirLight(dirLight, norm, viewDir);
		fragColor = vec4(result, 1.0);
	}
}