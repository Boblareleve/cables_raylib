#version 410 core
layout (points) in;
layout (triangle_strip, max_vertices = 256) out;


in int v_index[];
in int v_part[];

uniform usamplerBuffer chunks; // TBO binding

// out vec4 color;
out vec2 TexCoord;

uniform vec2 campos;
uniform float camzoom;
// uniform float width;
// uniform float height;
uniform float ratio;

const int chunk_width = 16;
const int chunk_height = 16;
const float atlas_width = 4096;
const float cell_size = 16.0;
const int part_count = 4;
vec2 sss = vec2(camzoom / ratio, camzoom) / chunk_width;

const float tex_iner_padding = 1. / 16.; // 1 tex px

vec2 get_tex_pos_dl(uint cell) {
    return vec2((0. + floor(cell)) * cell_size / atlas_width, tex_iner_padding );
}
vec2 get_tex_pos_dr(uint cell) {
    return vec2((1. + floor(cell)) * cell_size / atlas_width, tex_iner_padding);
}
vec2 get_tex_pos_ul(uint cell) {
    return vec2((0. + floor(cell)) * cell_size / atlas_width, 1.0 - tex_iner_padding);
}
vec2 get_tex_pos_ur(uint cell) {
    return vec2((1. + (cell)) * cell_size / atlas_width, 1.0 - tex_iner_padding);
}
/* vec2 get_tex_pos_dl(uint cell) {
    return vec2((0. + floor(cell)) * cell_size / atlas_width, 0);
}
vec2 get_tex_pos_dr(uint cell) {
    return vec2((1. + floor(cell)) * cell_size / atlas_width, 0);
}
vec2 get_tex_pos_ul(uint cell) {
    return vec2((0. + floor(cell)) * cell_size / atlas_width, 1);
}
vec2 get_tex_pos_ur(uint cell) {
    return vec2((1. + (cell)) * cell_size / atlas_width, 1);
} */

void empty_emit(vec4 pos) {
    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = vec2(0);
    EmitVertex();

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = vec2(0);
    EmitVertex();

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 1.0) * sss, 0.0, 0.0);
    TexCoord = vec2(0);
    EmitVertex();

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 1.0) * sss, 0.0, 0.0);
    TexCoord = vec2(0);
    EmitVertex();

    EndPrimitive();
}

void cell_emit_primitive(uint cell, vec4 pos) {
    if (cell == 0)
    {
        // empty_emit(pos);
        return ;
    }
    

    // color = vec4(1.0, 0.0, 0.0, 1.0); //vec4(1.0, 0.0, 0.0, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_dl(cell);
    EmitVertex();

    // color = vec4(1.0, 1.0, 0.0, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_dr(cell);
    EmitVertex();

    // color = vec4(1.0, 0.0, 1.0, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 1.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_ul(cell);
    EmitVertex();

    // color = vec4(1.0, 1.0, 1.0, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 1.0) * sss, 0.0, 0.0);
    TexCoord = get_tex_pos_ur(cell);
    EmitVertex();

    EndPrimitive();
}

void debug_emit() {
    vec4 pos = gl_in[0].gl_Position;

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 0.0) * sss, 0.0, 0.0);
    TexCoord = vec2(0, 0);
    EmitVertex();

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 0.0) * sss * 16, 0.0, 0.0);
    TexCoord = vec2(1, 0);
    EmitVertex();

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(0.0, 1.0) * sss * 16, 0.0, 0.0);
    TexCoord = vec2(0, 1);
    EmitVertex();

    // color = vec4(0.30, 0.0, 0.45, 1.0);
    gl_Position = pos + vec4(vec2(1.0, 1.0) * sss * 16, 0.0, 0.0);
    TexCoord = vec2(1, 1);
    EmitVertex();

    EndPrimitive();
}


void for_part(int sx, int ex, int sy, int ey) {

    vec4 pos = gl_in[0].gl_Position;

    for (int x = sx; x < ex; x++)
        for (int y = sy; y < ey; y++)
        {
            uint cell = 0; //16;
            // if (x%2 == 0) cell = 32; // (y * 16 + x) % (80) + 16;
            // else 
            cell = texelFetch(chunks, int(v_index[0]) * 16*16 + x * 16 + y).x;
            

            cell_emit_primitive(cell, pos + vec4(sss * vec2(x, y), 0, 0));
        }
}



void main()
{
    if (true)
    {
        if (v_part[0] == 0)
            for_part(chunk_width*0/2, chunk_width*1/2, chunk_height*0/2, chunk_height*1/2);
        else if (v_part[0] == 1)
            for_part(chunk_width*0/2, chunk_width*1/2, chunk_height*1/2, chunk_height*2/2);
        else if (v_part[0] == 2)
            for_part(chunk_width*1/2, chunk_width*2/2, chunk_height*0/2, chunk_height*1/2);
        else if (v_part[0] == 3)
            for_part(chunk_width*1/2, chunk_width*2/2, chunk_height*1/2, chunk_height*2/2);
        else debug_emit();
    }
    else
    {
        /* if (v_part[0] == 0)
            for_part_uvec4(0, 4);
        else if (v_part[0] == 1)
            for_part_uvec4(4, 8);
        else if (v_part[0] == 2)
            for_part_uvec4(8, 12);
        else if (v_part[0] == 3)
            for_part_uvec4(12, 16);
         */
        debug_emit();
    }
}
