#version 450 core

out vec4 frag_color;

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

uniform sampler2D shadow_map;
uniform sampler2D texture_map;

uniform Material mat;
uniform Light light;
uniform vec3 view_pos;
uniform vec3 hit_pos;
uniform float brush_size;
uniform int has_texture;

in vec3 normal;
in vec3 frag_pos;
in vec4 frag_pos_light;
in vec2 frag_texCoords;

float getShadow(vec4 light_space, float cos_theta)
{
	vec3 proj_coords = light_space.xyz / light_space.w; // Make homogenous coordinate to 1 by perspective division
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


void main()
{
    vec3 l = normalize(-light.direction);
    vec3 n = normal;
    vec3 v = normalize(view_pos - frag_pos);
    vec3 h = normalize(l+v);
    float shadow = getShadow(frag_pos_light, max(dot(l, n), 0.0));

	vec3 color;
	if(has_texture == 1)
	{
		color = vec3(texture(texture_map, frag_texCoords));
	}
	else
	{
		color = mat.color;
	}

    vec3 ambient = light.ambient * 0.5 * color;
    vec3 diffuse = light.diffuse * max(dot(n, l), 0.0) * color;
    vec3 specular = light.specular * pow(max(dot(v, h), 0.0), 32.0) * color;
    vec3 final_color = ambient + (1.0-shadow)*(diffuse + specular);

	vec3 diff = frag_pos - hit_pos;
	float r2 = dot(diff, diff);

	if(r2 < brush_size)
	{
		frag_color = vec4( (1.0-r2) * 0.4, 0.0, 0.0, 0.5);
	}
	else
	{
    	frag_color = vec4(final_color, 1.0);
	}

}