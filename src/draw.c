#include "main.h"

extern int texture_unit_count;
Texs Texs_make(const char *atlas_name, const char *paste_mouver_name)
{
    Texs res = {0};
    res.atlas = rd_load_texture(atlas_name, texture_unit_count++);
    res.paste_mouver = rd_load_texture(paste_mouver_name, texture_unit_count++);
    res.cell_size = W;
    
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
    // da_push_array(&win->wrd_render.chunk_texture, exemple);
    /* Chunk_raw_2D tmp = {0};
    memcpy(tmp, chunk->arr, sizeof(Chunk_raw));
    printf("chunk %p:\n", (void*)chunk);
    for (int y = 15; y >= 0; y--)
    {
        for (int x = 0; x < 16; x++)
            printf("%02u ", tmp[x][y]);
        printf("\n");
    } */
    
    da_push_array(&win->wrd_render.chunk_texture, chunk->arr);

    da_push_array(&win->wrd_render.vertices, ((Vec24){
        POS_TO_VEC2(chunk->pos), 
        POS_TO_VEC2(chunk->pos),
        POS_TO_VEC2(chunk->pos),
        POS_TO_VEC2(chunk->pos)
    }));
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

    glActiveTexture(GL_TEXTURE0 + win->wrd_render.TBO_texture_unit);
    glBindBuffer(GL_TEXTURE_BUFFER, win->wrd_render.TBO);
    glBindTexture(GL_TEXTURE_BUFFER, win->wrd_render.TEX);
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
    // Camera_print(win->wrd_render.cam);
    // const Pos tmp = screen_to_Pos(win->wrd_render.cam, (Vec2){0, 0});
    
    // Vec2 world_vec = rd_get_screen_to_world(win->wrd_render.cam, (Vec2){0});
    // printf(" -> ");
    // v2_print(world_vec);
    // printf("\n");
    
    Pos lu_ws = get_chunk_Pos(screen_to_Pos(win->wrd_render.cam, (Vec2){0, 0}));
    Pos rd_ws = get_chunk_Pos(screen_to_Pos(win->wrd_render.cam, (Vec2){ win->render->screen_width, win->render->screen_height }));
    
    lu_ws = (Pos){ -3,  3 };
    rd_ws = (Pos){  3, -3 };

    // printf("{0,0} to screen %d,%d\n", tmp.x, tmp.y);
    if (0) printf("form chunk %d,%d to %d,%d\n", 
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

    // if too many for buffer or 50%+ chunk are not visible
    if (win->wrd_render.chunks.size >= (double)win->wrd_render.max_visible_chunk
     || (double)draw_count < 0.5 * win->wrd_render.chunks.size
    ) {
        assert(0);
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
        if (save_index < win->wrd_render.chunks.size)
            Chunk_push_gpu(win, save_index, win->wrd_render.chunks.size - save_index);
    }

    if (0) printf("chunk to draw: %d\n", win->wrd_render.chunks.size);

    {
        glUseProgram(win->wrd_render.shader.program);
        glBindVertexArray(win->wrd_render.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
        glBindTexture(GL_TEXTURE_2D, win->wrd_render.atlas.id);
        glActiveTexture(GL_TEXTURE0 + win->wrd_render.atlas.texture_unit);
        glBindTexture(GL_TEXTURE_BUFFER, win->wrd_render.TEX);
        
        glDrawArrays(GL_POINTS, 0, 4 * win->wrd_render.vertices.size);
    }
}

void Window_draw(Window *win)
{
    glDepthMask(GL_FALSE);

    World_draw(win);
    /* else if (0)
    {
        Chunk_raw_2D chunk_raw = {0};
        //     { 16, 16, 16, 16 },
        //     { 16, 16, 16, 16 }
        // };
        // memset(chunk_raw, 17, sizeof(Chunk_raw_2D));
        for (int x = 0; x < 16; x++)
            for (int y = 0; y < 16; y++)
                if (x == y)
                    chunk_raw[y][x] = 16;
        for (int x = 0; x < 16; x++)
            chunk_raw[0][x] = 18;
        
        
        glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vec24), &(Vec24){0}, GL_DYNAMIC_DRAW);
        

        glActiveTexture(GL_TEXTURE0 + win->wrd_render.TBO_texture_unit);
        glBindBuffer(GL_TEXTURE_BUFFER, win->wrd_render.TBO);
        glBufferData(GL_TEXTURE_BUFFER, sizeof(Chunk_raw_2D), chunk_raw, GL_DYNAMIC_DRAW);
        
        
        glUseProgram(win->wrd_render.shader.program);
        glBindVertexArray(win->wrd_render.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
        glBindTexture(GL_TEXTURE_BUFFER, win->wrd_render.TEX);
        glDrawArrays(GL_POINTS, 0, 4);
    }
    else
    {
        reset_chunk_to_draw(win);

        // printf("size wrd.chs.size %d\n", win->wrd.chunks.item_count);
        for_set (Item_chunks, item, &win->wrd.chunks)
        {
            assert(item && item->data);
            Chunk_push(win, item->data);
        }
        Chunk_push_gpu(win, 0, win->wrd_render.vertices.size);

        assert(win->wrd_render.vertices.size == win->wrd_render.chunk_texture.size);
        // assert(win->wrd_render.vertices.size == 2);
        

        glUseProgram(win->wrd_render.shader.program);
        glBindVertexArray(win->wrd_render.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, win->wrd_render.VBO);
        glBindTexture(GL_TEXTURE_BUFFER, win->wrd_render.TEX);
        glDrawArrays(GL_POINTS, 0, 4 * win->wrd_render.vertices.size);
    }
 */

    // Ui_draw(win);
}

