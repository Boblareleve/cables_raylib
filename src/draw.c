#include "main.h"


Texture2D load_tex(const char *name)
{
    Texture2D res = LoadTexture(name);
    assert(IsTextureValid(res));
    return res;
}


Texs Texs_make(const char *atlas_name)
{
    return (Texs){
        .tex = load_tex(atlas_name),
        .cell_size = W
    };
}
void Texs_free(Texs *obj)
{
    UnloadTexture(obj->tex);
}

static inline void draw_Texs_Cell_V(const Texs *texs, Cell id, Rectangle pos)
{
    if ((id & TYPE_MASK) == empty)
        return ;

    DrawTexturePro(
        texs->tex,
        (Rectangle){
            .x = texs->cell_size * (id),
            .y = 0, //texs->cell_size * TEX_CHUNK_Y_ID(id),
            .width = texs->cell_size,
            .height = texs->cell_size
        },
        pos,
        (Vector2){0},   // texs->cell_size / 2, texs->cell_size / 2}, 
        0,              //direction_to_rotation[DIRECTION_MASK & id], 
        WHITE
    );
}

void Chunk_draw(const Window *win, const Chunk *obj)
{
    if (obj == NULL)
        return ;

    Vector2 chunk_pos = (Vector2){
         obj->pos.x * CHUNK_WORLD_SIZE, 
        -obj->pos.y * CHUNK_WORLD_SIZE
    };


    if (CHUNK_GRID_VALUE)
    {
        DrawRectangleLinesEx(
            (Rectangle){
                chunk_pos.x, 
                chunk_pos.y - CHUNK_WORLD_SIZE, 
                CHUNK_WORLD_SIZE, 
                CHUNK_WORLD_SIZE
            }, 
            1, RED
        );
        for (int row = 1; row < H; row++)
        DrawLineEx(
            (Vector2){chunk_pos.x,                    chunk_pos.y - CHUNK_WORLD_SIZE + row * win->texs.cell_size },
            (Vector2){chunk_pos.x + CHUNK_WORLD_SIZE, chunk_pos.y - CHUNK_WORLD_SIZE + row * win->texs.cell_size },
            .5, RED
        );
        for (int col = 1; col < H; col++)
        DrawLineEx(
            (Vector2){chunk_pos.x + col * win->texs.cell_size, chunk_pos.y },
            (Vector2){chunk_pos.x + col * win->texs.cell_size, chunk_pos.y - CHUNK_WORLD_SIZE },
            1, RED
        );
    }
    
    
    Vector2 current_cell_pos = (Vector2){
        .x = chunk_pos.x,
        .y = chunk_pos.y - CHUNK_WORLD_SIZE
    };
    for (int y = H - 1; y >= 0; y--)
    {
        for (int x = 0; x < W; x++)
        {
            draw_Texs_Cell_V(
                &win->texs,
                obj->arr2D[x][y],
                (Rectangle){
                    .x = current_cell_pos.x,
                    .y = current_cell_pos.y,
                    .width =  CHUNK_WORLD_SIZE / W,
                    .height = CHUNK_WORLD_SIZE / H
                }
            );
            current_cell_pos.x += win->texs.cell_size;
        }
        current_cell_pos.x = chunk_pos.x;
        current_cell_pos.y += win->texs.cell_size;
    }

    if (CHUNK_POS_VALUE)
        DrawTextF("(%d, %d)", 
            chunk_pos.x + 4,
            chunk_pos.y + 4 - CHUNK_WORLD_SIZE,
            4., PURPLE, obj->pos.x, obj->pos.y
        );
}


static inline Rectangle get_from_pos_to_pos(float cell_size, Pos start, Pos end)
{
    if (end.x < start.x)
        SWAP(end.x, start.x);
    if (end.y < start.y)
        SWAP(end.y, start.y);
    
    const int width =  abs(end.x - start.x) + 1;
    const int height = abs(end.y - start.y) + 1;

    return (Rectangle){
         start.x * W,
        -end.y   * H - H,
        width  * cell_size,
        height * cell_size,
    };
}

void Window_draw(const Window *obj)
{
    int draw_count = 0;
    
    
    ClearBackground(DARKGRAY);
    DrawTextF("[FPS %d]", 10, 10, 24, WHITE, GetFPS());
    
    BegEnd(Mode2D,(obj->cam),())
    {
        {

            const Pos_Globale lu_ws = screen_to_Pos_Globale(
                obj->cam, 
                (Vector2){ 0, 0 }
            );
            const Pos_Globale rd_ws = screen_to_Pos_Globale(
                obj->cam, 
                (Vector2){ GetScreenWidth(), GetScreenHeight() }
            );
    
            for (int y =  rd_ws.chunk.y;
                        y <= lu_ws.chunk.y;
                        y++
            ) {
                for (int x =  lu_ws.chunk.x;
                            x <= rd_ws.chunk.x;
                            x++
                ) {
                    Item_chunks *chunk = set_Item_chunks_get(
                        &obj->wrd.chunks,
                        (Item_chunks){ .pos = (Pos){
                            .x = x,
                            .y = y
                        }}
                    );
                    if (chunk)
                    {
                        Chunk_draw(obj, chunk->data);
                        draw_count++;
                    }
                    if (CHUNK_POS_VALUE)
                        DrawTextF("(%d, %d)", 
                            x * CHUNK_WORLD_SIZE + 4,
                            -y * CHUNK_WORLD_SIZE + 4 - CHUNK_WORLD_SIZE,
                            4., PURPLE, x, y
                        );
                }
            }
            
        }

        if (obj->ui.mode == mode_editing)
        { // draw overed cell
            DrawRectangleLines(
                 obj->ui.mouse_pos.x * W,
                -obj->ui.mouse_pos.y * H - H,
                obj->texs.cell_size,
                obj->texs.cell_size,
                WHITE
            );
        }
        else if (obj->ui.mode == mode_select)
        { // draw selection
            if (obj->ui.select.mode == selection_selecting
             || obj->ui.select.mode == selection_seleted
            ) {
                DrawRectangleRec(
                    get_from_pos_to_pos(obj->texs.cell_size,
                        obj->ui.select.start,
                        obj->ui.select.end
                    ),
                    (Color){ 0, 121, 241, 32 }
                );
                DrawRectangleLinesEx(
                    get_from_pos_to_pos(obj->texs.cell_size,
                        obj->ui.select.start,
                        obj->ui.select.end
                    ),
                    2, 
                    (Color){ 0, 121, 241, 128 }
                );
            }
            else if (obj->ui.select.mode == selection_paste_preview)
            {
                DrawRectangleRec(
                    get_from_pos_to_pos(obj->texs.cell_size, 
                        obj->ui.select.start, 
                        obj->ui.select.end
                    ),
                    (Color){ 230, 41, 55, 32 }
                );
                DrawRectangleLinesEx(
                    get_from_pos_to_pos(obj->texs.cell_size,
                        obj->ui.select.start, 
                        obj->ui.select.end
                    ),
                    2, 
                    (Color){ 230, 41, 55, 128 }
                );
            }

        }
    }
    draw_Texs_Cell_V(
        &obj->texs,
        obj->ui.edit.current_state,
        (Rectangle){
            .x = 0, .y = 0,
            .width =  8 * obj->texs.cell_size,
            .height = 8 * obj->texs.cell_size,
        }
    );
}

