#version 460 core

in vec2 texCoords;
uniform sampler2D map;
uniform vec2 dir;

void main()
{
    float depth = texture(map, texCoords).r;

    if(depth <= 0.0)
    {
        depth = 0.0; //vec4(vec3(0.0), 1.0);
        return;
    }
    
    if (1.0 <= depth) 
    {
		gl_FragDepth = depth; //vec4(vec3(depth), 1.0);
		return;
	}

    float sum = 0.0;
	float wsum = 0.0;

	for (float x = -5; x <= 5; x += 1.0) 
    {
		float s = texture(map, texCoords + x * dir).r;

		if (1.0 <= s)
        {
            continue;
        }

		float r = x * 0.1;
		float w = exp(-r*r);
		
		float r2 = (s - depth) * 65.0;
		float g = exp(-r2*r2);
		
		sum += s * w * g;
		wsum += w * g;
	}

    if(wsum > 0.0)
    {
        sum /= wsum;
    }

    gl_FragDepth = sum; //vec4(vec3(sum), 1.0);
}