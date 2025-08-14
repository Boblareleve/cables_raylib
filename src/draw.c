#include "main.h"


Texture2D load_tex(const char *name)
{
    Texture2D res = LoadTexture(name);
    assert(IsTextureValid(res));
    return res;
}


Texs Texs_make(const char *atlas_name, const char *paste_mouver_name)
{
    Texs res = {
        .tex = load_tex(atlas_name),
        .paste_mouver = load_tex(paste_mouver_name),
        .cell_size = W
    };

    SetTextureFilter(res.tex, TEXTURE_FILTER_POINT);
    // SetTextureFilter(res.tex, TEXTURE_FILTER_BILINEAR);
    // SetTextureFilter(res.tex, TEXTURE_FILTER_TRILINEAR);       // Trilinear filtering (linear with mipmaps)
    // SetTextureFilter(res.tex, TEXTURE_FILTER_ANISOTROPIC_4X);  // Anisotropic filtering 4x
    // SetTextureFilter(res.tex, TEXTURE_FILTER_ANISOTROPIC_8X);  // Anisotropic filtering 8x
    // SetTextureFilter(res.tex, TEXTURE_FILTER_ANISOTROPIC_16X); // Anisotropic filtering 16x

    return res;
}
void Texs_free(Texs *texs)
{
    UnloadTexture(texs->tex);
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

void Chunk_draw(const Window *win, const Chunk *chunk)
{
    if (chunk == NULL)
        return ;
    
    Vector2 chunk_pos = (Vector2){
         chunk->pos.x * CHUNK_WORLD_SIZE, 
        -chunk->pos.y * CHUNK_WORLD_SIZE
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
                chunk->arr2D[x][y],
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
            4., PURPLE, chunk->pos.x, chunk->pos.y
        );
}




static void ui_select_draw(const Ui *ui, const Texs *texs, const float thickness)
{
    UNUSED(texs);

    DrawRectangleRec(
        ui->select.selection_box,
        SELECTION_COLOR
    );
    DrawRectangleLinesEx(
        ui->select.selection_box,
        2.0f  * thickness, 
        SELECTION_COLOR_OUTLINE
    );

    for (Direction dir = 0; dir < (ssize_t)ARRAY_LEN(ui->select.sides_vertices); dir++)
    {
        if (
            ((dir == up   || dir == down ) && (ui->select.mode_extend_sides & extend_sides_horizontal))
         || ((dir == left || dir == right) && (ui->select.mode_extend_sides & extend_sides_vertical))
        )
            DrawRectangleRec(
                ui->select.sides_vertices[dir], 
                SELECTION_COLOR_OUTLINE
            );
    }
}

static void ui_paste_previw_draw(const Ui *ui, const Texs *texs, const float thickness)
{
    DrawRectangleRec(
        ui->select.selection_box,
        PASTE_SELECTION_COLOR
    );
    DrawRectangleLinesEx(
        ui->select.selection_box,
        2.0 * thickness,
        PASTE_SELECTION_COLOR_OUTLINE
    );
    
    DrawTexturePro(texs->paste_mouver,
        (Rectangle){
            .x = 0, 
            .y = 0, 
            .width = texs->paste_mouver.width, 
            .height = texs->paste_mouver.height
        },
        ui->select.paste_vertices,
        (Vector2){0},
        0, PASTE_SELECTION_COLOR_OUTLINE
    );
}

static inline void Ui_cam_draw(Ui *ui, const Texs *texs, const Camera2D cam)
{
    StartBodyEnd(BeginMode2D(cam), EndMode2D())
    {
        const float thickness = fmaxf(1.0f, 1.0f / cam.zoom);
        if (ui->mode == mode_editing)
        { // draw overed cell
            if (ui->edit.interpolation_mode != interpolation_lign)
                DrawRectangleLinesEx(
                    get_rec_from_pos_to_pos(
                        texs->cell_size,
                        ui->edit.current_drag_start,
                        ui->edit.current_drag_end
                    ), thickness, WHITE
                );
            else
            {
                DrawRectangleLinesEx(
                    (Rectangle){
                        ui->edit.current_drag_end.x * W,
                        -ui->edit.current_drag_end.y * H - H,
                        texs->cell_size,
                        texs->cell_size,
                    }, thickness, RED
                );
                DrawRectangleLinesEx(
                    (Rectangle){
                        ui->edit.current_drag_start.x * W,
                        -ui->edit.current_drag_start.y * H - H,
                        texs->cell_size,
                        texs->cell_size,
                    }, thickness, BLUE
                );
            }
        }
        else if (ui->mode == mode_select)
        { // draw selection
            if (ui->select.mode & selection_show_blue) 
                ui_select_draw(ui, texs, thickness);
            else if (ui->select.mode & selection_show_red)
                ui_paste_previw_draw(ui, texs, thickness);
        }
    }
}
static inline void Ui_draw(const Window *win, Ui *ui, const Texs *texs, const Camera2D cam)
{
    char std_fps[64];
    snprintf(std_fps, sizeof(std_fps), "[FPS %d]", GetFPS());
    DrawText(std_fps, GetScreenWidth() - MeasureText(std_fps, 24) - 10, 10, 24, WHITE);

    Ui_cam_draw(ui, texs, cam);

    const int padding = 4;
    const int dim = 32;
    const Camera2D uicam = { .zoom = 2.0f };
    StartBodyEnd(BeginMode2D(uicam), EndMode2D())
    {
        
        if (ui->mode == mode_editing)
        {
            // currently selected cell
            
            const float selected_scale = 4;
            Rectangle currently_selected_cell_rec = {
                .x = 6, .y = 6,
                .width =  selected_scale * texs->cell_size,
                .height = selected_scale * texs->cell_size,
            };
            DrawRectangleRec(currently_selected_cell_rec, win->background_color);
            
            Cell cell_to_draw = ui->edit.current_state;
            if (cell_to_draw == cable_off 
             || cell_to_draw == cable_on
            )
                cell_to_draw = cable_on | UP_MASK | RIGHT_MASK | DOWN_MASK | LEFT_MASK;
            if ((cell_to_draw & TYPE_MASK) == ty_transistor
             || (cell_to_draw & TYPE_MASK) == ty_not_gate
            )
                cell_to_draw |= var_on;

            draw_Texs_Cell_V(texs, cell_to_draw, currently_selected_cell_rec);
            

            
            Rectangle button_bound = (Rectangle){
                .x = selected_scale * texs->cell_size + 16,
                .y = padding,
                .width = dim,
                .height = dim
            };

            
            if (button(ui, uicam, button_bound, "#51#"))
                ui->edit.interpolation_mode = interpolation_none;
            button_bound.x += dim + padding;
            if (button(ui, uicam, button_bound, "#68#"))
                ui->edit.interpolation_mode = interpolation_orthogonal;
            button_bound.x += dim + padding;
            if (button(ui, uicam, button_bound, "#69#"))
                ui->edit.interpolation_mode = interpolation_lign;

        }
     
        
        Rectangle button_bound = {
            .x = GetScreenWidth()  / uicam.zoom - (padding * 3 + dim * 3),
            .y = GetScreenHeight() / uicam.zoom - (padding + dim),
            .width = dim,
            .height = dim
        };

        ui->mode_idle.bound = button_bound;
        button_bound.x += dim + padding;
        ui->mode_editing.bound = button_bound;
        button_bound.x += dim + padding;
        ui->mode_select.bound = button_bound;
        
        foreach_static (i, ui->modes)
            draw_Button(ui->modes[i]);
    }
}

void Window_draw(const Window *win, Ui *ui)
{
    int draw_count = 0;
    

    ClearBackground(win->background_color);
    static int bmode = 0;

    
    // StartBodyEnd(BeginTextureMode(), EndTextureMode())
    // StartBodyEnd(BeginBlendMode(bmode), EndBlendMode())
    {
        StartBodyEnd(BeginMode2D(win->cam), EndMode2D())
        {
            const Pos lu_ws = get_chunk_Pos(screen_to_Pos(win->cam, (Vector2){ 0, 0 }));
            const Pos rd_ws = get_chunk_Pos(screen_to_Pos(win->cam, (Vector2){ GetScreenWidth(), GetScreenHeight() }));
    
            for (int y = rd_ws.y; y <= lu_ws.y; y++)
            {
                for (int x = lu_ws.x; x <= rd_ws.x; x++)
                {
                    Item_chunks *chunk = set_Item_chunks_get(
                        &win->wrd.chunks,
                        (Item_chunks){ .pos = (Pos){ x, y }}
                    );
                    if (chunk)
                    {
                        Chunk_draw(win, chunk->data);
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
        Ui_draw(win, ui, &win->texs, win->cam);
    }
    bmode = (bmode + IsKeyPressed(KEY_B)) % 4;
}

