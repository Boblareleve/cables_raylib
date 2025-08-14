#include "main.h"


static Rectangle TransformRectangleByCamera2D(Camera2D cam, Rectangle rec)
{
    return (Rectangle){
        .x = rec.x * cam.zoom,
        .y = rec.y * cam.zoom,
        .width = rec.width * cam.zoom,
        .height = rec.height * cam.zoom
    };
}

bool pull_Button(const Ui *ui, Button *button)
{
    button->is_pressed = false;
    
    if (button->active)
    {
        if (CheckCollisionPointRec(
            GetMousePosition(), 
            TransformRectangleByCamera2D(button->used_cam, button->bound))
        ) {
            if (is_mouse_button_released(ui, MOUSE_BUTTON_LEFT))
                button->is_pressed = true;
        }
    }
    else button->is_pressed = false;
 
    
    return button->is_pressed;
}

void draw_Button(Button button)
{
    GuiButton(button.used_cam, button.bound, button.text);
}
