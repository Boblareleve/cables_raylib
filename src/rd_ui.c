#include "render.h"


bool rd_ui_set_uniforms(Shader_programme_id program, rd_ui_Handel *arg)
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
}


bool rd_ui_reset_locs(Shader_programme_id program, void *rd_ui_handel)
{
    rd_ui_Handel *arg = rd_ui_handel;
    memset(&arg->loc, -1, sizeof(arg->loc));
    return true;
}

rd_ui_Handel rd_ui_make_Handel(Camera cam)
{ // draw
    assert(cam.zoom != 0);
    rd_ui_Handel res = {
        .vertices = *da_make_size(&res.vertices, BASE_MAX_UI_ELEMENT_COUNT),
        .loc = {
            .campos = -1,
            .camzoom = -1,
            .ratio = -1,
        },
        .cam = cam
    };
    memset(&res.loc, -1, sizeof(res.loc));

    glGenVertexArrays(1, &res.VAO);
    glGenBuffers(1, &res.VBO);
    
    glBindVertexArray(res.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, res.VBO);
    glBufferData(GL_ARRAY_BUFFER, 
        res.vertices.capacity * sizeof(res.vertices.arr[0]),
        NULL,
        GL_STREAM_DRAW
    );
    res.VBO_capacity = res.vertices.capacity;

    { // vertex pointer attribute
        // layout (location = 0) in vec4  box;
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, box)
        );
        glEnableVertexAttribArray(0);

        // layout (location = 1) in vec4  fill_color;
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, fill_color)
        );
        glEnableVertexAttribArray(1);

        // layout (location = 2) in vec4  border_color;
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, border_color)
        );
        glEnableVertexAttribArray(2);

        // layout (location = 3) in float border_thickness;
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex, border_thickness)
        );
        glEnableVertexAttribArray(3);
        
        // layout (location = 4) in uint  texture_index;
        glVertexAttribPointer(4, 1, GL_UNSIGNED_INT, GL_FALSE,
            sizeof(rd_ui_Omni_rect_vertex), STRUCT_OFFSET(rd_ui_Omni_rect_vertex , texture_index)
        );
        glEnableVertexAttribArray(4);
    }

    if (!rd_load_shader(&res.shader, 
        rd_ui_reset_locs, &res,
        "./shaders/ui/omni_rect.vs",
        GL_VERTEX_SHADER,
        "./shaders/ui/omni_rect.gs",
        GL_GEOMETRY_SHADER,
        "./shaders/ui/omni_rect.fs",
        GL_FRAGMENT_SHADER
    )) TODO("error handeling");
    
    return res;
}

void rd_ui_draw(rd_ui_Handel *handel)
{
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
}
