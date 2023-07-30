// Reference: 3D Graphics Rendering Cookbook, Chapter5 
// https://github.com/PacktPublishing/3D-Graphics-Rendering-Cookbook/tree/master/Chapter5/GL01_Grid

#version 330 core
out vec4 frag_color;

in vec3 frag_pos;
in vec3 frag_norm;
in vec2 out_cam;
in vec2 out_uv;

vec2 clampVec2(vec2 v)
{
	return clamp(v, vec2(0.0), vec2(1.0));
}

vec4 gridColor(vec2 uv, vec2 cam_pos)
{
	float grid_size = 100.0;
	float grid_cell_size = 1.0;
	vec4 grid_thin = vec4(0.5, 0.5, 0.5, 1.0);

	vec2 dudv = vec2( length(vec2(dFdx(uv.x), dFdy(uv.x))), 
					  length(vec2(dFdx(uv.y), dFdy(uv.y))) );

	float level = (length(dudv) * 2.0) / grid_cell_size; 
	level = max(0.0, log(level)/log(10.0) + 1.0 );
	float fade = fract(level);

	// How many pixel for each level, ex) if level = 1.xxx, then floor(level) => 1
	float lod0 = grid_cell_size * pow(10.0, floor(level)); 
	float lod1 = 10 * lod0;
	dudv *= 2.0;

	vec2 dist = vec2(1.0) - abs(clampVec2(mod(uv, lod0)/dudv) * 2.0 - vec2(1.0));
	float lod0_alpha = max(dist.x, dist.y);
	dist = vec2(1.0) - abs(clampVec2(mod(uv, lod1)/dudv) * 2.0 - vec2(1.0));
	float lod1_alpha = max(dist.x, dist.y);
	vec4 color = grid_thin;
	
	uv -= cam_pos;
	float fade_out = (1.0 - clamp(length(uv)/grid_size, 0.0, 1.0));
	color.a = lod1_alpha > 0.0 ? lod1_alpha : (lod0_alpha * (1.0-fade)) * fade_out;

	return color;
}

void main()
{
	if(out_uv.x < 0.05 && out_uv.x > -0.05)
	{
		frag_color = vec4(0.0, 0.0, 1.0, 1.0);
	}
	else if(out_uv.y < 0.05 && out_uv.y > -0.05)
	{
		frag_color = vec4(1.0, 0.0, 0.0, 1.0);
	}
	else
	{
		frag_color = gridColor(out_uv, out_cam);
	}
}