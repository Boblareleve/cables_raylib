#version 410 core
layout (location = 0) in vec2 chunk_pos;
layout (location = 1) in uint index;

out uint v_index;
out uint v_part;

// uniform usamplerBuffer chunks; // TBO binding
uniform vec2 campos;
uniform float camzoom;
uniform float ratio;

void main()
{
    // asssume glDrawArrays start at 0
    v_index = gl_VertexID / 4;
    v_part  = gl_VertexID % 4;
    v_part  = 0;

    vec2 chunk_pos_screen = camzoom / vec2(ratio, 1.0) * (chunk_pos - vec2(campos.x, -campos.y));
    
    gl_Position = vec4(chunk_pos_screen, 0.0, 1.0); 
}
