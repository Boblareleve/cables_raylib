#include "main.h"


void camera_mouvement_input(Window *win)
{
    const float mouv_speed = 5. * 60. / GetFPS() * (2. / win->cam.zoom);
    if (IsKeyDown(KEY_D))
    {
        printf("right"); 
        UPDATE_LINE;
        win->cam.target.x += mouv_speed;
    }
    if (IsKeyDown(KEY_A))
    {
        printf("left"); 
        UPDATE_LINE;
        win->cam.target.x -= mouv_speed;
    }
    if (IsKeyDown(KEY_W))
    {
        printf("up"); 
        UPDATE_LINE;
        win->cam.target.y -= mouv_speed;
    }    
    if (IsKeyDown(KEY_S))
    {
        printf("down"); 
        UPDATE_LINE;
        win->cam.target.y += mouv_speed;
    }
    
    win->cam.zoom = expf(logf(win->cam.zoom) + ((float)GetMouseWheelMove()*0.1f));
    if (GetMouseWheelMove() != 0.0f)
    {
        printf("zoom in or out"); 
        UPDATE_LINE;
    }
    
    const float max_zoom = 20.0f;
    const float min_zoom = 0.01f;
    if (win->cam.zoom > max_zoom) 
        win->cam.zoom = max_zoom;
    else if (win->cam.zoom < min_zoom)
        win->cam.zoom = min_zoom;
}

void edition_input(Window *win)
{
    Ui *ui = &win->ui;

    // tmp -> before ui
    if (IsKeyPressed(KEY_P))
    {
        ui->select.mode = selection_waiting_to_select;
        ui->mode = mode_select;
        printf("mode selection");
        UPDATE_LINE;
    }
    else if (IsKeyPressed(KEY_O))
    {
        ui->select.mode = selection_off;
        ui->mode = mode_editing;
        printf("mode editing");
        UPDATE_LINE;
    }
    else if (IsKeyPressed(KEY_I))
    {
        ui->select.mode = selection_off;
        ui->mode = mode_idle;
        printf("mode idle");
        UPDATE_LINE;
    }

    if (ui->mode == mode_editing)
    {
        if (IsKeyDown(KEY_ONE))
        {
            ui->edit.current_state = cable_off;
            printf("select cable");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_TWO))
        {
            ui->edit.current_state = ty_transistor 
                                    | var_off
                                    | ui->edit.current_direction;
            printf("select transistor on");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_THREE))
        {
            ui->edit.current_state = ty_not_gate
                                    | var_off
                                    | ui->edit.current_direction;
            printf("select not gate");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_FOUR))
        {
            ui->edit.current_state = bridge;
            printf("select bridge");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_ZERO))
        {
            ui->edit.current_state = cable_on;
            printf("select cable on");
            UPDATE_LINE;
        }
        if (IsKeyPressed(KEY_R))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT))
                ui->edit.current_direction = (ui->edit.current_direction - 1) % 4;
            else 
                ui->edit.current_direction = (ui->edit.current_direction + 1) % 4;
            
            if ((ui->edit.current_state & TYPE_MASK) == ty_transistor
                || (ui->edit.current_state & TYPE_MASK) == ty_not_gate)
                ui->edit.current_state = (ui->edit.current_state 
                                        & (TYPE_MASK | VARIANT_MASK)
                                    )  | ui->edit.current_direction;
        }
    
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            Pos_Globale gpos = Pos_to_Pos_Globale(ui->mouse_pos);
            
            /* TODO: interpolate to set all cell in mouse trace */
            (void)World_set_cell(
                &win->wrd,
                gpos.chunk,
                gpos.cell,
                ui->edit.current_state
            );
            printf("click!");
            UPDATE_LINE;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            Pos_Globale gpos = Pos_to_Pos_Globale(ui->mouse_pos);
            
            (void)World_set_cell(
                &win->wrd,
                gpos.chunk,
                gpos.cell,
                empty
            );
            printf("click!");
            UPDATE_LINE;
        }
    }

    if (ui->mode == mode_select)
    {
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
         && ui->select.mode == selection_waiting_to_select
        ) {
            ui->select.mode = selection_selecting;
            ui->select.start = ui->mouse_pos;
            ui->select.end = ui->mouse_pos;
        }
        if (ui->select.mode == selection_selecting)
        {
            ui->select.end = ui->mouse_pos;

            // if (ui->mouse_pos.x < ui->select.start.x)
            
                // SWAP(ui->mouse_pos.x, ui->select.start.x);
            // if (ui->select.end.y < ui->select.start.y)
            //     SWAP(ui->select.end.y, ui->select.start.y);
            
            printf("select start %i,%i | ", ui->select.start.x, ui->select.start.y);
            printf("select end %i,%i", ui->select.end.x, ui->select.end.y);
            UPDATE_LINE;
        }
         if (ui->select.mode == selection_selecting
          && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
        ) {
            ui->select.mode = selection_seleted;
        }
        if (IsKeyDown(KEY_LEFT_CONTROL))
        {
            if (IsKeyPressed(KEY_C)
             && ui->select.mode != selection_off 
             && ui->select.mode != selection_waiting_to_select
            ) { // ctrl+c
                save_pattern(
                    &win->wrd, 
                    (
                        ui->select.clipboard.size = 0,
                        &ui->select.clipboard
                    ),
                    "clipboard",
                    ui->select.start,
                    ui->select.end
                );
            }
            else if (IsKeyPressed(KEY_V))
            { // ctrl+v
                ui->select.mode = selection_paste_preview;
                Pattern_header header = get_pattern_header(ui->select.clipboard.self);
                ui->select.start.x = ui->mouse_pos.x;
                ui->select.start.y = ui->mouse_pos.y - header.height + 1;
                ui->select.end.x = ui->mouse_pos.x + header.width - 1;
                ui->select.end.y = ui->mouse_pos.y;
            }
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
         && ui->select.mode == selection_paste_preview
        ) {
            add_pattern(
                &win->wrd,
                ui->select.start,
                ui->select.clipboard.self,
                &ui->select.msg_clipboard
            );
            ui->select.mode = selection_waiting_to_select;
        }
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
         && ui->select.mode == selection_seleted
        ) // cancel/end the selection
            ui->select.mode = selection_waiting_to_select;
        
        
    }
}

void simulation_input(Window *win)  
{ 
    if (IsKeyPressed(KEY_LEFT_ALT))
    {
        printf("play one tick...");
        fflush(stdout);
        
        World_tick(&win->wrd);

        printf("\b\b\b"); // cut the ...
        UPDATE_LINE;
    }
}


void inputs(Window *win)
{
    win->ui.mouse_pos = screen_to_Pos(win->cam, GetMousePosition());
    
    camera_mouvement_input(win);
    
    edition_input(win);
    
    simulation_input(win);
}

Ui Ui_make(Camera2D cam)
{
    return (Ui){ 
        .mode = mode_idle,
        .mouse_pos = screen_to_Pos(cam, GetMousePosition()),
        .edit = (Edit_ui){
            .current_state = cable_off,
            .current_direction = up,
        },
        .select = (Select_ui){
            .mode = selection_off,
            .clipboard = (Strb){0},
            .msg_clipboard = (Strb){0}
        }
    };
}

void Ui_free(Ui *ui)
{
    Strb_free(ui->select.clipboard);
    Strb_free(ui->select.msg_clipboard);
}


