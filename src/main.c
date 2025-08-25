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

bool set_uniform(Shader_programme_id shader_program, void *arg_void)
{
    Texs *texs = (Texs*)arg_void;

    // lazly get locs
    if (texs->atlas.loc != -1)
        texs->atlas.loc = glGetUniformLocation(shader_program, "atlas");
    if (texs->paste_mouver.loc != -1)
        texs->paste_mouver.loc = glGetUniformLocation(shader_program, "ui_texture1");
    

    glUseProgram(shader_program);

    foreach_static (size_t, i, texs->tex_array)
        glUniform1i(texs->tex_array[i].loc, texs->tex_array[i].texture_unit);
    
    // glUniform2f(loc_campos, 0.0, 0.0);
    // glUniform1i(loc_atlas, atlas_texture_unit_index); // set it manually

    return true;
}

void set_rd_opengl_objects(struct Render_world *rw)
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

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
    glEnableVertexAttribArray(0);

    // glBufferSubData // mod part of buffer
    // glMapBuffer     // map kind as in mmap some implementation can desactivate caching (CPU) so will be really slow
    // glMapBufferRange with the GL_MAP_INVALIDATE_BUFFER_BIT  <=> glBufferData(NULL) -> glBufferData(VBO) to allocate a new
    // glMapBufferRange with the GL_MAP_UNSYNCHRONIZED_BIT tell I won't change queued value in a gl command can be dangerous

    // glDrawElements ?? to render subset
    // keep how many are inactive and use glBufferData when too much
    
    glBindBuffer(GL_TEXTURE_BUFFER, rw->TBO);
    glBufferData(GL_TEXTURE_BUFFER, 
        rw->chunk_texture.capacity * sizeof(rw->chunk_texture.arr[0]),
        rw->chunk_texture.arr, 
        GL_DYNAMIC_DRAW
    );
    glBindTexture(GL_TEXTURE_BUFFER, rw->TEX);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, rw->TBO);
}


Render_world render_init(Texs *texs)
{
    printf("width %f height %f\n", rd_get_screen_width(), rd_get_screen_height());
    
    Render_world res = { 
        // .cam = {
        //     .target = (Vec2){ rd_get_screen_width() / 2, rd_get_screen_width() / 2 }, // Camera offset (displacement from target)
        //     .zoom = rd_get_screen_width() / CHUNK_WORLD_SIZE
        // },
        .cam = { .target = 0, .zoom = 1 },
        .uniform_campos_loc = -1,
        .uniform_camzoom_loc = -1,
        .uniform_width_loc = -1,
        .uniform_height_loc = -1,
        .uniform_ratio_loc = -1
    };

    Camera_print(res.cam);

    da_push(&res.shader.shaders, ((Shader_metadata){
        .file_path = "shaders/world_shaders/world.vs",
        .type = GL_VERTEX_SHADER
    }));
    da_push(&res.shader.shaders, ((Shader_metadata){
        .file_path = "shaders/world_shaders/world.gs",
        .type = GL_GEOMETRY_SHADER
    }));
    da_push(&res.shader.shaders, ((Shader_metadata){
        .file_path = "shaders/world_shaders/world.fs",
        .type = GL_FRAGMENT_SHADER
    }));
    
    if (!rd_reload_shader(&res.shader, set_uniform, texs))
        UNREACHABLE("shader compile");


    set_rd_opengl_objects(&res);

    return res;
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
    res.rd = render_init(&res.texs);

    

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
    foreach_static (size_t, i, rd->uniform_locs)
        if (rd->uniform_locs[i] == -1)
            return true;
    return false;
}
void send_dynamic_uniform_to_gpu(Window *win)
{
    Render_world *rd = &win->rd;

    if (should_update_uniforms_locs(rd))
    {
        // glGetUniformIndices(
        //     rd->shader.shader_program, 
        //     ARRAY_LEN(rd->uniform_locs),
        //     (char*[]){ "campos", "camzoom", "width", "height", "ratio" },
        //     rd->uniform_locs
        // );
        rd->uniform_campos_loc  = glGetUniformLocation(rd->shader.shader_program, "campos");
        rd->uniform_camzoom_loc = glGetUniformLocation(rd->shader.shader_program, "camzoom");
        rd->uniform_width_loc = glGetUniformLocation(rd->shader.shader_program, "width");
        rd->uniform_height_loc = glGetUniformLocation(rd->shader.shader_program, "height");
        rd->uniform_ratio_loc = glGetUniformLocation(rd->shader.shader_program, "ratio");
    }
    
    glUseProgram(rd->shader.shader_program);
    glUniform2f(rd->uniform_campos_loc, rd->cam.target.x, rd->cam.target.y);
    glUniform1f(rd->uniform_camzoom_loc, rd->cam.zoom);
    glUniform1f(rd->uniform_width_loc, rd_get_screen_width());
    glUniform1f(rd->uniform_height_loc, rd_get_screen_height());
    glUniform1f(rd->uniform_ratio_loc, rd_get_screen_ratio());
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
    

    if (1) 
    { // load world
        char *err_world = load_World("./save/world1.wrd", &win.wrd);
        if (err_world) {
            perror("open");
            printf("%s\n", err_world);
            win.wrd = World_make("main");
        }
    }

    while (!rd_should_close())
    {
        glfwSwapInterval(1);
        if (rd_is_key_down(GLFW_KEY_ESCAPE)) rd_set_to_close();
        if (rd_is_key_pressed(GLFW_KEY_SPACE)) 
            rd_reload_shader(&win.rd.shader, set_uniform, &win.texs);
        

        inputs(&win);
        send_dynamic_uniform_to_gpu(&win);
        Window_draw(&win);
        
        rd_end_frame();

        // printf("fps %d\n", rd_get_fps());
        // UPDATE_LINE;
    }
    
    quit(&win);
    
    // Window_free(&win);
    return (0);
}
