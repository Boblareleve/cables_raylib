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

#ifdef FPS
    const int fps = FPS < 1 ? INT32_MAX : FPS;
    printf("FPS %d\n", fps);
    SetTargetFPS(fps);
#else
    printf("FPS: 60\n");
    SetTargetFPS(60);
#endif

    const float ratio = 1.;
    InitWindow(1600 / ratio, 900 / ratio, name);

    Camera2D cam = (Camera2D){
        .offset = (Vector2){ GetScreenHeight() / 2, GetScreenWidth() / 2 }, // Camera offset (displacement from target)
        .target = (Vector2){0},                                             // Camera target (rotation and zoom origin)
        .rotation = 0.0f,
        .zoom = GetScreenWidth() / CHUNK_WORLD_SIZE,
    };
    Window res = (Window){
        .wrd = (World){0},
        .cam = cam,
        .texs = Texs_make("./assets/new_atlas.png", "./assets/four_way_arrow.png"),
        .ui = Ui_make(cam),
        .background_color = DARKGRAY
    };
    return res;
}
void Window_free(Window *win)
{
    Texs_free(&win->texs);
    World_free(&win->wrd);
    Ui_free(&win->ui);
    CloseWindow();
}


/* TODO:

 * edition 
    -> selection copy paste rotate flip
 * load/save file
    -> GUI
 * color by chunk for thread lockless
 * shader render
 * finish connection engin (support for bridge and update when possing T en N)
 * debug engin
 * 
*/

int main(void)
{   
    Window win = Window_make("main");
    
    
    if (1) /* load world */ {
        char *err_world = load_World("./save/world1.wrd", &win.wrd);
        if (err_world) {
            perror("open");
            printf("%s\n", err_world);
            win.wrd = World_make("main");
        }
    }


    while (!WindowShouldClose())
    {
        StartBodyEnd(BeginDrawing(), EndDrawing())
        {
            inputs(&win);
            Window_draw(&win, &win.ui);
        }
    }
    
    quit(&win);
    
    // Window_free(&win);
    return (0);
}
