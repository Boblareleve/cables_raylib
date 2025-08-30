#version 410 core
layout (location = 0) in vec4  box;
layout (location = 1) in vec4  fill_color;
layout (location = 2) in vec4  border_color;
layout (location = 3) in float border_thickness;
layout (location = 4) in uint  texture_index;


uniform vec2 campos;
uniform float camzoom;
uniform float ratio;
// uniform float window_width;
// uniform float window_height;
// /* const */ vec2 window_dim = vec2(window_width, window_height);


out VS_OUT {
    vec2  dim; // pos pass through gl_Position
    vec4  fill_color;
    vec4  border_color;
    float border_thickness;
    uint  texture_index;
} vs_out;

void main()
{
	vec2 cam_ratio_zoom = camzoom / vec2(ratio, 1.0);
    vec2 chunk_pos_screen = cam_ratio_zoom * (box.xy - vec2(campos.x, -campos.y));
	gl_Position = vec4(chunk_pos_screen, 0.0, 1.0);
    vs_out.dim = cam_ratio_zoom * box.zw;

    // gl_Position = vec4((box.xy / window_dim) * 2.0 - 1.0, 0.0, 1.0);
    // vs_out.dim = (box.zw / window_dim) * 2.0 - 1.0;

    vs_out.fill_color = fill_color;
    vs_out.border_color = border_color;
    vs_out.border_thickness = border_thickness;
    vs_out.texture_index = texture_index;
}


