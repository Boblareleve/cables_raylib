#version 410 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;



in VS_OUT {
    vec2  dim; // pos pass through gl_Position
    vec4  fill_color;
    vec4  border_color;
    vec4  on_hover_filter;
    float border_thickness;
    float  texture_index;
} gs_in[];


out GS_OUT {
    vec2       tex_pos;
    flat vec4  fill_color; // use provoking vertex to optimize ? 
    flat vec4  border_color;
    flat vec4  on_hover_filter;
    flat float border_thickness;
    flat float texture_index;
} gs_out;


void rect_vertex(vec2 pos, vec2 dim, vec2 tex_pos)
{
    gl_Position = vec4(pos + dim, 0.0, 1.0);
    
    gs_out.tex_pos           = tex_pos;
    gs_out.fill_color        = gs_in[0].fill_color;
    gs_out.border_color      = gs_in[0].border_color;
    gs_out.on_hover_filter   = gs_in[0].on_hover_filter;
    gs_out.border_thickness  = gs_in[0].border_thickness;
    gs_out.texture_index     = gs_in[0].texture_index;
    EmitVertex();
}

void rect_primitive(vec2 pos, vec2 dim)
{
    rect_vertex(pos, vec2(0,     0    ), vec2(0,  0));
    rect_vertex(pos, vec2(dim.x, 0    ), vec2(1,  0));
    rect_vertex(pos, vec2(0,     dim.y), vec2(0, -1));
    rect_vertex(pos, vec2(dim.x, dim.y), vec2(1, -1));
    EndPrimitive();
}

void main()
{
    rect_primitive(gl_in[0].gl_Position.xy, gs_in[0].dim);
}
