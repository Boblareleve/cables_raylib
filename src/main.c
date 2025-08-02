#include "main.h"


int _LATCH = 0;
const float direction_to_rotation[4] = { 0.0f, 90.0f, 180.0f, 270.0f };
// static_assert(direction_to_rotation[up] == 0.0f);
// static_assert(direction_to_rotation[left] == 90.0f);
// static_assert(direction_to_rotation[down] == 180.0f);
// static_assert(direction_to_rotation[right] == 270.0f);



Window Window_make(const char *name)
{
    // SetTraceLogLevel(LOG_DEBUG);
    SetTraceLogLevel(LOG_WARNING);
    // SetTargetFPS(INT32_MAX);
    SetTargetFPS(60);
    InitWindow(900, 900, name);

    Camera2D cam = (Camera2D){
        .offset = (Vector2){ GetScreenHeight() / 2, GetScreenWidth() / 2 }, // Camera offset (displacement from target)
        .target = (Vector2){0},                                             // Camera target (rotation and zoom origin)
        .rotation = 0.0f,
        .zoom = GetScreenWidth() / CHUNK_WORLD_SIZE,
    };
    Window res = (Window){
        .wrd = World_make(name),
        .cam = cam,
        .texs = Texs_make("./assets/new_atlas.png", "./assets/four_way_arrow.png"),
        .ui = Ui_make(cam)
    };
    return res;
}
void Window_free(Window *obj)
{
    Texs_free(&obj->texs);
    World_free(&obj->wrd);
    Ui_free(&obj->ui);
    CloseWindow();
}


/* TODO:

 * edition 
    -> selection copy paste rotate flip X
 * load/save file
    -> GUI
 * color by chunk for thread lockless
 * shader render
 * finish connection engin (support for bridge and update when possing T en N)

*/

int main(void)
{   
    Window win = Window_make("main");
    
    
    if (1) /* load world */ {
        World tmp_world = {0};
        char *err_world = load_World("./save/world1.wrd", &tmp_world);
        if (err_world) {
            perror("open");
            printf("%s\n", err_world);
        } else {
            World_free(&win.wrd);
            win.wrd = tmp_world;
        }
    }

    while (!WindowShouldClose()) 
    {
        StartBodyEnd(BeginDrawing(), EndDrawing())
        {
            inputs(&win);
            Window_draw(&win);
        }
    } 
    
    const char *err_save = save_World(&win.wrd, "./save/world1.wrd");
    if (err_save)
    {
        perror("save World");
        
        printf(err_save);
    }
    
    Window_free(&win);
    return (0);
}
