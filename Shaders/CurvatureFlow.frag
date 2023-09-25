#version 460 core

in vec2 texCoords;

uniform sampler2D depth_map;
uniform mat4 projection;
uniform vec2 res;

const float dt = 0.0005;
const float near = 0.1;
const float far = 100.0;
const float threshold = 4.0;

float dz(vec2 uv, vec2 dir)
{
    // First derivative three-point midpoint

    float a = texture(depth_map, uv + dir).r;
    float b = texture(depth_map, uv - dir).r;
    float h = max(dir.x, dir.y);
    
    return (a-b) * 0.5; // / h;
}

float ddz(vec2 uv, vec2 dir)
{
    // Second derivative midpoint

    float a = texture(depth_map, uv + dir).r;
    float b = texture(depth_map, uv).r;
    float c = texture(depth_map, uv - dir).r;
    float h = max(dir.x, dir.y);

    return (a - 2*b + c); // / (h*h);
}

float ddz_partial(vec2 uv, vec2 dir)
{
    // Partial numerical differentiation

    float a = texture(depth_map, uv + dir).r; // Top Right
    float b = texture(depth_map, uv + vec2(-dir.x, dir.y)).r; // Top Left
    float c = texture(depth_map, uv + vec2(dir.x, -dir.y)).r; // Bottom Right
    float d = texture(depth_map, uv - dir).r; // Bottom Left

    float hx = dir.x;
    float hy = dir.y;

    return (a - b - c + d) * 0.25; // / (hx * hy);
}

float linearizeDepth(float depth)
{
    const float ndc = 2.0 * depth - 1.0;
    return 2.0 * near * far / (far + near - ndc * (far - near));
}

void main()
{
    vec2 texel = 1.0 / res;
    float z = texture(depth_map, texCoords).r;

    if (z == 1.0)
    {
        gl_FragDepth = 1.0;
        return;
    }

    if (z == 0.0)
    {
        gl_FragDepth = 0.0;
        return;
    }

	vec2 uv_x1 = texCoords - vec2(texel.x, 0.0);
	vec2 uv_x2 = texCoords + vec2(texel.x, 0.0);
	vec2 uv_y1 = texCoords - vec2(0.0, texel.y);
	vec2 uv_y2 = texCoords + vec2(0.0, texel.y);

    float left = texture(depth_map, uv_x1).r;
    float right = texture(depth_map, uv_x2).r;
    float bottom = texture(depth_map, uv_y1).r;
    float top = texture(depth_map, uv_y2).r;

    float eye_z = linearizeDepth(z);
    float eye_z_l = linearizeDepth(left);
    float eye_z_r = linearizeDepth(right);
    float eye_z_b = linearizeDepth(bottom);
    float eye_z_t = linearizeDepth(top);

    vec4 z_diff = abs(eye_z - vec4(eye_z_l, eye_z_r, eye_z_b, eye_z_t));

    if (any(greaterThan(z_diff, vec4(threshold))))
    {
        gl_FragDepth = z;
        return;
    }

    // if(abs(z-right) > threshold || abs(z-left) > threshold ||
    //     abs(z-top) > threshold || abs(z-bottom) > threshold)
    // {
    //     gl_FragDepth = z;
    //     return;
    // }

    float dz_x = dz(texCoords, vec2(texel.x, 0.0));
    float dz_y = dz(texCoords, vec2(0.0, texel.y));

    float ddz_x = ddz(texCoords, vec2(texel.x, 0.0));
    float ddz_y = ddz(texCoords, vec2(0.0, texel.y));
    float ddz_xy = ddz_partial(texCoords, texel);

    float Fx = -projection[0][0];
    float Fy = -projection[1][1];
    float Vx = res.x;
    float Vy = res.y;

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
}