#version 410 core
out vec4 frag_color;

struct Material
{
	vec3 color;
    float metallic;
    float roughness;
};

struct Light
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

in VS_OUT
{
	vec3 frag_norm;
	vec2 frag_texCoord;
    vec4 frag_pos_light;
} fs_in;

in vec4 frag_pos;

uniform sampler2D shadow_map;
// uniform sampler2D texture_map;

uniform Material mat;
uniform Light light;
uniform vec3 view_pos;

float ShadowCalculation(vec4 light_space, float cos_theta)
{
	vec3 proj_coords = light_space.xyz / light_space.w; // Make homogenous coordinate to 1 by perspective divide
	proj_coords = proj_coords * 0.5 + 0.5; // Convert [-1, 1] => [0, 1]

	if(proj_coords.z > 1.0)
	{
		return 0.0;
	}

	float closest_depth = texture(shadow_map, proj_coords.xy).r;
	float current_depth = proj_coords.z;

	// If current depth is larger than closest depth
	// then it is in the shadow
	// The finite precision of floating point can cause the shadow acne
	// Need to add small float point to the closest depth
	float bias = mix(0.01, 0.005, cos_theta);
	float shadow = 0.0;
	vec2 offset = 1.0/textureSize(shadow_map, 0);
	for(int i=-1; i<=1; ++i)
	{
		for(int j=-1; j<=1; ++j)
		{
			float depth = texture(shadow_map, proj_coords.xy + vec2(i,j)*offset).r;
			shadow += (current_depth - depth) > bias ? 1.0 : 0.0;
		}
	}
	shadow /= 9.0;
	return shadow;
}

vec3 CalcDirLight(Light light, vec3 normal, vec3 view_dir, float shadow)
{
	vec3 light_dir = normalize(-light.direction);

	// Ambient
	float amb = 0.5f;
	vec3 ambient = light.ambient * amb * mat.color;
	
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * mat.color;

	// Specular
	vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	vec3 h = normalize(light_dir + view_dir);
	float spec = pow(max(dot(view_dir, h), 0.0), 64.0);	
	vec3 specular = light.specular * spec * mat.color;

	return (ambient + (1.0-shadow)*(diffuse + specular) );
}

void main()
{
    vec3 n = normalize(fs_in.frag_norm);
    vec3 v = normalize(view_pos-frag_pos.xyz);
    vec3 l = normalize(-light.direction);
    vec3 h = normalize(l+v);

    // float shadow = ShadowCalculation(fs_in.frag_pos_light, max(dot(l, n), 0.0));
    vec3 result = CalcDirLight(light, n, v, 0.0);
    frag_color = vec4(result, 1.0);
}