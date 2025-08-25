#version 410 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;


in VS_OUT {
    vec4 color;
    uint texture_index;
    uint type;
    vec2 dim;
} gs_in[];

out GS_OUT {
    vec2 TexCoord;
    uint type;
    uint texture_index;
    vec4 color;
} gs_out;


void rect_vertex(vec2 pos, vec2 dim)
{
    gl_Position = pos + dim;

    gs_out.TexCoord        = dim;
    gs_out.color           = gs_in[0].color;
    gs_out.texture_index   = gs_in[0].color;
    gs_out.type            = gs_in[0].type;
    EmitVertex();
}

void rect_primitive(vec2 pos, vec2 dim)
{
    rect_vertex(pos, vec2(0));
    rect_vertex(pos, vec2(dim.x, 0));
    rect_vertex(pos, vec2(dim));
    rect_vertex(pos, vec2(0, dim.y));
    EndPrimitive();
}

void main()
{
    rect_primitive(gl_in[0].gl_Position, gs_in[0].dim);
}
