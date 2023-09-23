#version 330 core
// out vec4 frag_color;

in vec2 texCoords;

uniform sampler2D map;
uniform mat4 projection;
uniform vec2 screen_size;
uniform float dt;
const float threshold = 8.0;

float dz(vec2 uv, vec2 dir)
{
    // First derivative three-point midpoint

    float a = texture(map, uv + dir).r;
    float b = texture(map, uv - dir).r;
    float h = max(dir.x, dir.y);
    
    return (a-b)/(2*h);
}

float ddz(vec2 uv, vec2 dir)
{
    // Second derivative midpoint

    float a = texture(map, uv + dir).r;
    float b = texture(map, uv).r;
    float c = texture(map, uv - dir).r;
    float h = max(dir.x, dir.y);

    return (a - 2*b + c);
}

float ddz_partial(vec2 uv, vec2 dir)
{
    // Partial numerical differentiation

    float a = texture(map, uv + dir).r;
    float b = texture(map, uv + vec2(-dir.x, dir.y)).r;
    float c = texture(map, uv + vec2(dir.x, -dir.y)).r;
    float d = texture(map, uv - dir).r;

    float hx = dir.x;
    float hy = dir.y;

    return (a - b - c + d) / 4.0;
}

void main()
{
    vec2 texel = 1.0 / screen_size;
    float z = texture(map, texCoords).r;

	vec2 uv_x1 = texCoords - vec2(texel.x, 0.0);
	vec2 uv_x2 = texCoords + vec2(texel.x, 0.0);
	vec2 uv_y1 = texCoords - vec2(0.0, texel.y);
	vec2 uv_y2 = texCoords + vec2(0.0, texel.y);

    float left = texture(map, uv_x1).r;
    float right = texture(map, uv_x2).r;
    float bottom = texture(map, uv_y1).r;
    float top = texture(map, uv_y2).r;

    if(abs(z-right) > threshold || abs(z-left) > threshold ||
        abs(z-top) > threshold || abs(z-bottom) > threshold)
    {
        gl_FragDepth = z;
        return;
    }

    float dz_x = dz(texCoords, vec2(texel.x, 0.0));
    float dz_y = dz(texCoords, vec2(0.0, texel.y));

    float ddz_x = ddz(texCoords, vec2(texel.x, 0.0));
    float ddz_y = ddz(texCoords, vec2(0.0, texel.y));
    float ddz_xy = ddz_partial(texCoords, texel);

    float Fx = projection[0][0];
    float Fy = projection[1][1];
    float Vx = screen_size.x;
    float Vy = screen_size.y;

    float Cx = 2.0 / (Fx*Vx);
    float Cy = 2.0 / (Fy*Vy);
    float Cx2 = Cx*Cx;
    float Cy2 = Cy*Cy;

    float D = Cy2 * dz_x * dz_x + Cx2 * dz_y * dz_y + Cx2 * Cy2 * z * z;
    float dD_x = 2.0*Cy2*dz_x*ddz_x + 2.0*Cx2*dz_y*ddz_xy + 2.0*Cx2*Cy2*z*dz_x;
    float dD_y = 2.0*Cy2*dz_x*ddz_xy + 2.0*Cx2*dz_y*ddz_y + 2.0*Cx2*Cy2*z*dz_y;

    float Ex = 0.5*dz_x*dD_x - ddz_x*D;
    float Ey = 0.5*dz_y*dD_y - ddz_y*D;

    float H = (Cy*Ex + Cx*Ey) / (2*D*sqrt(D));

    gl_FragDepth = z + H * dt;
    // frag_color = vec4(z, 0.0, 0.0, 1.0);
}