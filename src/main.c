#include "main.h"


int _LATCH = 0;
const float direction_to_rotation[4] = { 0.0f, 90.0f, 180.0f, 270.0f };
// static_assert(direction_to_rotation[up] == 0.0f);
// static_assert(direction_to_rotation[left] == 90.0f);
// static_assert(direction_to_rotation[down] == 180.0f);
// static_assert(direction_to_rotation[right] == 270.0f);


void resize_callback(Render *render, int width, int height)
{
    UNUSED(render);
    printf("resize w%d h%d\n", width, height);
}


// char const* atlas_name;
// int atlas_texture_unit_index;
// char const* paste_mouver_name;
// int paste_mouver_texture_unit_index;

bool set_world_static_uniform(Shader_programme_id shader_program, void *arg_void)
{
    Window *win = (Window*)arg_void;

    glUseProgram(shader_program);

    // lazly get locs
    /* if (win->texs.atlas.loc        == -1
     || win->texs.paste_mouver.loc == -1
     || win->wrd_render.loc.chunks == -1
    ) */ 
    {
        win->texs.atlas.loc        = glGetUniformLocation(shader_program, "atlas");
        win->texs.paste_mouver.loc = glGetUniformLocation(shader_program, "ui_texture1");
        win->wrd_render.loc.chunks = glGetUniformLocation(shader_program, "chunks");
    }
    
    foreach_static (size_t, i, win->texs.tex_array)
        glUniform1i(win->texs.tex_array[i].loc, win->texs.tex_array[i].texture_unit);
    glUniform1i(win->wrd_render.loc.chunks, win->wrd_render.TBO_texture_unit);
   
    return true;
}

int texture_unit_count = 0;
void set_rd_opengl_objects(Render_world *rw, Shader *shader)
{
    rw->vertices_to_update = true;
    rw->max_visible_chunk = 64*64;



    rw->vertices      = *da_make_size(&rw->vertices, rw->max_visible_chunk);
    rw->chunk_texture = *da_make_size(&rw->chunk_texture, rw->max_visible_chunk);

    glGenVertexArrays(1,    &rw->VAO);
    glGenBuffers(1,         &rw->VBO);
    glGenBuffers(1,         &rw->TBO);
    glGenTextures(1,        &rw->TEX);
    
    glBindVertexArray(rw->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, rw->VBO);
    glBufferData(GL_ARRAY_BUFFER, 
        rw->vertices.capacity * sizeof(rw->vertices.arr[0]),
        rw->vertices.arr, 
        GL_DYNAMIC_DRAW
    );
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vec2), (void*)0);
    glEnableVertexAttribArray(0);

    // glBufferSubData // mod part of buffer
    // glMapBuffer     // map kind as in mmap some implementation can desactivate caching (CPU) so will be really slow
    // glMapBufferRange with the GL_MAP_INVALIDATE_BUFFER_BIT  <=> glBufferData(NULL) -> glBufferData(VBO) to allocate a new
    // glMapBufferRange with the GL_MAP_UNSYNCHRONIZED_BIT tell I won't change queued value in a gl command can be dangerous

    // glDrawElements ?? to render subset
    // keep how many are inactive and use glBufferData when too much

    rw->TBO_texture_unit = texture_unit_count++;
    glBindBuffer(GL_TEXTURE_BUFFER, rw->TBO);
    glBufferData(GL_TEXTURE_BUFFER,
        rw->chunk_texture.capacity * sizeof(rw->chunk_texture.arr[0]),
        rw->chunk_texture.arr,
        GL_DYNAMIC_DRAW
    );
    glBindTexture(GL_TEXTURE_BUFFER, rw->TEX);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R8UI, rw->TBO);
    // glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, rw->TBO);
}



Window Window_make(const char *name)
{
    const float scale = 800.;
    const float ratio = 1.;
    // InitWindow(1600 / ratio, 900 / ratio, name);
    
    
    Window res = {0};
    res.render = rd_init(name, scale * ratio, scale, resize_callback);
    res.wrd = (World){0};
    res.texs = Texs_make("./assets/new_atlas.png", "./assets/four_way_arrow.png");
    res.ui = Ui_make(res.render);
    res.wrd_render = (Render_world){ 
        .cam = { .target = 0, .zoom = 1 },
        .loc = {
            .campos = -1,
            .camzoom = -1,
            .width = -1,
            .height = -1,
            .ratio = -1,
            .chunks = -1
        }
    };
    if (!rd_load_shader(&res.wrd_render.shader,
        set_world_static_uniform, &res, 
        "shaders/world/chunk.vs", GL_VERTEX_SHADER,
        "shaders/world/chunk.gs", GL_GEOMETRY_SHADER,
        "shaders/world/chunk.fs", GL_FRAGMENT_SHADER
    )) assert("shader compile"[0] != 's');

    
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // glBlendFunc(GL_ONE, GL_SRC_ALPHA);
    
    glDepthMask(GL_FALSE);



    set_rd_opengl_objects(&res.wrd_render, &res.wrd_render.shader);

    rd_set_clear_color(DARKGRAY);


    
    return res;
}
void Window_free(Window *win)
{
    Texs_free(&win->texs);
    World_free(&win->wrd);
    Ui_free(&win->ui);
    rd_deinit();
}


static inline bool should_update_uniforms_locs(const Render_world *rd)
{
    foreach_static (size_t, i, rd->loc.array)
        if (rd->loc.array[i] == -1)
            return true;
    return false;
}
void send_dynamic_uniform_to_gpu(Window *win)
{
    Render_world *rd = &win->wrd_render;

    if (should_update_uniforms_locs(rd))
    {
        // glGetUniformIndices(
        //     rd->shader.shader_program, 
        //     ARRAY_LEN(rd->uniform_locs),
        //     (char*[]){ "campos", "camzoom", "width", "height", "ratio" },
        //     rd->uniform_locs
        // );
        rd->loc.campos  = glGetUniformLocation(rd->shader.program, "campos");
        rd->loc.camzoom = glGetUniformLocation(rd->shader.program, "camzoom");
        rd->loc.width = glGetUniformLocation(rd->shader.program, "width");
        rd->loc.height = glGetUniformLocation(rd->shader.program, "height");
        rd->loc.ratio = glGetUniformLocation(rd->shader.program, "ratio");
        rd->loc.chunks = glGetUniformLocation(rd->shader.program, "chunks");
    }
    
    glUseProgram(rd->shader.program);
    glUniform2f(rd->loc.campos, rd->cam.target.x, rd->cam.target.y);
    glUniform1f(rd->loc.camzoom, rd->cam.zoom);
    glUniform1f(rd->loc.width, rd_get_screen_width());
    glUniform1f(rd->loc.height, rd_get_screen_height());
    glUniform1f(rd->loc.ratio, rd_get_screen_ratio());
    
    glActiveTexture(GL_TEXTURE0 + rd->TBO_texture_unit);
    glUniform1i(rd->loc.chunks, rd->TBO_texture_unit);
}


/* TODO:

 * edition 
    -> selection copy paste rotate flip
 * load/save file
    -> GUI
 * color by chunk for thread lockless
 * shader render
 * finish connection engin (support for bridge and update when possing T en N)
 * debug engin
 * 
*/

int main(void)
{
    Window win = Window_make("main");
    

    if (1) // load world
    {
        char *err_world = load_World("./save/world1.wrd", &win.wrd);
        if (err_world) {
            perror("open");
            printf("%s\n", err_world);
            win.wrd = World_make("main");
        }
    }

    /* {
        win.wrd_render.vertices.size = 0;
        win.wrd_render.chunk_texture.size = 0;
        da_push_zero(&win.wrd_render.vertices);
        
        // da_push(&win.wrd_render.vertices, ((Vec2){ 1, 0 }));
        // da_push(&win.wrd_render.vertices, ((Vec2){ 1, 0 }));
        // da_push(&win.wrd_render.vertices, ((Vec2){ 1, 0 }));
        // da_push(&win.wrd_render.vertices, ((Vec2){ 1, 0 }));
        
        
        da_push_zero(&win.wrd_render.chunk_texture);
        // da_push_zero(&win.wrd_render.chunk_texture);
    } */
    
    rd_ui_Handel ui_handel = rd_ui_make_Handel((Camera){ .zoom = 1.0 });
    da_push(&ui_handel.vertices, ((rd_ui_Omni_rect_vertex){
        .box = (Rect){
            .pos = { 0.1, 0.2 },
            .width = 0.1,
            .height = 0.1
        },
        .border_color = BLACK,
        .fill_color = GRAY,
        .border_thickness = 0.03,
        .texture_index = 0
    }));
    da_push(&ui_handel.vertices, ((rd_ui_Omni_rect_vertex){
        .box = (Rect){
            .pos = { 0.25, 0.2 },
            .width = 0.1,
            .height = 0.1
        },
        .border_color = BLACK,
        .fill_color = GRAY,
        .border_thickness = 0.03,
        .texture_index = 0
    }));
    

    while (!rd_should_close())
    {
        inputs(&win);


        send_dynamic_uniform_to_gpu(&win);
        Window_draw(&win);


        if (rd_is_key_pressed(GLFW_KEY_SPACE))
            rd_reload_shader(&ui_handel.shader, &ui_handel);
        rd_ui_draw(&ui_handel);
        
        rd_end_frame();

#ifndef __linux__
        glfwSetWindowTitle(win.render->glfw, Sprintf("fps %d", rd_get_fps()));
        printf(print_buffer);
        print_buffer_size = 0;
#endif /* not __linux__ */

        // printf("fps %d\n", rd_get_fps());
        // UPDATE_LINE;
    }
    
    // rd_ui_free_Handel(&ui_handel);
    quit(&win);
    
    // Window_free(&win);
    return (0);
}
