#include "main.h"


#ifdef REMANENT_CURRENT
# define REMANENT_CURRENT_VALUE 1
#else 
# define REMANENT_CURRENT_VALUE 0
#endif /* REMANENT_CURRENT */


#ifndef SELECTION_ANIMATION_SPEED
# define SELECTION_ANIMATION_SPEED 40.
#endif /* SELECTION_ANIMATION_SPEED */


/* void ui_state_print(const Ui *ui)
{
    printf("ui state:\n");
    switch (ui->mode)
    {
    case mode_idle:
        printf("\tui idle\n");
        break;
    case mode_editing:
        printf("\tui editing:\n");
        printf("\t\tcurrent state %d\n", ui->edit.current_state);
        printf("\t\tcurrent direction %d\n", ui->edit.current_direction);
        break;
    case mode_select:
        printf("\tui select:\n");
        switch (ui->select.mode)
        {
        case selection_off:
            UNREACHABLE("OFF");
            break;
        case selection_waiting:
            printf("\t\twaiting to select\n");
            assert(ui->select.mode_sides_resize == extend_resize_none);
            break;
        case selection_selecting:
            printf("\t\tselecting:\n");
            printf("\t\tform %i,%i to %i,%i\n", 
                ui->select.start.x, ui->select.start.y,
                ui->select.end.x, ui->select.end.y
            );
            assert(ui->select.mode_sides_resize == extend_resize_none);
            break;
        case selection_selected:
            printf("\t\tselected:\n");
            printf("\t\tform %i,%i to %i,%i\n", 
                ui->select.start.x, ui->select.start.y,
                ui->select.end.x, ui->select.end.y
            );
            assert(ui->select.mode_sides_resize == extend_resize_none);
            break;
        case selection_paste_preview:
            printf("\t\tpaste preview:\n");
            printf("\t\tfrom %i,%i to %i,%i\n", 
                ui->select.start.x, ui->select.start.y,
                ui->select.end.x, ui->select.end.y
            );
            assert(ui->select.mode_sides_resize == extend_resize_none);
            break;
        case selection_resizing:
            printf("\t\tresizing:\n");
            printf("\t\tfrom %i,%i to %i,%i\n", 
                ui->select.start.x, ui->select.start.y,
                ui->select.end.x, ui->select.end.y
            );
            switch (ui->select.mode_sides_resize)
            {
            case extend_resize_up:
                printf("\t\t\textend resize up\n");
                break;
            case extend_resize_right:
                printf("\t\t\textend resize right\n");
                break;
            case extend_resize_down:
                printf("\t\t\textend resize down\n");
                break;
            case extend_resize_left:
                printf("\t\t\textend resize left\n");
                break;
            default:
                assert(ui->select.mode_sides_resize == extend_resize_none);
                printf("\t\t\tEXTEND RESIZE NONE\n");
                break;
            }
            break;
        
        default:
            printf("\t\tDEFAULT\n");
            break;
        }
        printf("\t\tclip board character count: %d\n", ui->select.clipboard.size);
        printf("\t\tclip board msg: \"");
        Str_print(ui->select.msg_clipboard);
        printf("\"\n");
        break;
    
    default:
        break;
    }
}
 */



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

static bool is_mouse_colliding_Rec(Window *win, Rectangle rec)
{
    const Vector2 mouse = GetScreenToWorld2D(GetMousePosition(), win->cam);
    const bool res = CheckCollisionPointRec(mouse, rec);

    return res;
}

static void state_show_resize(Window *win)
{
    Ui *ui = &win->ui;

    if (GET_WIDTH(ui->select.start, ui->select.end) >= 3
        && GET_HEIGHT(ui->select.start, ui->select.end) >= 2
    ) {
        ui->select.mode_extend_sides |= extend_sides_horizontal;
    }
    else ui->select.mode_extend_sides &= ~extend_sides_horizontal;
    if (GET_HEIGHT(ui->select.start, ui->select.end) >= 3
        && GET_WIDTH(ui->select.start, ui->select.end) >= 2
    ) {
        ui->select.mode_extend_sides |= extend_sides_vertical;
    }
    else ui->select.mode_extend_sides &= ~extend_sides_vertical;
}

void set_to_paste_preview(Ui *ui)
{
    Pos mid = (Pos){
        ui->mouse_pos.x - ui->select.paste_header_width  / 2,
        ui->mouse_pos.y - ui->select.paste_header_height / 2,
    };
    
    ui->select.start = mid;
    ui->select.end = (Pos){
        mid.x + ui->select.paste_header_width  - 1,
        mid.y + ui->select.paste_header_height - 1
    };

}

void edition_input(Window *win)
{
    Ui *ui = &win->ui;
    
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

void update_current_select(Window *win)
{
    Ui *ui = &win->ui;

    const float frame_delta = GetFrameTime();
    const Vector2 target_start = POS_TO_VECTOR2(win->ui.select.start);
    const Vector2 target_end   = POS_TO_VECTOR2(win->ui.select.end  );

    // every 100ms  -> frame_delta = 0.1 s
    // /2 distance to target
    const float speed = SELECTION_ANIMATION_SPEED;

    ui->select.current_start.x += speed * (target_start.x - ui->select.current_start.x) * frame_delta;
    ui->select.current_start.y += speed * (target_start.y - ui->select.current_start.y) * frame_delta;
    
    ui->select.current_end.x += speed * (target_end.x - ui->select.current_end.x) * frame_delta;
    ui->select.current_end.y += speed * (target_end.y - ui->select.current_end.y) * frame_delta;
    
    ui->select.selection_box = get_rec_from_pos_to_pos(win->texs.cell_size, 
        ui->select.current_start, ui->select.current_end
    );
    
    if (ui->select.mode & selection_show_blue)
    { // side resize

        const float win_wh = GET_MIN_WIDTH_HEIGHT(ui->select.current_start, ui->select.current_end);
        const float cell_size = win->texs.cell_size * (win_wh / 10 + 1);
        const float padding = MOUV_PADDING * H;
        const float width   = cell_size * .5;

        if (ui->select.mode_extend_sides & extend_sides_horizontal)
        {
            ui->select.sides_vertices[up] = (Rectangle){
                .x = ui->select.selection_box.x + cell_size,
                .y = ui->select.selection_box.y + padding,
                .width = ui->select.selection_box.width - 2* cell_size,
                .height = width,
            };
            ui->select.sides_vertices[down] = (Rectangle){
                .x = ui->select.selection_box.x + cell_size,
                .y = ui->select.selection_box.y + ui->select.selection_box.height 
                                                - padding - width,
                .width = ui->select.selection_box.width - 2* cell_size,
                .height = width,
            };
        }
        if (ui->select.mode_extend_sides & extend_sides_vertical)
        {
            ui->select.sides_vertices[right] = (Rectangle){
                .x = ui->select.selection_box.x + ui->select.selection_box.width 
                                                - padding - width,
                .y = ui->select.selection_box.y + cell_size,
                .width = width,
                .height = ui->select.selection_box.height - 2* cell_size,
            };
            ui->select.sides_vertices[left] = (Rectangle){
                .x = ui->select.selection_box.x + padding,
                .y = ui->select.selection_box.y + cell_size,
                .width = width,
                .height = ui->select.selection_box.height - 2* cell_size,
            };
        }
    }
    else if (ui->select.mode & selection_show_red)
    { // paste area
        const float min_wh = MIN(
            ui->select.selection_box.width,
            ui->select.selection_box.height
        );
        ui->select.paste_vertices = (Rectangle){
            .x = ui->select.selection_box.x + ui->select.selection_box.width  / 2. 
                                            - min_wh / 4.,
            .y = ui->select.selection_box.y + ui->select.selection_box.height / 2. 
                                            - min_wh / 4.,
            .width  = min_wh / 2.,
            .height = min_wh / 2.,
        };
    }
}
void select_input(Window *win)
{
    Ui *ui = &win->ui;
    
    if (ui->select.mode == selection_waiting
        && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) {
        ui->select.mode          = selection_selecting;
        ui->select.start         = ui->mouse_pos;
        ui->select.end           = ui->mouse_pos;
        
        if (!REMANENT_CURRENT_VALUE)
        {
            ui->select.current_start = POS_TO_VECTOR2(ui->mouse_pos);
            ui->select.current_end   = POS_TO_VECTOR2(ui->mouse_pos);
        }
        
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            ui->select.mode = selection_selected;
    }
    else if (ui->select.mode == selection_selecting
            && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)
    ) {
        ui->select.end = ui->mouse_pos;
        ui->select.mode = selection_selected;
        state_show_resize(win);
    }
    else if (ui->select.mode == selection_selecting)
    {
        ui->select.end = ui->mouse_pos;
        state_show_resize(win);
    }
    else if (ui->select.mode == selection_selected
            && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) { // cancel/end the selection
        if (is_mouse_colliding_Rec(win, ui->select.sides_vertices[up]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_up;
        }
        else if (is_mouse_colliding_Rec(win, ui->select.sides_vertices[right]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_right;
        }
        else if (is_mouse_colliding_Rec(win, ui->select.sides_vertices[down]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_down;
        }
        else if (is_mouse_colliding_Rec(win, ui->select.sides_vertices[left]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_left;
        }
        else
        {
            ui->select.mode_sides_resize = extend_resize_none;
            ui->select.mode = selection_waiting;
        }
    }
    else if (ui->select.mode == selection_paste_preview
            && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
    ) {
        if (is_mouse_colliding_Rec(win, ui->select.paste_vertices))
            ui->select.mode = selection_paste_mouv_preview;
        else
        {
            ui->select.mode = selection_waiting;
            add_pattern(
                &win->wrd,
                ui->select.start,
                ui->select.clipboard.self,
                &ui->select.msg_clipboard
            );
        }
    }
    else if (ui->select.mode == selection_paste_mouv_preview
            && IsMouseButtonDown(MOUSE_BUTTON_LEFT)
    ) {
        ui->select.mode = selection_paste_mouv_preview;
        set_to_paste_preview(ui);
    }
    else if (ui->select.mode == selection_paste_mouv_preview
            && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        ui->select.mode = selection_paste_preview;
    else if (ui->select.mode == selection_resizing
            && ui->select.mode_sides_resize != extend_resize_none
            && IsMouseButtonDown(MOUSE_BUTTON_LEFT)
    ) {
        switch (ui->select.mode_sides_resize)
        {
        case extend_resize_up:
            if (ui->select.end.y < ui->select.start.y)
                ui->select.start.y = ui->mouse_pos.y;
            else
                ui->select.end.y  = ui->mouse_pos.y;
            break;
        case extend_resize_right:
            if (ui->select.end.x < ui->select.start.x)
                ui->select.start.x = ui->mouse_pos.x;
            else
                ui->select.end.x  = ui->mouse_pos.x;
            break;
        case extend_resize_down:
            if (ui->select.end.y > ui->select.start.y)
                ui->select.start.y = ui->mouse_pos.y;
            else
                ui->select.end.y  = ui->mouse_pos.y;
            break;
        case extend_resize_left:
            if (ui->select.end.x > ui->select.start.x)
                ui->select.start.x = ui->mouse_pos.x;
            else
                ui->select.end.x = ui->mouse_pos.x;
            break;
        default:
            UNREACHABLE("");
            break;
        }
        
        state_show_resize(win);
    }
    else if (ui->select.mode_sides_resize != extend_resize_none
            && IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
    {
        ui->select.mode_sides_resize = extend_resize_none;
        ui->select.mode = selection_selected;
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        ui->select.mode = selection_waiting;
    else if (IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (IsKeyPressed(KEY_C) 
            && (  ui->select.mode == selection_selected
            || ui->select.mode == selection_selecting
            || ui->select.mode == selection_resizing)
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
            ui->select.paste_header_width =  ABS(ui->select.end.x - ui->select.start.x) + 1;
            ui->select.paste_header_height = ABS(ui->select.end.y - ui->select.start.y) + 1;
            
            ui->select.mode = selection_waiting;
        }
        else if (IsKeyPressed(KEY_V)) // ctrl+v
        {
            ui->select.mode = selection_paste_preview;
            set_to_paste_preview(ui);
            
            // start small only the first time
            ui->select.current_start = POS_TO_VECTOR2(ui->mouse_pos);
            ui->select.current_end   = POS_TO_VECTOR2(ui->mouse_pos);
        }
        
    }

    if (win->ui.select.mode & (selection_show_blue | selection_show_red))
        update_current_select(win);
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



// TODO: 
//    FIXS:
//     - when resizing from up passed the down of the selection the entier selection folow down
//       same for all direction
//     - flickering of the resize bar on the previous bug
//    FEATURES:
//     - event queue

void inputs(Window *win)
{
    win->ui.mouse_pos = screen_to_Pos(win->cam, GetMousePosition());
    Ui *ui = &win->ui;

    // tmp -> before ui
    if (IsKeyPressed(KEY_P))
    {
        ui->select.mode = selection_waiting;
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


    camera_mouvement_input(win);
    
    if (ui->mode == mode_editing)
        edition_input(win);
    else if (ui->mode == mode_select)
        select_input(win);
    else assert(ui->mode == mode_idle);

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
            .mode_sides_resize = extend_resize_none,
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

