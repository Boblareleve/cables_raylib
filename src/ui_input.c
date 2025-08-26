#include "main.h"


#ifdef REMANENT_CURRENT
# define REMANENT_CURRENT_VALUE 1
#else 
# define REMANENT_CURRENT_VALUE 0
#endif /* REMANENT_CURRENT */


#ifndef SELECTION_ANIMATION_SPEED
# define SELECTION_ANIMATION_SPEED 40.
#endif /* SELECTION_ANIMATION_SPEED */


 
static inline Vec2 get_new_current(Vec2 current, Vec2 target, float speed)
{
    Vec2 res = {
        current.x + speed * (target.x - current.x) * rd_get_delta_time(),
        current.y + speed * (target.y - current.y) * rd_get_delta_time()
    };
    // to avoid jigle 
    if (current.x < target.x && res.x > target.x)
        res.x = target.x;
    if (current.x > target.x && res.x < target.x)
        res.x = target.x;
    if (current.y < target.y && res.y > target.y)
        res.y = target.y;
    if (current.y > target.y && res.y > target.y)
        res.y = target.y;
    
    return res;
}
void update_current_select(Window *win)
{
    Ui *ui = &win->ui;

    const Vec2 target_start = POS_TO_VEC2(win->ui.select.start);
    const Vec2 target_end   = POS_TO_VEC2(win->ui.select.end  );

    // every 100ms  -> frame_delta = 0.1 s
    // /2 distance to target
    const float speed = SELECTION_ANIMATION_SPEED;

    ui->select.current_start = get_new_current(ui->select.current_start, target_start, speed);
    ui->select.current_end =   get_new_current(ui->select.current_end,   target_end,   speed);

    // ui->select.current_start.x += speed * (target_start.x - ui->select.current_start.x) * frame_delta;
    // ui->select.current_start.y += speed * (target_start.y - ui->select.current_start.y) * frame_delta;
    // ui->select.current_end.x += speed * (target_end.x - ui->select.current_end.x) * frame_delta;
    // ui->select.current_end.y += speed * (target_end.y - ui->select.current_end.y) * frame_delta;
    
    
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
            ui->select.sides_vertices[up] = (Rect){
                .x = ui->select.selection_box.x + cell_size,
                .y = ui->select.selection_box.y + padding,
                .width = ui->select.selection_box.width - 2* cell_size,
                .height = width,
            };
            ui->select.sides_vertices[down] = (Rect){
                .x = ui->select.selection_box.x + cell_size,
                .y = ui->select.selection_box.y + ui->select.selection_box.height 
                                                - padding - width,
                .width = ui->select.selection_box.width - 2* cell_size,
                .height = width,
            };
        }
        if (ui->select.mode_extend_sides & extend_sides_vertical)
        {
            ui->select.sides_vertices[right] = (Rect){
                .x = ui->select.selection_box.x + ui->select.selection_box.width 
                                                - padding - width,
                .y = ui->select.selection_box.y + cell_size,
                .width = width,
                .height = ui->select.selection_box.height - 2* cell_size,
            };
            ui->select.sides_vertices[left] = (Rect){
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
        ui->select.paste_vertices = (Rect){
            .x = ui->select.selection_box.x + ui->select.selection_box.width  / 2. 
                                            - min_wh / 4.,
            .y = ui->select.selection_box.y + ui->select.selection_box.height / 2. 
                                            - min_wh / 4.,
            .width  = min_wh / 2.,
            .height = min_wh / 2.,
        };
    }
}
void update_current_drag(Window *win)
{
    Ui *ui = &win->ui;

    const Vec2 target_start = POS_TO_VEC2(win->ui.edit.drag_start);
    const Vec2 target_end   = POS_TO_VEC2(win->ui.edit.drag_end  );

    // every 100ms  -> frame_delta = 0.1 s
    // /2 distance to target
    const float speed = SELECTION_ANIMATION_SPEED;

    ui->edit.current_drag_start = get_new_current(ui->edit.current_drag_start, target_start, speed);
    ui->edit.current_drag_end   = get_new_current(ui->edit.current_drag_end,   target_end,   speed);
}



static float *zoom_ptr = NULL;
void zoom_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    UNUSED(window);
    *zoom_ptr = MIN(
        MAX(
            expf(log(*zoom_ptr) + ( yoffset + xoffset ) * 0.1),
            // 0.01f
            0.00001f
        ),
        200.0f
    );
    printf("zoom in or out (zoom: %f)", *zoom_ptr);
    UPDATE_LINE;
}

void camera_mouvement_input(Window *win)
{
    const float mouv_speed = 1. / win->render->frame_time.fps
                                / win->wrd_render.cam.zoom;
    // const float mouv_speed = 1.0 / (win->wrd_render.cam.zoom * 16.0);
    if (rd_is_key_down(GLFW_KEY_D))
    {
        win->wrd_render.cam.target.x += mouv_speed;
        printf("right cam target.x = %f", win->wrd_render.cam.target.x);
        UPDATE_LINE;
    }
    if (rd_is_key_down(GLFW_KEY_A))
    {
        win->wrd_render.cam.target.x -= mouv_speed;
        printf("left cam target.x = %f", win->wrd_render.cam.target.x);
        UPDATE_LINE;
    }
    if (rd_is_key_down(GLFW_KEY_W))
    {
        win->wrd_render.cam.target.y -= mouv_speed;
        printf("up cam target.y = %f", win->wrd_render.cam.target.y);
        UPDATE_LINE;
    }    
    if (rd_is_key_down(GLFW_KEY_S))
    {
        win->wrd_render.cam.target.y += mouv_speed;
        printf("down cam target.y = %f", win->wrd_render.cam.target.y);
        UPDATE_LINE;
    }
    zoom_ptr = &win->wrd_render.cam.zoom;
}

static bool is_mouse_colliding_World_Rec(Window *win, Rect rec)
{
    return rd_collision_Vec2_Rect(
        rd_get_screen_to_world(win->wrd_render.cam, rd_get_cursor_pos()),
        rec 
    );
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

    if (rd_Button(&ui->edit.style_normal))
        ui->edit.style_mode = edit_style_normal;
    if (rd_Button(&ui->edit.style_orthogonal))
        ui->edit.style_mode = edit_style_orthogonal;
    if (rd_Button(&ui->edit.style_lign))
        ui->edit.style_mode = edit_style_lign;
    
    
    if (rd_is_key_pressed(GLFW_KEY_K))
    {
        ui->edit.style_mode = edit_style_normal;
        printf("interpolation none");
        UPDATE_LINE;
    }
    else if (rd_is_key_pressed(GLFW_KEY_SEMICOLON))
    {
        ui->edit.style_mode = edit_style_lign;
        printf("interpolation lign (4-ways)");
        UPDATE_LINE;
    }
    else if (rd_is_key_pressed(GLFW_KEY_L))
    {
        ui->edit.style_mode = edit_style_orthogonal;
        printf("interpolation orthogonal lign");
        UPDATE_LINE;
    }
    


    if (rd_is_key_down(GLFW_KEY_1))
    {
        ui->edit.current_state = cable_off;
        printf("select cable");
        UPDATE_LINE;
    }
    if (rd_is_key_down(GLFW_KEY_2))
    {
        ui->edit.current_state = ty_transistor 
                                | var_off
                                | ui->edit.current_direction;
        printf("select transistor on");
        UPDATE_LINE;
    }
    if (rd_is_key_down(GLFW_KEY_3))
    {
        ui->edit.current_state = ty_not_gate
                                | var_off
                                | ui->edit.current_direction;
        printf("select not gate");
        UPDATE_LINE;
    }
    if (rd_is_key_down(GLFW_KEY_4))
    {
        ui->edit.current_state = bridge;
        printf("select bridge");
        UPDATE_LINE;
    }
    if (rd_is_key_down(GLFW_KEY_0))
    {
        ui->edit.current_state = cable_on;
        printf("select cable on");
        UPDATE_LINE;
    }
    if (rd_is_key_pressed(GLFW_KEY_R))
    {
        if (rd_is_key_down(GLFW_KEY_LEFT_SHIFT))
            ui->edit.current_direction = (ui->edit.current_direction == 0 
                                            ? 3 : (ui->edit.current_direction - 1));
        else
            ui->edit.current_direction = (ui->edit.current_direction + 1) % 4;
        
        if ((ui->edit.current_state & TYPE_MASK) == ty_transistor
            || (ui->edit.current_state & TYPE_MASK) == ty_not_gate)
            ui->edit.current_state = (ui->edit.current_state 
                                    & (TYPE_MASK | VARIANT_MASK)
                                )  | ui->edit.current_direction;
    }

    if (ui->edit.style_mode == edit_style_normal
     && rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        World_set_lign_cell(
            &win->wrd,
            ui->old_mouse_pos,
            ui->mouse_pos,
            ui->edit.current_state
        );
    }
    if (ui->edit.style_mode == edit_style_orthogonal
     && rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        World_set_orth_lign_cell(
            &win->wrd,
            ui->edit.drag_start, ui->edit.drag_end,
            ui->edit.current_state
        );
    }
    if (ui->edit.style_mode == edit_style_lign
     && rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        World_set_lign_cell(
            &win->wrd,
            ui->edit.drag_start, ui->edit.drag_end,
            ui->edit.current_state
        );
    }

    if (ui->edit.style_mode == edit_style_orthogonal
     && rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        if (ABS(ui->mouse_pos.x - ui->edit.drag_start.x) 
         >  ABS(ui->mouse_pos.y - ui->edit.drag_start.y))
            ui->edit.drag_end = (Pos){
                ui->mouse_pos.x,
                ui->edit.drag_start.y
            };
        else ui->edit.drag_end = (Pos){
                ui->edit.drag_start.x,
                ui->mouse_pos.y
            };
    }
    else if (ui->edit.style_mode == edit_style_lign
     && rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT))
        ui->edit.drag_end = ui->mouse_pos;
    else ui->edit.drag_start = ui->edit.drag_end 
                             = ui->mouse_pos;
    
    
    if (rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_RIGHT))
    {
        World_set_cell(
            &win->wrd,
            ui->mouse_pos,
            empty
        );
        printf("erase!");
        UPDATE_LINE;
    }


    update_current_drag(win);
}


void select_input(Window *win)
{
    Ui *ui = &win->ui;
    
    if (ui->select.mode == selection_waiting
        && rd_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        ui->select.mode          = selection_selecting;
        ui->select.start         = ui->mouse_pos;
        ui->select.end           = ui->mouse_pos;
        
        if (!REMANENT_CURRENT_VALUE)
        {
            ui->select.current_start = POS_TO_VEC2(ui->mouse_pos);
            ui->select.current_end   = POS_TO_VEC2(ui->mouse_pos);
        }
        
        if (rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT))
            ui->select.mode = selection_selected;
    }
    else if (ui->select.mode == selection_selecting
          && rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT)
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
            && rd_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)
    ) { // cancel/end the selection
        if (is_mouse_colliding_World_Rec(win, ui->select.sides_vertices[up]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_up;
        }
        else if (is_mouse_colliding_World_Rec(win, ui->select.sides_vertices[right]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_right;
        }
        else if (is_mouse_colliding_World_Rec(win, ui->select.sides_vertices[down]))
        {
            ui->select.mode = selection_resizing;
            ui->select.mode_sides_resize = extend_resize_down;
        }
        else if (is_mouse_colliding_World_Rec(win, ui->select.sides_vertices[left]))
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
            && rd_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        if (is_mouse_colliding_World_Rec(win, ui->select.paste_vertices))
            ui->select.mode = selection_paste_mouv_preview;
        else
        {
            add_pattern(
                &win->wrd,
                ui->select.start,
                ui->select.clipboard.self,
                &ui->select.msg_clipboard
            );
            ui->select.mode = selection_waiting;
        }
    }
    else if (ui->select.mode == selection_paste_mouv_preview
            && rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)
    ) {
        ui->select.mode = selection_paste_mouv_preview;
        set_to_paste_preview(ui);
    }
    else if (ui->select.mode == selection_paste_mouv_preview
            && rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT))
        ui->select.mode = selection_paste_preview;
    else if (ui->select.mode == selection_resizing
            && ui->select.mode_sides_resize != extend_resize_none
            && rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT)
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
            && rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT))
    {
        ui->select.mode_sides_resize = extend_resize_none;
        ui->select.mode = selection_selected;
    }
    else if (rd_is_mouse_button_down(GLFW_MOUSE_BUTTON_RIGHT))
        ui->select.mode = selection_waiting;
    else if (rd_is_key_down(GLFW_KEY_LEFT_CONTROL))
    {
        if (rd_is_key_pressed(GLFW_KEY_Z)
            && (ui->select.mode & selection_show_blue)
        ) { // delete
            World_delete_area(
                &win->wrd, 
                ui->select.start,
                ui->select.end
            );
            ui->select.mode = selection_waiting;
        }
        else if (rd_is_key_pressed(GLFW_KEY_C) 
            && (ui->select.mode & selection_show_blue)
        ) { // copy
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
        else if (rd_is_key_pressed(GLFW_KEY_V)) 
        { // preview paste
            ui->select.mode = selection_paste_preview;
            set_to_paste_preview(ui);
            
            // start small only the first time
            ui->select.current_start = POS_TO_VEC2(ui->mouse_pos);
            ui->select.current_end   = POS_TO_VEC2(ui->mouse_pos);
        }
        
    }

    if (win->ui.select.mode & (selection_show_blue | selection_show_red))
        update_current_select(win);
}


void simulation_input(Window *win)
{ 
    if (rd_is_key_pressed(GLFW_KEY_LEFT_ALT))
    {
        printf("play one tick...");
        fflush(stdout);
        
        World_tick(&win->wrd);

        printf("\b\b\b"); // cut the ...
        UPDATE_LINE;
    }
}


void quit(Window *win)
{
    // const char *err_save = save_World(&win->wrd, win->ui.menu.current_file_save_path
    //                                                 ? win->ui.menu.current_file_save_path
    //                                                 : win->ui.menu.default_file_save_path
    // );
    // if (err_save)
    // {
    //     perror("save World");
    //     printf(err_save);
    // }

    Window_free(win);
    exit(0);
}


void main_menu(Window *win)
{

    if (win->ui.menu.submenu == Submenu_self)
    {
        if (rd_is_key_pressed(GLFW_KEY_ESCAPE))
            win->ui.in_main_menu = false;
        if (rd_Button(&win->ui.menu.quit))
            quit(win);
        if (rd_Button(&win->ui.menu.options))
            win->ui.menu.submenu = Submenu_option;
        if (rd_Button(&win->ui.menu.load_file))
            win->ui.menu.submenu = Submenu_load_file;
        if (rd_Button(&win->ui.menu.save_file))
            win->ui.menu.submenu = Submenu_save_file;
    }
    else if (win->ui.menu.submenu == Submenu_option)
    {
        TODO("options");
        
        if (rd_is_key_pressed(GLFW_KEY_ESCAPE))
            win->ui.menu.submenu = Submenu_self;
    }
    else if (win->ui.menu.submenu == Submenu_load_file)
    {
        if (rd_is_key_pressed(GLFW_KEY_ESCAPE))
            win->ui.menu.submenu = Submenu_self;
        
        
    }
    else if (win->ui.menu.submenu == Submenu_save_file)
    {
        if (rd_is_key_pressed(GLFW_KEY_ESCAPE))
            win->ui.menu.submenu = Submenu_self;
        
            
    }
}




// TODO: 
//    FIXS:
//     - flickering of the resize bar on the previous bug
//    FEATURES:
//     - event queue ?
//     - Keybinds
//     - Options

void inputs(Window *win)
{
    win->ui.old_mouse_pos = win->ui.mouse_pos;
    win->ui.mouse_pos = /* rdTODO screen_to_Pos */  VEC2_TO_POS(rd_get_cursor_pos(win->render));
    Ui *ui = &win->ui;

    if (win->ui.is_button_clicked
     && rd_is_mouse_button_released(GLFW_MOUSE_BUTTON_LEFT))
        win->ui.is_button_clicked = false;
    
    
    if (ui->in_main_menu)
        main_menu(win);
    else
    {
        if (rd_is_key_pressed(GLFW_KEY_ESCAPE))
            ui->in_main_menu = true;

        // tmp -> before ui
        if (rd_is_key_pressed(GLFW_KEY_P))
        {
            ui->select.mode = selection_waiting;
            ui->mode = mode_select;
            printf("mode selection");
            UPDATE_LINE;
        }
        else if (rd_is_key_pressed(GLFW_KEY_O))
        {
            ui->mode = mode_editing;
            printf("mode editing");
            UPDATE_LINE;
        }
        else if (rd_is_key_pressed(GLFW_KEY_I))
        {
            ui->mode = mode_idle;
            printf("mode idle");
            UPDATE_LINE;
        }
    
        
        
        if (rd_Button(&win->ui.mode_idle))
            win->ui.mode = mode_idle;
        if (rd_Button(&win->ui.mode_editing))
        {
            ui->select.mode = selection_waiting;
            win->ui.mode = mode_editing;
        }
        if (rd_Button(&win->ui.mode_select))
            win->ui.mode = mode_select;
        
    
        camera_mouvement_input(win);
        
        if (ui->mode == mode_editing)
            edition_input(win);
        else if (ui->mode == mode_select)
            select_input(win);
        else assert(ui->mode == mode_idle);
    
        simulation_input(win);
    
    }
    ui->is_button_clicked = false;
}

Ui Ui_make(Render *render)
{
    glfwSetScrollCallback(render->glfw, zoom_scroll_callback);

    const char *file_save_path = "./save/world1.wrd";
    Ui_draw_data draw = {0};

    {
        glGenVertexArrays(1, &draw.VAO);
        glGenBuffers(1, &draw.VBO);
        
        glBindVertexArray(draw.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, draw.VBO);
        glBufferData(GL_ARRAY_BUFFER, 
            draw.vertices.capacity * sizeof(draw.vertices.arr[0]),
            draw.vertices.arr, 
            GL_STREAM_DRAW
        );

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Ui_vertex_data), 
            (void*)0
        );
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Ui_vertex_data), 
            (void*)sizeof((Ui_vertex_data){0}.box)
        );
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Ui_vertex_data), 
            (void*)(sizeof((Ui_vertex_data){0}.box) 
                  + sizeof((Ui_vertex_data){0}.color))
        );
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(3, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(Ui_vertex_data), 
            (void*)(sizeof((Ui_vertex_data){0}.box)
                  + sizeof((Ui_vertex_data){0}.color)
                  + sizeof((Ui_vertex_data){0}.type))
        );
        glEnableVertexAttribArray(3);

        
    }

    return (Ui){
        .draw = draw,
        .mode = mode_idle,
        .mouse_pos = {0},
        .edit = {
            .current_state = cable_off,
            .current_direction = up,
            
            .style_normal = {
                .text = "#51#",
                .bounds = {0},
            },
            .style_orthogonal = {
                .text = "#68#",
                .bounds = {0},
            },
            .style_lign = {
                .text = "#69#",
                .bounds = {0},
            }
        },
        .select = {
            .mode_sides_resize = extend_resize_none,
            .clipboard = {0},
            .msg_clipboard = {0}
        },
        .menu = {
            .quit = {
                .text = "quit",
                .bounds = {0},
            },
            .options = {
                .text = "options",
                .bounds = {0},
            },
            .load_file = {
                .text = "load",
                .bounds = {0},
            },
            .save_file = {
                .text = "save",
                .bounds = {0},
            },
            .current_file_save_path = file_save_path,
            .default_file_save_path = file_save_path,
            .background_shade = (Color){ 0, 10, 30, 120 } // (Color){ 130, 130, 130, 255 / 4 }
        },
        .cam = { .zoom = 2. },
        .mode_idle = {
            .text = "#149#",
            .bounds = {0},
        },
        .mode_editing = {
            .text = "#22#", // 23 for big one
            .bounds = {0},
        },
        .mode_select = {
            .text = "#33#",
            .bounds = {0},
        },
        .mode_mouv_drag = {
            .text = "#19#",
            .bounds = {0},
        },
    };
}
void Ui_free(Ui *ui)
{
    Strb_free(ui->select.clipboard);
    Strb_free(ui->select.msg_clipboard);
}

