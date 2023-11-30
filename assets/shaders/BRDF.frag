// References:
// 1. https://learnopengl.com/PBR/IBL/Specular-IBL
// 2. https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// 3. http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html

#version 450 core
out vec4 frag_color;

struct Material
{
	vec3 color;
    float metallic;
    float roughness;
	float ao;
};

in VS_OUT
{
	vec3 frag_pos;
	vec3 frag_norm;
	vec2 frag_texCoord;
    vec4 frag_pos_light;
} fs_in;

struct Light
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct TextureFBX
{
	sampler2D color1;
};

uniform sampler2D texture_map;
uniform sampler2D shadow_map;
uniform samplerCube irradiance_map;
uniform samplerCube prefilter_map;
uniform sampler2D lut_map;

uniform Material mat;
uniform TextureFBX tex_fbx;
uniform vec3 view_pos;
uniform vec3 light_pos;
uniform Light light;
uniform int type;

#define M_PI 3.1415926535897932384626433832795

float normalDistribution(float cos_theta)
{
    float a = pow(mat.roughness, 4);
    float d = pow(cos_theta,2) * (a-1) + 1;
    return a/(M_PI * pow(d,2));
}

float geometry(float cos_theta)
{
    float k = pow((mat.roughness+1.0),2) / 8.0;
    float d = cos_theta*(1-k) + k;
    return cos_theta / d;
}

vec3 fernel(float cos_theta)
{
    vec3 F0 = mix(vec3(0.04), mat.color, mat.metallic);
    return F0 + (1-F0)*pow(clamp(1.0-cos_theta, 0.0, 1.0), 5);
}

vec3 fernelRoughness(float cos_theta)
{
    vec3 F0 = mix(vec3(0.04), mat.color, mat.metallic);
	return F0 + (max(vec3(1.0-mat.roughness), F0)-F0) * pow(clamp(1.0-cos_theta, 0.0, 1.0), 5);
}

float ShadowCalculation(vec4 light_space, float cos_theta)
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

vec3 CalcDirLight(Light light, vec3 normal, vec3 view_dir)
{
	vec3 light_dir = normalize(-light.direction);

	// Ambient
	float amb = 0.5f;
	vec3 ambient = light.ambient * amb * mat.color;
	
	// Diffuse
	float diff = max(dot(normal, light_dir), 0.0);
	vec3 diffuse = light.diffuse * diff * mat.color;

	// Specular
	//vec3 reflect_dir = 2 * dot(normal, light_dir) * normal - light_dir;
	vec3 h = normalize(light_dir + view_dir);
	float spec = pow(max(dot(view_dir, h), 0.0), 32.0);	
	vec3 specular = light.specular * spec * mat.color;

	return (ambient + diffuse + specular);
}

void main()
{
    vec3 l = normalize(-light.direction);
    vec3 n = normalize(fs_in.frag_norm);
    vec3 v = normalize(view_pos-fs_in.frag_pos);
    vec3 h = normalize(l+v);
	vec3 r = normalize(2 * dot(v, n) * n - v);
	vec3 color = vec3(0.0);
	float shadow = ShadowCalculation(fs_in.frag_pos_light, max(dot(l, n), 0.0));

	if(type == 0)
	{
		vec3 F = fernel(max(dot(n,v),0.0)); // Reflection
		float G = geometry(max(dot(n,l),0.0)) * geometry(max(dot(n,v),0.0));
		float D = normalDistribution(max(dot(n,h),0.0));
		vec3 specular2 = (F*G*D) / (4.0*max(dot(n,v),0.0)*max(dot(n,l),0.0)+0.0001);
		vec3 kS = F;
		vec3 kD = vec3(1.0)-kS;
		kD *= 1.0-mat.metallic;
		vec3 diffuse2 = kD*mat.color/M_PI;
		vec3 second_sum = (diffuse2 + specular2) * light.ambient * max(dot(l, n), 0.0);

		F = fernelRoughness(max(dot(n,v),0.0));
		kS = F;
		kD = 1.0-kS;
		kD *= 1.0 - mat.metallic;
		vec3 diffuse1 = texture(irradiance_map, n).rgb * mat.color;
		vec3 prefilter_color = textureLod(prefilter_map, r, mat.roughness*4.0).rgb;
		vec2 lut_color = texture(lut_map, vec2(max(dot(n,v),0.0), mat.roughness)).rg;
		vec3 specular1 = prefilter_color * (F*lut_color.x + lut_color.y);
		vec3 first_sum = (kD * (1.0-shadow) * diffuse1 + specular1) * mat.ao;
		
		color = (first_sum + second_sum);
		color = color / (color + vec3(1.0)); // HDR
		color = pow(color, vec3(1.0/2.2)); // Gamma correction
	} 
	else if(type == 1)
	{
		vec3 texture_color = vec3(texture(texture_map, fs_in.frag_texCoord));
		vec3 ambient = vec3(1.0) * texture_color;
		vec3 diffuse = light.diffuse * max(dot(n, l), 0.0) * texture_color;
		vec3 specular = light.specular * pow(max(dot(v, h), 0.0), 32.0) * vec3(0.5);
		color = ambient + (1.0-shadow)*(diffuse+specular);
	}
	else if(type == 2)
	{
		vec3 texture_color = vec3(texture(tex_fbx.color1, fs_in.frag_texCoord));
		vec3 ambient = vec3(1.0) * texture_color;
		vec3 diffuse = light.diffuse * max(dot(n, l), 0.0) * texture_color;
		vec3 specular = light.specular * pow(max(dot(v, h), 0.0), 32.0) * vec3(0.5);
		color = ambient + (1.0-shadow)*(diffuse+specular);
	}
	else if(type == 3)
	{
		color = CalcDirLight(light, n, v);
	}

    frag_color = vec4(color, 1.0);
}  