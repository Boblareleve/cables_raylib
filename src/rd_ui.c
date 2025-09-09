#include "render.h"
extern int texture_unit_count;


/* bool rd_ui_set_uniforms(Shader_programme_id program, rd_ui_Handel *arg)
{
    // lazly get locs
    if (arg->loc.campos  == -1
     || arg->loc.camzoom == -1
     || arg->loc.ratio   == -1
    ) {
        arg->loc.campos  = glGetUniformLocation(program, "campos");
        arg->loc.camzoom = glGetUniformLocation(program, "camzoom");
        arg->loc.ratio   = glGetUniformLocation(program, "ratio");
    }
    
    glUseProgram(program);

    // printf("uniform ui screen ratio %f\n", rd_get_screen_ratio());
    glUniform2f(arg->loc.campos,  arg->cam.target.x, arg->cam.target.y);
    glUniform1f(arg->loc.camzoom, arg->cam.zoom);
    glUniform1f(arg->loc.ratio,   rd_get_screen_ratio());
    
    return true;
} */
bool rd_ui_set_uniforms(Shader_programme_id program, rd_ui_Handel *arg)
{
    glUseProgram(program);

    // lazly get locs
    if (arg->loc.window_width  == -1
     || arg->loc.window_height == -1
     || arg->atlas.loc         == -1
    ) {
        arg->loc.window_width  = glGetUniformLocation(program, "window_width" );
        arg->loc.window_height = glGetUniformLocation(program, "window_height");
        arg->atlas.loc         = glGetUniformLocation(program, "atlas"        );

        glUniform1i(arg->atlas.loc, arg->atlas.texture_unit); // only set texture sampler once
    }
    
    // printf("uniform ui screen dim w%f  h%f\n", arg->ui_virtual_width, arg->ui_virtual_height);
    glUniform1f(arg->loc.window_width,  arg->ui_virtual_width);
    glUniform1f(arg->loc.window_height, arg->ui_virtual_height);
    
    return true;
}


bool rd_ui_reset_locs(Shader_programme_id program, void *rd_ui_handel)
{
    UNUSED(program);
    rd_ui_Handel *arg = rd_ui_handel;
    memset(&arg->loc, -1, sizeof(arg->loc));
    return true;
}

static void set_rd_ui_rect_vpa(void)
{
    // layout (location = 0) in vec4  box;
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
        sizeof(rd_ui_rect), STRUCT_OFFSET(rd_ui_rect, box)
    );
    glEnableVertexAttribArray(0);

    // layout (location = 1) in vec4  fill_color;
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
        sizeof(rd_ui_rect), STRUCT_OFFSET(rd_ui_rect, fill_color)
    );
    glEnableVertexAttribArray(1);

    // layout (location = 2) in vec4  border_color;
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
        sizeof(rd_ui_rect), STRUCT_OFFSET(rd_ui_rect, border_color)
    );
    glEnableVertexAttribArray(2);

    // layout (location = 3) in vec4  on_hover_filter;
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE,
        sizeof(rd_ui_rect), STRUCT_OFFSET(rd_ui_rect, on_hover_filter)
    );
    glEnableVertexAttribArray(3);

    // layout (location = 4) in float border_thickness;
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
        sizeof(rd_ui_rect), STRUCT_OFFSET(rd_ui_rect, border_thickness)
    );
    glEnableVertexAttribArray(4);
    
    // layout (location = 5) in uint  texture_index;
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE,
        sizeof(rd_ui_rect), STRUCT_OFFSET(rd_ui_rect, texture_index)
    );
    glEnableVertexAttribArray(5);
    
    printf("struct layout: sizeof(T)=%"PRIu64"\n", sizeof(rd_ui_rect));
    printf("\t&NULL.box = %"PRIu64"\n",               (uintptr_t)STRUCT_OFFSET(rd_ui_rect, box));
    printf("\t&NULL.fill_color = %"PRIu64"\n",        (uintptr_t)STRUCT_OFFSET(rd_ui_rect, fill_color));
    printf("\t&NULL.border_color = %"PRIu64"\n",      (uintptr_t)STRUCT_OFFSET(rd_ui_rect, border_color));
    printf("\t&NULL.on_hover_filter = %"PRIu64"\n",   (uintptr_t)STRUCT_OFFSET(rd_ui_rect, on_hover_filter));
    printf("\t&NULL.border_thickness = %"PRIu64"\n",  (uintptr_t)STRUCT_OFFSET(rd_ui_rect, border_thickness));
    printf("\t&NULL.texture_index = %"PRIu64"\n",     (uintptr_t)STRUCT_OFFSET(rd_ui_rect, texture_index));
}
   

typedef uint32_t rd_ui_rect_handel;
rd_ui_rect_handel rd_ui_append_rect(rd_ui_Handel *handel, rd_ui_rect rect)
{
    rd_ui_rect_handel res = handel->vertices.size;
    da_push(&handel->vertices, rect);

    return res;
}

void rd_ui_enable_rect(rd_ui_Handel *handel, rd_ui_rect_handel rect)
{
    da_push(&handel->indices, 0);
}

rd_ui_Handel rd_ui_make_Handel(float scale)
{ // draw
    assert(scale > 0.1);
    rd_ui_Handel res = {
        .vertices = *da_make_size(&res.vertices, BASE_MAX_UI_ELEMENT_COUNT),
        .ui_scale = scale,
        .ui_virtual_width  = rd_get_screen_width()  * scale,
        .ui_virtual_height = rd_get_screen_height() * scale,
        .atlas = {0}
    };
    memset(&res.loc, -1, sizeof(res.loc));
    
    res.atlas = rd_load_texture(
        "./assets/ui_atlas.png",
        texture_unit_count++ //rd_dispatch_texture_unit(&res.shader, GL_FRAGMENT_SHADER)
    );

    if (!rd_load_shader(&res.shader, 
        rd_ui_reset_locs, &res,
        "./shaders/ui/omni_rect.vs",
        GL_VERTEX_SHADER,
        "./shaders/ui/omni_rect.gs",
        GL_GEOMETRY_SHADER,
        "./shaders/ui/omni_rect.fs",
        GL_FRAGMENT_SHADER
    )) TODO("error handeling");

    glGenVertexArrays(1, &res.VAO);
    glGenBuffers(1, &res.VBO);
    
    glBindVertexArray(res.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, res.VBO);
    glBufferData(GL_ARRAY_BUFFER, 
        res.vertices.capacity * sizeof(res.vertices.arr[0]),
        NULL,
        GL_STREAM_DRAW
    );
    res.VBO_capacity = 1; //res.vertices.capacity;

/*     { // vertex pointer attribute
        // layout (location = 0) in vec4  box;
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, box)
        );
        glEnableVertexAttribArray(0);

        // layout (location = 1) in uint  fill_color;
        glVertexAttribPointer(1, 1, GL_UNSIGNED_INT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, fill_color)
        );
        glEnableVertexAttribArray(1);

        // layout (location = 2) in uint  border_color;
        glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, border_color)
        );
        glEnableVertexAttribArray(2);

        // layout (location = 3) in uint  on_hover_color;
        glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, on_hover_color)
        );
        glEnableVertexAttribArray(3);

        // layout (location = 4) in float border_thickness;
        glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, border_thickness)
        );
        glEnableVertexAttribArray(4);
        
        // layout (location = 5) in uint  texture_index;
        glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, texture_index)
        );
        glEnableVertexAttribArray(5);
        
        printf("struct layout: sizeof(T)=%"PRIu64"\n", sizeof(rd_ui_Omni_rect_vertex));
        printf("\t&NULL.box = %"PRIu64"\n",               (uintptr_t)STRUCT_OFFSET(rd_ui_Omni_rect_vertex, box));
        printf("\t&NULL.fill_color = %"PRIu64"\n",        (uintptr_t)STRUCT_OFFSET(rd_ui_Omni_rect_vertex, fill_color));
        printf("\t&NULL.border_color = %"PRIu64"\n",      (uintptr_t)STRUCT_OFFSET(rd_ui_Omni_rect_vertex, border_color));
        printf("\t&NULL.on_hover_color = %"PRIu64"\n",    (uintptr_t)STRUCT_OFFSET(rd_ui_Omni_rect_vertex, on_hover_color));
        printf("\t&NULL.border_thickness = %"PRIu64"\n",  (uintptr_t)STRUCT_OFFSET(rd_ui_Omni_rect_vertex, border_thickness));
        printf("\t&NULL.texture_index = %"PRIu64"\n",     (uintptr_t)STRUCT_OFFSET(rd_ui_Omni_rect_vertex, texture_index));
    }
 */
    
    set_rd_ui_rect_vpa();

    
    return res;
}

void rd_ui_draw(rd_ui_Handel *handel)
{
    handel->ui_virtual_width  = rd_get_screen_width()  * handel->ui_scale;
    handel->ui_virtual_height = rd_get_screen_height() * handel->ui_scale;
    

    glBindBuffer(GL_ARRAY_BUFFER, handel->VBO);
    if (handel->VBO_capacity < handel->vertices.size)
    {
        glBufferData(GL_ARRAY_BUFFER, 
            handel->vertices.size * sizeof(handel->vertices.arr[0]),
            NULL,
            GL_STREAM_DRAW
        );
    }
    
    glBufferSubData(GL_ARRAY_BUFFER,
        0, handel->vertices.size * sizeof(handel->vertices.arr[0]),
        handel->vertices.arr
    );

    if (!rd_ui_set_uniforms(handel->shader.program, handel)) TODO("error handeling");
    glActiveTexture(GL_TEXTURE0 + handel->atlas.texture_unit);
    glBindTexture(GL_TEXTURE_2D, handel->atlas.id);
    glBindVertexArray(handel->VAO);
    glUseProgram(handel->shader.program);
    glDrawArrays(GL_POINTS, 0, handel->vertices.size);
}


void rd_ui_free_Handel(rd_ui_Handel *handel)
{
    glDeleteVertexArrays(1, &handel->VAO);
    glDeleteBuffers(1, &handel->VBO);
    da_free(&handel->vertices);
    rd_unload_shader(&handel->shader);
    // rd_unload_texture(&handel->atlas);
}
