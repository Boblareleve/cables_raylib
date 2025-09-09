#version 410 core
layout (location = 0) in vec4  box;
layout (location = 1) in vec4  fill_color;
layout (location = 2) in vec4  border_color;
layout (location = 3) in vec4  on_hover_filter;
layout (location = 4) in float border_thickness;
layout (location = 5) in float texture_index;
 

// uniform vec2 campos;
// uniform float camzoom;
// uniform float ratio;
uniform float window_width;
uniform float window_height;
vec2 window_dim = vec2(window_width, window_height);


out VS_OUT {
    vec2  dim; // pos pass through gl_Position
    vec4  fill_color;
    vec4  border_color;
    vec4  on_hover_filter;
    float border_thickness;
    float texture_index;
} vs_out;

void main()
{
	// vec2 cam_ratio_zoom = camzoom / vec2(ratio, 1.0);
    // vec2 chunk_pos_screen = cam_ratio_zoom * (box.xy - vec2(campos.x, -campos.y));
	// gl_Position = vec4(chunk_pos_screen, 0.0, 1.0);
    // vs_out.dim = cam_ratio_zoom * box.zw;

    vec2 pos_px = box.xy;
    vec2 dim_px = vec2(box.z, -box.w);

    gl_Position = vec4(
        vec2(pos_px / window_dim * 2.0 - 1.0),
        0.0, 1.0
    );
    gl_Position.y = -gl_Position.y;
    
    vs_out.dim = (dim_px / window_dim) * 2.0;
    
    vs_out.border_thickness = border_thickness / abs(dim_px.x);
            // / window_dim.x * 2.0;

    vs_out.border_color    = border_color;
    vs_out.on_hover_filter = on_hover_filter;
    vs_out.fill_color      = fill_color;
    vs_out.texture_index   = texture_index;
}
