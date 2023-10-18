#version 450 core
out vec4 frag_color;

in vec2 texCoords;

uniform mat4 projection;
uniform mat4 view;

uniform sampler2D map;
uniform samplerCube cubemap;
uniform vec2 inverse_tex;

const vec3 light_dir = vec3(-1.0, -1.0, -1.0);
const vec3 light_pos = vec3(0.0, 1000.0, 0.0);
// const float shininess = 1000.0;
const float fres_power = 5.0;
const float fres_scale = 0.9;
const float fres_bias = 0.1;
const vec4 base_color = vec4(0.275, 0.65, 0.85, 0.9);

vec3 uvToView(vec2 uv) 
{
	float x = uv.x * 2.0 - 1.0;
	float y = uv.y * 2.0 - 1.0;
	float z = texture(map, uv).r;
	vec4 clip = vec4(x, y, z, 1.0);
	vec4 view = inverse(projection) * clip;
	return view.xyz / view.w;
}

float fresnel(vec3 a, vec3 b)
{
	float f = fres_bias + fres_scale * pow( 1.0f - max(dot(a, b), 0.0), fres_power);
	return f;
}

void main() 
{
	vec2 texel = inverse_tex; // vec2(0.001, 0.001); //1.0 / (screen_size);
    float depth = texture(map, texCoords).x;

	if (depth == 0.0) 
	{
		frag_color = vec4(0.0);
		return;
	}

	if (depth == 1.0) 
	{
		frag_color = vec4(1.0);
		return;
	}

	vec3 view_pos = uvToView(texCoords);
	vec4 world_pos = inverse(view) * vec4(view_pos, 1.0);

	vec2 uv_x1 = texCoords - vec2(texel.x, 0.0);
	vec2 uv_x2 = texCoords + vec2(texel.x, 0.0);
	vec3 ddx1 = view_pos - uvToView(uv_x1);
	vec3 ddx2 = uvToView(uv_x2) - view_pos;
	if (abs(ddx2.z) < abs(ddx1.z))
		ddx1 = ddx2;
	
	vec2 uv_y1 = texCoords - vec2(0.0, texel.y);
	vec2 uv_y2 = texCoords + vec2(0.0, texel.y);
	vec3 ddy1 = view_pos - uvToView(uv_y1);
    vec3 ddy2 = uvToView(uv_y2) - view_pos;
	if (abs(ddy2.z) < abs(ddy1.z))
		ddy1 = ddy2;
	
	vec3 n =  normalize(cross(ddx1, ddy1));
    vec3 l =  normalize(-light_dir);
    vec3 v = -normalize(view_pos);
	float n_dot_l = dot(n, l)*0.5 + 0.5;
    // frag_color = vec4(n, 1.0);
    
    float amb = 0.5f;
    vec3 ambient = vec3(1.0, 1.0, 1.0) * amb * base_color.xyz;

    float diff = max(n_dot_l, 0.0);
    // vec3 diffuse = vec3(0.8, 0.8, 0.8) * diff * vec3(0.65, 0.85, 0.90);
	vec3 diffuse = base_color.xyz * mix(vec3(0.29, 0.379, 0.59), vec3(1.0), n_dot_l) * (1 - base_color.w);

    // vec3 reflect_dir = 2 * n_dot_l * n - l;
    vec3 h = normalize(l + v);
	float spec = pow(max(dot(v, h), 0.0), 128.0);	
    vec3 specular = vec3(0.5, 0.5, 0.5) * spec;// * base_color.xyz;

	// vec3 color = ambient + diffuse + specular;
	
	float ratio = 1.00/1.52;
	vec3 T =  refract(-v, normalize(n), ratio);
	vec3 refract_color = texture(cubemap, T).rgb;


	vec3 sky = vec3(0.1, 0.2, 0.4)*1.2;
	vec3 ground = vec3(0.1, 0.1, 0.2);
	vec3 view_reflect = reflect(v, n);
	vec3 world_reflect = (inverse(view) * vec4(view_reflect, 0.0)).xyz;
	vec3 reflect_color = vec3(1.0) + mix(ground, sky, smoothstep(0.15, 0.25, world_reflect.y));

	// float fresnel = fres_bias + fres_scale * pow( 1.0f - max(dot(n, v), 0.0), fres_power);
	vec3 color = refract_color*(1 - fresnel(n, v)) + reflect_color*fresnel(n, v) + specular; //diffuse + (mix(refract_color, reflect_color, fresnel) + spec) * base_color.w;

    frag_color = vec4(color, 1.0);
}