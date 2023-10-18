#version 450 core
out vec4 frag_color;

uniform sampler2D outline_map;
uniform sampler2D mask_map;
uniform float jump;
uniform float width;
uniform float height;
uniform int pass;

in vec4 frag_frame_proj;
in vec2 frag_texCoords;

float grayScale(vec3 color)
{
    return (0.2126 * color.r + 0.7152 * color.g + 0.0722 * color.b);
}

float getOutline(vec3 proj_coords, float jump)
{
	float Gx[9] = float[](
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );

    float Gy[9] = float[](
        -1,-2,-1,
         0, 0, 0,
         1, 2, 1
    );
    
    float gx = 0.0;
    float gy = 0.0;

    float w = jump/width;
    float h = jump/height;

    for(int y=0; y<3; ++y)
	{
		for(int x=0; x<3; ++x)
		{
			float color = grayScale(texture2D(outline_map, frag_texCoords.xy + vec2( (x-1)*w, (y-1)*h )).rgb);
            gx += Gx[x + y*3] * color;
            gy += Gy[x + y*3] * color;
		}
	}

    float magnitude = sqrt(pow(gx, 2.0)+pow(gy, 2.0));
	return magnitude;
}

float getFlood(vec3 proj_coords, float jump)
{
    float w = jump/width;
    float h = jump/height;
    float curr_color = 0.0;
    for(int y=-1; y<=1; ++y)
	{
		for(int x=-1; x<=1; ++x)
		{
            vec2 offset = frag_texCoords.xy + vec2( x*w, y*h );
			float color_offset = texture2D(outline_map, offset).r;
            if( curr_color < color_offset)
            {
                curr_color = color_offset;
            }
		}
	}

    return curr_color;
}

float blur(vec3 proj_coords)
{
    float w = 1.0/width;
    float h = 1.0/height;

    float sum = 0.0;
    for(int y=-1; y<=1; ++y)
	{
		for(int x=-1; x<=1; ++x)
		{
            vec2 offset = frag_texCoords.xy + vec2( x*w, y*h );
			float color_offset = texture2D(outline_map, offset).r;
            sum += color_offset;
		}
	}

    sum /= 9.0;

    return clamp(sum, 0.0, 1.0);
}


void main()
{
    vec3 proj_coords = frag_frame_proj.xyz / frag_frame_proj.w;
    proj_coords = proj_coords * 0.5 + 0.5; 

    if(pass == 0)
    {
        float g = getOutline(proj_coords, jump);
        if(g > 0.5)
        {
            frag_color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(0.8, 0.6, 0.1, 1.0);
        }
        else
        {
            frag_color = vec4(0.0);
        }
    }
    else if(pass == 1)
    {
        float g = getFlood(proj_coords, jump);
        frag_color = vec4(g, 0.0, 0.0, 1.0);
    }
    else if(pass == 2)
    {
        vec3 color = (texture2D(mask_map, frag_texCoords.xy).rgb);
        if(color == vec3(1.0))
        {
            frag_color = vec4(0.0);
        }
        else
        {
            float g = blur(proj_coords);
            if(g < 0.1)
                frag_color = vec4(0.0);
            else
                frag_color = vec4(0.8, 0.6, 0.1, 1.0);
        }
    }
    else
    {
        frag_color = vec4(0.0,0.0,0.0, 1.0);
    }
}