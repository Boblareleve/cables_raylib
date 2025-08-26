#version 410 core
layout (location = 0) in vec2 chunk_pos;

// uniform usamplerBuffer chunks; // TBO binding
uniform vec2 campos;
uniform float camzoom;
uniform float ratio;

out uint v_index;
out uint v_part;

void main()
{
	v_index = gl_VertexID / 4;
	v_part  = gl_VertexID % 4;

	vec2 cam_ratio_zoom = camzoom / vec2(ratio, 1.0);
    vec2 chunk_pos_screen = cam_ratio_zoom * (chunk_pos - vec2(campos.x, -campos.y));
	gl_Position = vec4(chunk_pos_screen, 0.0, 1.0);
}
