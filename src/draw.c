#include "main.h"

Texs Texs_make(const char *atlas_name, const char *paste_mouver_name)
{
    Texs res = {
        .atlas = rd_load_texture(atlas_name, 0),
        .paste_mouver = rd_load_texture(paste_mouver_name, 1),
        .cell_size = W
    };
    foreach_static (size_t, i, res.tex_array)
        assert(res.tex_array[i].id);

    // SetTextureFilter(res.atlas, TEXTURE_FILTER_POINT);
    // SetTextureFilter(res.tex, TEXTURE_FILTER_BILINEAR);
    // SetTextureFilter(res.tex, TEXTURE_FILTER_TRILINEAR);       // Trilinear filtering (linear with mipmaps)
    // SetTextureFilter(res.tex, TEXTURE_FILTER_ANISOTROPIC_4X);  // Anisotropic filtering 4x
    // SetTextureFilter(res.tex, TEXTURE_FILTER_ANISOTROPIC_8X);  // Anisotropic filtering 8x
    // SetTextureFilter(res.tex, TEXTURE_FILTER_ANISOTROPIC_16X); // Anisotropic filtering 16x

    return res;
}
void Texs_free(Texs *texs)
{
    foreach_static (size_t, i, texs->tex_array)
        glDeleteTextures(TEXS_TEXTURE_COUNT, &texs->tex_array[i].id);
}


static inline void Chunk_push(Window *win, Chunk *chunk)
{
    chunk->render.index_buffer = win->wrd_render.chunk_texture.size;

    da_push_array(&win->wrd_render.chunk_texture, chunk->arr);
    static_assert(sizeof(chunk->arr) == 256);
    static_assert(sizeof(win->wrd_render.chunk_texture.arr[0]) == 256);
    
    // da_push_array(&win->wrd_render.vertices, ((Vec24){
    //     POS_TO_VEC2(chunk->pos), 
    //     POS_TO_VEC2(chunk->pos),
    //     POS_TO_VEC2(chunk->pos),
    //     POS_TO_VEC2(chunk->pos)
    // }));
    da_push(&win->wrd_render.vertices, POS_TO_VEC2(chunk->pos));
    static_assert(sizeof(chunk->pos) == 4 * 2);
}

// offset, size (index) 
static inline void Chunk_push_gpu(Window *win, int offset, int size)
{
    // glMapBufferRange with the GL_MAP_UNSYNCHRONIZED_BIT

    glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        offset * sizeof(win->wrd_render.vertices.arr[0]),
        size   * sizeof(win->wrd_render.vertices.arr[0]),
        &win->wrd_render.vertices.arr[offset]
    );
    glBindBuffer(GL_TEXTURE_BUFFER, win->wrd_render.TBO);
    glBufferSubData(
        GL_TEXTURE_BUFFER,
        offset * sizeof(win->wrd_render.chunk_texture.arr[0]),
        size   * sizeof(win->wrd_render.chunk_texture.arr[0]),
        &win->wrd_render.chunk_texture.arr[offset]
    );
}

void push_chunk_to_draw(Window *win, Pos upper_left_pos, Pos lower_right_pos, int *draw_count)
{
    *draw_count = 0;
    

    for (int y = lower_right_pos.y; y <= upper_left_pos.y; y++)
    {
        for (int x = upper_left_pos.x; x <= lower_right_pos.x; x++)
        {
            // TODO use graph
            Item_chunks *chunk = set_Item_chunks_get(
                &win->wrd.chunks,
                (Item_chunks){ .pos = (Pos){ x, y }}
            );
            if (chunk && chunk->data)
            {
                (*draw_count)++;
                if (chunk->data->render.index_buffer == -1)
                {
                    chunk->data->render.index_buffer = win->wrd_render.chunks.size;
                    da_push(&win->wrd_render.chunks, chunk->data);
                }
                
                // Chunk_draw(win, chunk->data);
            }
            // if (CHUNK_POS_VALUE)
            //     DrawTextF("(%d, %d)", 
            //         x * CHUNK_WORLD_SIZE + 4,
            //         -y * CHUNK_WORLD_SIZE + 4 - CHUNK_WORLD_SIZE,
            //         4., PURPLE, x, y
            //     );
        }
    }
}
void reset_chunk_to_draw(Window *win)
{
    foreach_ptr (Chunk *, chunk, &win->wrd_render.chunks)
        (*chunk)->render.index_buffer = -1;
    
    win->wrd_render.chunks.size = 0;
    win->wrd_render.vertices.size = 0;
    win->wrd_render.chunk_texture.size = 0;
}


void World_draw(Window *win)
{
    const Pos lu_ws = get_chunk_Pos(screen_to_Pos(win->wrd_render.cam, (Vec2){0, 0}));
    Camera_print(win->wrd_render.cam);
    const Pos TMP = screen_to_Pos(win->wrd_render.cam, (Vec2){0, 0});
    printf("{0,0} to screen %d,%d\n", TMP.x, TMP.y);

    const Pos rd_ws = get_chunk_Pos(screen_to_Pos(win->wrd_render.cam, (Vec2){ win->render->screen_width, win->render->screen_height }));
    printf("form chunk %d,%d to %d,%d\n", 
        lu_ws.x,
        lu_ws.y,
        rd_ws.x,
        rd_ws.y
    );

    assert(win->wrd_render.chunk_texture.size == win->wrd_render.vertices.size);
    assert(win->wrd_render.chunk_texture.size == win->wrd_render.chunks.size);
    
    // static da_ptr_Chunk to_draw_chunk = {0};
    int draw_count = 0;
    push_chunk_to_draw(win, lu_ws, rd_ws, &draw_count);
    assert(win->wrd_render.chunks.size <= draw_count);
    
    // draw_count -> how many chunk I should draw (min)
    // to_draw_chunk -> how many chunk I the previous tick 
    // win->wrd_render.vertices.size -> how many chunk I draw (might be reduce in the vertex shader)
    // win->wrd_render.max_visible_chunk -> the numbre of chunk I should draw MAX

    // if too many for buffer or 50% + chunk are not visible
    if (win->wrd_render.chunks.size >= (double)win->wrd_render.max_visible_chunk
     || (double)draw_count < 0.5 * win->wrd_render.chunks.size
    ) {
        reset_chunk_to_draw(win);
        
        // maybe will cause lag spike
        for (int y = rd_ws.y; y <= lu_ws.y; y++)
            for (int x = lu_ws.x; x <= rd_ws.x; x++)
            {
                Item_chunks *chunk = set_Item_chunks_get(
                    &win->wrd.chunks,
                    (Item_chunks){ .pos = (Pos){ x, y }}
                );
                if (chunk) Chunk_push(win, chunk->data);
            }
        
        Chunk_push_gpu(win, 0, win->wrd_render.vertices.size);
    }
    else 
    {
        int save_index = win->wrd_render.chunk_texture.size;
        assert(win->wrd_render.chunk_texture.size == win->wrd_render.vertices.size);

        for (int i = save_index; i < win->wrd_render.chunks.size; i++)
            Chunk_push(win, win->wrd_render.chunks.arr[i]);

        Chunk_push_gpu(win, save_index, win->wrd_render.chunks.size - save_index);
    }
    printf("chunk to draw: %d\n", win->wrd_render.chunks.size);
}

void Window_draw(Window *win)
{
    if (1)
    {
        if (win->wrd_render.vertices.size != 1 || win->wrd_render.chunk_texture.size != 1)
        {
            win->wrd_render.vertices.size = 0;
            win->wrd_render.chunk_texture.size = 0;
            da_push_zero(&win->wrd_render.vertices);
            // da_push(&win->wrd_render.vertices, ((Vec2){ 1, 0 }));
            da_push_zero(&win->wrd_render.chunk_texture);
            // da_push_zero(&win->wrd_render.chunk_texture);
        }
        
        const int offset = 0;
        const int size = 1;
        glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
        glBufferSubData(
            GL_ARRAY_BUFFER,
            offset * sizeof(win->wrd_render.vertices.arr[0]),
            size   * sizeof(win->wrd_render.vertices.arr[0]),
            &win->wrd_render.vertices.arr[offset]
        );
        glBindBuffer(GL_TEXTURE_BUFFER, win->wrd_render.TBO);
        glBufferSubData(
            GL_TEXTURE_BUFFER,
            offset * sizeof(win->wrd_render.chunk_texture.arr[0]),
            size   * sizeof(win->wrd_render.chunk_texture.arr[0]),
            &win->wrd_render.chunk_texture.arr[offset]
        );
    }
    else
    {
        reset_chunk_to_draw(win);

        for_set (Item_chunks, item, &win->wrd.chunks)
        {
            if (item && item->data)
                Chunk_push(win, item->data);
            else printf("set NULL\n");
        }
        Chunk_push_gpu(win, 0, win->wrd_render.vertices.size);

        assert(win->wrd_render.vertices.size == win->wrd_render.chunk_texture.size);
        assert(win->wrd_render.vertices.size <= 3);
    }

    // World_draw(win);
    // Ui_draw(win);

    {
        glUseProgram(win->wrd_render.shader.program);
        glBindVertexArray(win->wrd_render.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
        glDrawArrays(GL_POINTS, 0, win->wrd_render.vertices.size);
        {
            // da_print(&win->wrd_render.vertices, v2_print);
            // printf("cam: %s zoom %f\n", v2_sprint(win->wrd_render.cam.target), win->wrd_render.cam.zoom);
            // print_buffer_size = 0;
            assert(win->wrd_render.vertices.size == 1);
        }
        // assert(glGetError() == GL_NO_ERROR);
        // printf("draw %d points\n", win->wrd_render.vertices.size);
    }
}

