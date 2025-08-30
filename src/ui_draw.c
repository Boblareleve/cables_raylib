#include "main.h"


/* static void ui_select_draw(const Ui *ui, const Texs *texs, const float thickness)
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
            if (ui->edit.style_mode != edit_style_lign)
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
static inline void Ui_draw_main_menu(const Window *win, Ui *ui, const Texs *texs, float menu_padding, float menu_button_width, float menu_button_height)
{
    Rectangle button_bound = {
        .x = GetScreenWidth()  /2 / ui->in_game_ui_cam.zoom 
                - (menu_padding * (int)ceil(1.5) + menu_button_height * 2),
        .y = GetScreenHeight() /2 / ui->in_game_ui_cam.zoom
                - (menu_button_width / 2) + 4,
        .width = menu_button_width,
        .height = menu_button_height,
    };

    ui->menu.quit.bounds = button_bound;
    button_bound.y += menu_button_height + menu_padding;
    ui->menu.options.bounds = button_bound;
    button_bound.y += menu_button_height + menu_padding;
    ui->menu.load_file.bounds = button_bound;
    button_bound.y += menu_button_height + menu_padding;
    ui->menu.save_file.bounds = button_bound;

    foreach_static (size_t, i, ui->menu.buttons)
        draw_Button(ui->menu.buttons[i]);
}

 */

void Ui_draw_call(Ui_draw_data *draw)
{
    glBindBuffer(GL_ARRAY_BUFFER, draw->VBO);
    glBufferSubData(GL_ARRAY_BUFFER,
        0, draw->vertices.size * sizeof(draw->vertices.arr[0]),
        draw->vertices.arr
    );
    glBindVertexArray(draw->VAO);
    glDrawArrays(GL_POINTS, 0, draw->vertices.size);
}

void Ui_draw(const Window *win)
{
    Ui *ui = &win->ui;
    const Texs *texs = &win->texs; 
    
    
    // TODO
    // char std_fps[64];
    // snprintf(std_fps, sizeof(std_fps), "[FPS %d]", GetFPS());
    // DrawText(std_fps, GetScreenWidth() - MeasureText(std_fps, 24) - 10, 10, 24, WHITE);

    // Ui_cam_draw(ui, texs, cam);
    
    // const int padding = 4;
    // const int dim = 32;

    // StartBodyEnd(BeginMode2D(ui->in_game_ui_cam), EndMode2D())
    // {
    //     if (ui->mode == mode_editing)
    //     {
    //         // currently selected cell
            
    //         const float selected_scale = 4;
    //         Rectangle currently_selected_cell_rec = {
    //             .x = 6, .y = 6,
    //             .width =  selected_scale * texs->cell_size,
    //             .height = selected_scale * texs->cell_size,
    //         };
    //         DrawRectangleRec(currently_selected_cell_rec, win->background_color);
            
    //         Cell cell_to_draw = ui->edit.current_state;
    //         if (cell_to_draw == cable_off 
    //          || cell_to_draw == cable_on
    //         )
    //             cell_to_draw = cable_on | UP_MASK | RIGHT_MASK | DOWN_MASK | LEFT_MASK;
    //         if ((cell_to_draw & TYPE_MASK) == ty_transistor
    //          || (cell_to_draw & TYPE_MASK) == ty_not_gate
    //         )
    //             cell_to_draw |= var_on;

    //         draw_Texs_Cell_V(texs, cell_to_draw, currently_selected_cell_rec);
            

            
    //         Rectangle button_bound = (Rectangle){
    //             .x = selected_scale * texs->cell_size + 16,
    //             .y = padding,
    //             .width = dim,
    //             .height = dim
    //         };

            
    //         ui->edit.style_normal.bounds = button_bound;
    //         //  if (button(ui, uicam, button_bound, "#51#"))
    //         //     ui->edit.interpolation_mode = edit_style_normal;
            
    //         button_bound.x += dim + padding;
    //         ui->edit.style_orthogonal.bounds = button_bound;
    //         // if (button(ui, uicam, button_bound, "#68#"))
    //         //     ui->edit.style_mode = edit_style_orthogonal;
            
    //         button_bound.x += dim + padding;
    //         ui->edit.style_lign.bounds = button_bound;
    //         // if (button(ui, uicam, button_bound, "#69#"))
    //         //     ui->edit.style_mode = edit_style_lign;

    //         foreach_static (size_t, i, ui->edit.styles)
    //             draw_Button(ui->edit.styles[i]);

    //     }
     
    //     { // parmanent ui
    //         Rectangle button_bound = {
    //             .x = GetScreenWidth()  / ui->in_game_ui_cam.zoom - (padding * 3 + dim * 3),
    //             .y = GetScreenHeight() / ui->in_game_ui_cam.zoom - (padding + dim),
    //             .width = dim,
    //             .height = dim
    //         };
    
    //         ui->mode_idle.bounds = button_bound;
    //         button_bound.x += dim + padding;
    //         ui->mode_editing.bounds = button_bound;
    //         button_bound.x += dim + padding;
    //         ui->mode_select.bounds = button_bound;
            
    //         foreach_static (size_t, i, ui->modes)
    //             draw_Button(ui->modes[i]);
    //     }
    //     if (ui->in_main_menu)
    //     {
    //         DrawRectangle(
    //             0, 0,
    //             GetScreenWidth(),
    //             GetScreenHeight(),
    //             ui->menu.background_shade
    //         );

    //         const int menu_padding = 6;
    //         const int menu_button_width = 256;
    //         const int menu_button_height = 64;

    //         if (ui->menu.submenu == Submenu_self)
    //             Ui_draw_main_menu(win, ui, texs, menu_padding, menu_button_width, menu_button_height);
    //         else if (ui->menu.submenu == Submenu_option)
    //         {
                
    //         }
    //         else if (ui->menu.submenu == Submenu_load_file)
    //         {

    //         }
    //         else if (ui->menu.submenu == Submenu_save_file)
    //         {

    //         }
    //         else UNREACHABLE("ui.menu.submenu");
    //     }
    // }

    Ui_draw_call(&ui->draw);
}

