#include "main.h"
#include "raygui.h"

static Rectangle TransformRectangleByCamera2D(Camera2D cam, Rectangle rec)
{
    return (Rectangle){
        .x = rec.x * cam.zoom,
        .y = rec.y * cam.zoom,
        .width = rec.width * cam.zoom,
        .height = rec.height * cam.zoom
    };
}

bool pull_Button(Ui *ui, Button *button)
{
    if (button->state != state_disabled)
    {
        if (CheckCollisionPointRec(
            GetMousePosition(), 
            TransformRectangleByCamera2D(ui->in_game_ui_cam, button->bounds))
        ) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                button->state = state_pressed;
            else button->state = state_focused;
        }
        else button->state = state_normal;
    }

    if (button->state == state_pressed)
    {
        ui->is_button_clicked = true;
        return true;
    }
    return false;
}

void draw_Button(Button button)
{
    GuiDrawButton(button.bounds, button.text, button.state);
}
