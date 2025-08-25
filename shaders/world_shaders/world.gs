#version 410 core
layout (points) in;
layout (triangle_strip, max_vertices = 1000) out;



in uint v_index[];
in uint v_part[];
uniform usamplerBuffer chunks; // TBO binding

out vec4 color;
out vec2 TexCoord;

uniform vec2 campos;
uniform float camzoom;
uniform float width;
uniform float height;
uniform float ratio;

const int chunk_width = 16;
const int chunk_height = 16;
const float atlas_width = 4096;
const float cell_size = 16.0;
const int part_count = 4;
vec2 sss = vec2(camzoom / ratio, camzoom) / 16;

vec2 get_tex_pos_dl(int cell)
{
    return vec2((0. + floor(cell)) * cell_size / atlas_width, 0);
}
vec2 get_tex_pos_dr(int cell)
{
    return vec2((1. + floor(cell)) * cell_size / atlas_width, 0);
}
vec2 get_tex_pos_ul(int cell)
{
    return vec2((0. + floor(cell)) * cell_size / atlas_width, 1);
}
vec2 get_tex_pos_ur(int cell)
{
    return vec2((1. + (cell)) * cell_size / atlas_width, 1);
}

void cell_emit_primitive(int cell, vec4 pos)
{
    color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_dl(cell);
    EmitVertex();

    color = vec4(1.0, 1.0, 0.0, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_dr(cell);
    EmitVertex();

    color = vec4(1.0, 0.0, 1.0, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 1.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_ul(cell);
    EmitVertex();

    color = vec4(1.0, 1.0, 1.0, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 1.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_ur(cell);
    EmitVertex();

    EndPrimitive();
}

void for_part(int sx, int ex, int sy, int ey)
{
    vec4 pos = gl_in[0].gl_Position;
    int cell = 16;

    for (int y = sy; y < ey; y++)
    {
        for (int x = sx; x < ex; x++)
        {
            cell_emit_primitive(cell, pos + vec4(sss * vec2(x, y), 0, 0));
            // cell_emit_primitive(cell, pos + vec4(float(x), float(y), 0, 0));
        }
    }
}

void main()
{
    color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = vec4(0.0);
    TexCoord = vec2(0);
    EmitVertex();
    
    color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = vec4(1.0, 0.0, vec2(0.0));
    TexCoord = vec2(0);
    EmitVertex();
    
    color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = vec4(0.0, 1.0, vec2(0.0));
    TexCoord = vec2(0);
    EmitVertex();
    
    color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = vec4(1.0, 1.0, vec2(0.0));
    TexCoord = vec2(0);
    EmitVertex();
    EndPrimitive();

    return ;
    

    // cell_emit_primitive(cell, pos);
    if (false)
    {
        switch (v_part[0])
        {
        case 0: for_part(chunk_width*0/2, chunk_width*1/2, chunk_height*0/2, chunk_height*1/2);
        case 2: for_part(chunk_width*0/2, chunk_width*1/2, chunk_height*1/2, chunk_height*2/2);
        case 1: for_part(chunk_width*1/2, chunk_width*2/2, chunk_height*0/2, chunk_height*1/2);
        case 3: for_part(chunk_width*1/2, chunk_width*2/2, chunk_height*1/2, chunk_height*2/2);
        }
    }
    else 
    {
        vec4 pos = gl_in[0].gl_Position;
        int cell = 16;

        for (int y = 0; y < 16; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                cell_emit_primitive(cell, pos + vec4(sss * vec2(x, y), 0, 0));
                // cell_emit_primitive(cell, pos + vec4(float(x), float(y), 0, 0));
            }
        }
    }

    // float tex_pos_left  =       cell  * 16. / 4096.;
    // float tex_pos_right = (1. + cell) * 16. / 4096.;
    // float tex_pos_down  = 0;
    // float tex_pos_up    = 1;
}
