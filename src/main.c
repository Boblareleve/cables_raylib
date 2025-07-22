#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

#include <signal.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#define PRINT_IMPLEMENTATION
#define STRING_IMPLEMENTATION
#define STRING_FILEIO
#define SET_IMPLEMENTATION
#define ALF_IMPLEMENTATION



void *align_alloc(size_t size, size_t align)
{
    void *initial_alloc = malloc(sizeof(void*) + size + align);

    void *res = (void*)(((uintptr_t)initial_alloc & ~((uintptr_t)align - 1)) + align);
    assert(((uintptr_t)res & ((uintptr_t)align - 1)) == 0);
    assert((uintptr_t)((void**)res - 1) >= (uintptr_t)initial_alloc);

    ((void**)res)[-1] = initial_alloc;

    return res;
}
void align_free(void *ptr)
{
    assert(ptr - 0xffff <= (((void**)ptr)[-1]) && (((void**)ptr)[-1]) <= ptr + 0xffff);
    free(((void**)ptr)[-1]);
}

#define ALF_ALLOCATOR(size, align) align_alloc(size, align)
#define ALF_FREE(ptr) align_free(ptr)

#include "print.h"
#include "Str.h"
#include "sets.h"
#include "sa.h"
#include "alf.h"

#include "../engin/cell_auto.h"
#include "../engin/editor.h"

#ifdef CHUNK_GRID
# define CHUNK_GRID_VALUE 1
#else
# define CHUNK_GRID_VALUE 0
#endif 
#ifdef CHUNK_POS
# define CHUNK_POS_VALUE 1
#else
# define CHUNK_POS_VALUE 0
#endif 


#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define UPDATE_LINE printf("                                          \r"); fflush(stdout)

// int print_buffer_size = 0;
// char print_buffer[1024] = {0};
#define DrawTextF(format, x, y, font_size, color, ...)\
(\
    print_buffer_size = sprintf(print_buffer, format, __VA_ARGS__),\
    DrawText(print_buffer, x, y, font_size, color),\
    print_buffer_size = 0\
)
#define TEX_CHUNK_Y_ID(id) (id >> 4)
#define TEX_CHUNK_X_ID(id) ((id < u_transistor_off) ? 0 : ((id & VARIANT_MASK) >> 2))

#define SetWindowTitleF(format, ...)\
(\
    print_buffer_size = sprintf(format, __VA_ARGS__),\
    print_buffer[print_buffer_size + 1] = '\0',\
    SetWindowTitle(print_buffer),\
    print_buffer_size = 0\
)
#define Sprint_Vector2(v)\
(char*)({\
    print_buffer_size = snprintf(print_buffer, print_buffer_size, "(%f,%f)", v.x, v.y);\
    print_buffer[print_buffer_size + 1] = '\0';\
    print_buffer;\
})



static int _LATCH;
#define BegEnd(Name, BeginArg, EndArg)\
for ((_LATCH = 0, Begin ## Name BeginArg); _LATCH < 1; _LATCH = 1, End ## Name EndArg)


#define CHUNK_WORLD_SIZE 256


DA_TYPEDEF_ARRAY(Texture2D);
SA_TYPEDEF_ARRAY(float);
typedef struct Texs {
    //tex index ; tex chunk
    // sa_float *tex_chunks_x; // for the variant off o on
    // sa_float *tex_chunks_y; // for the type
    float cell_size;
    Texture2D  tex;
} Texs;



typedef struct Ui
{
    Cell current_state;
    Direction current_direction;
    Pos_Globale mouse_pos;

} Ui;

typedef struct Window
{
    World wrd;
    Camera2D cam;

    Texs texs;

    Ui ui;
} Window;

const float direction_to_rotation[4] = { 0.0f, 90.0f, 180.0f, 270.0f };
// static_assert(direction_to_rotation[up] == 0.0f);
// static_assert(direction_to_rotation[left] == 90.0f);
// static_assert(direction_to_rotation[down] == 180.0f);
// static_assert(direction_to_rotation[right] == 270.0f);



Texture2D load_tex(const char *name)
{
    Texture2D res = LoadTexture(name);
    assert(IsTextureValid(res));
    return res;
}


Texs Texs_make(const char *atlas_name)
{
    Texs res = {
        .tex = load_tex(atlas_name),
        /* .tex_chunks_x = sa_make(res.tex_chunks_x, 3),
        .tex_chunks_y = sa_make(res.tex_chunks_y, 5), */
        .cell_size = W
    };
    /* float offset = 0.;
    for (int i = 0; i < res.tex_chunks_y->size; i++)
    {
        res.tex_chunks_y->arr[i] = offset;
        offset += res.cell_size;
    }
    offset = 0.;
    for (int i = 0; i < res.tex_chunks_x->size; i++)
    {
        res.tex_chunks_x->arr[i] = offset;
        offset += res.cell_size;
    } */
    return res;
}
void Texs_free(Texs *obj)
{
    UnloadTexture(obj->tex);
    /* sa_free(obj->tex_chunks_x);
    sa_free(obj->tex_chunks_y); */
}


Pos_Globale vec2_world_to_pos_globale(Vector2 pos)
{
    pos.x /= W;
    pos.y /= H;

    return (Pos_Globale){
        .chunk = (Pos){
            .x = ((int)( pos.x + (pos.x < 0 ? -1 : 0)) >> 4),
            .y = ((int)(-pos.y + (pos.y > 0 ? -1 : 0)) >> 4)
        },
        .cell = (((int)( pos.x + (pos.x < 0 ? -1 : 0)) & 0xf) << 4)
              |  ((int)(-pos.y + (pos.y > 0 ? -1 : 0)) & 0xf)
    };
}
Pos_Globale screen_to_world_pos(Camera2D cam, Vector2 pos)
{
    Vector2 vpos = GetScreenToWorld2D(pos, cam);
    return vec2_world_to_pos_globale(vpos);
}

Window Window_make(const char *name)
{
    SetTraceLogLevel(LOG_DEBUG);
    // SetTraceLogLevel(LOG_WARNING);
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
        .texs = Texs_make("./assets/new_atlas.png"),
        .ui = (Ui){ 
            .current_state = cable_off,
            .current_direction = up,
            .mouse_pos = screen_to_world_pos(cam, GetMousePosition())
        }
    };
    return res;
}
void Window_free(Window *obj)
{
    Texs_free(&obj->texs);
    World_free(&obj->wrd);
    CloseWindow();
}





static inline void draw_Texs_Cell_V(const Texs *texs, Cell id, Rectangle pos)
{
    if (id == empty)
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

void Window_draw(const Window *obj)
{
    int draw_count = 0;
    
    BegEnd(Drawing,(),()) 
    {
        ClearBackground(DARKGRAY);
        DrawTextF("[FPS %d]", 10, 10, 24, WHITE, GetFPS());
        
        BegEnd(Mode2D,(obj->cam),())
        {
            if (1) {
                const Pos_Globale lu_ws = vec2_world_to_pos_globale(GetScreenToWorld2D(
                    (Vector2){ 0, 0 },
                    obj->cam
                ));
                const Pos_Globale rd_ws = vec2_world_to_pos_globale(GetScreenToWorld2D(
                    (Vector2){ GetScreenWidth(), GetScreenHeight() },
                    obj->cam
                ));

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
            else {
                for_set (Item_chunks, chunk, &obj->wrd.chunks)
                {
                    Chunk_draw(obj, chunk->data);
                    draw_count++;
                }
            }
            
            { // draw overed cell
                const Pos_Globale gpos = obj->ui.mouse_pos;
                DrawRectangleLines(
                     gpos.chunk.x * CHUNK_WORLD_SIZE + W * (gpos.cell >> 4),
                    -gpos.chunk.y * CHUNK_WORLD_SIZE - H * (gpos.cell & 0xf) - H,
                    obj->texs.cell_size,
                    obj->texs.cell_size,
                    WHITE
                );
            }
        }
        draw_Texs_Cell_V(
            &obj->texs,
            obj->ui.current_state,
            (Rectangle){
                .x = 0, .y = 0,
                .width =  8 * obj->texs.cell_size,
                .height = 8 * obj->texs.cell_size,
            }
        );
    }
}



void inputs(Window *win)
{
    win->ui.mouse_pos = screen_to_world_pos(win->cam, GetMousePosition());
    if (0) printf("(%i,%i).%02x\n", 
        win->ui.mouse_pos.chunk.x, 
        win->ui.mouse_pos.chunk.y, 
        win->ui.mouse_pos.cell
    );

    { // camera mouvements
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

        // Camera reset (zoom and rotation)
        if (0) // IsKeyPressed(KEY_R))
        {
            printf("reset"); 
            UPDATE_LINE;
            win->cam.zoom = 1.0f;
            win->cam.target = (Vector2){0};
        }
    }
    
    { // edition
        if (IsKeyDown(KEY_ONE))
        {
            win->ui.current_state = cable_off;
            printf("select cable");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_TWO))
        {
            win->ui.current_state = ty_transistor 
                                  | var_off
                                  | win->ui.current_direction;
            printf("select transistor on");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_THREE))
        {
            win->ui.current_state = ty_not_gate
                                  | var_off
                                  | win->ui.current_direction;
            printf("select not gate");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_FOUR))
        {
            win->ui.current_state = bridge;
            printf("select bridge");
            UPDATE_LINE;
        }
        if (IsKeyDown(KEY_ZERO))
        {
            win->ui.current_state = cable_on;
            printf("select cable on");
            UPDATE_LINE;
        }
        if (IsKeyPressed(KEY_R))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT))
                win->ui.current_direction = (win->ui.current_direction - 1) % 4;
            else 
                win->ui.current_direction = (win->ui.current_direction + 1) % 4;
            
            if ((win->ui.current_state & TYPE_MASK) == ty_transistor
             || (win->ui.current_state & TYPE_MASK) == ty_not_gate)
                win->ui.current_state = (win->ui.current_state 
                                       & (TYPE_MASK | VARIANT_MASK)
                                    )  | win->ui.current_direction;
        }
    
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            World_set_cell(
                &win->wrd,
                win->ui.mouse_pos.chunk,
                win->ui.mouse_pos.cell,
                win->ui.current_state
            );
            printf("click!");
            UPDATE_LINE;
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT))
        {
            World_set_cell(
                &win->wrd,
                win->ui.mouse_pos.chunk,
                win->ui.mouse_pos.cell,
                empty
            );
            printf("click!");
            UPDATE_LINE;
        }
    }

    { // simulation
        if (IsKeyPressed(KEY_LEFT_ALT))
        {
            printf("play one tick...");
            fflush(stdout);
            
            World_tick(&win->wrd);

            printf("\b\b\b");
            UPDATE_LINE;
        }

    }
}


typedef struct ToPlace 
{
    Pos chunk;
    Pos_Chunk cell;
    Cell value;
} ToPlace;

int main(void)
{
    // InitWindow(900, 900, "name");
    // CloseWindow();


    // return 0;
    Window win = Window_make("main");
    
    // Window_free(&win);
    // return 0;

    if (0)
    {
        ToPlace poss[] = {
            
/*             { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x00, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x10, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x01, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x0f, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x1f, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x0e, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0xf0, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0xe0, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0xf1, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0xff, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0xef, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0xfe, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1 },  .cell = 0x00, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1 },  .cell = 0x0f, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1 },  .cell = 0xf0, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1 },  .cell = 0xff, .value = cable_off},
            { .chunk = (Pos){ .x = 1, .y = 0 },  .cell = 0x00, .value = cable_off},
            { .chunk = (Pos){ .x = 1, .y = 0 },  .cell = 0x0f, .value = cable_off},
            { .chunk = (Pos){ .x = 1, .y = 0 },  .cell = 0xf0, .value = cable_off},
            { .chunk = (Pos){ .x = 1, .y = 0 },  .cell = 0xff, .value = cable_off},
            { .chunk = (Pos){ .x = -1, .y = 0 },  .cell = 0x00, .value = cable_off},
            { .chunk = (Pos){ .x = -1, .y = 0 },  .cell = 0x0f, .value = cable_off},
            { .chunk = (Pos){ .x = -1, .y = 0 },  .cell = 0xf0, .value = cable_off},
            { .chunk = (Pos){ .x = -1, .y = 0 },  .cell = 0xff, .value = cable_off},
            */
/*             { .chunk = (Pos){ .x = -1,.y = -1},  .cell = 0xfe, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1},  .cell = 0x1e, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1},  .cell = 0x0e, .value = cable_off},
            { .chunk = (Pos){ .x = -1,.y = -1},  .cell = 0xff, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1},  .cell = 0x1f, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = -1},  .cell = 0x0f, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x01, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x02, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 },  .cell = 0x03, .value = cable_off},
 */
            // { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x00, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x02, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x0a, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x2a, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x1a, .value = bridge},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x01, .value = bridge},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x02, .value = bridge},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x12, .value = bridge},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x11, .value = bridge},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x12, .value = bridge},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x03, .value = cable_off},
            { .chunk = (Pos){ .x = 0, .y = 0 }, .cell = 0x13, .value = cable_off},
        };
        

        size_t i = 0;
        for (i = 0; i < sizeof(poss)/sizeof(poss[0]); i++)
            World_set_cell(&win.wrd, poss[i].chunk, poss[i].cell, poss[i].value);
        
        if (0) {
            Chunk *chunk = set_Item_chunks_get(
                &win.wrd.chunks, 
                (Item_chunks){0}
            )->data;
            chunk->arr[0x01] &= 0b1111;
            chunk->arr[0x01] |= cable_on;

            chunk->arr[0x1a] &= 0b1111;
            chunk->arr[0x1a] |= cable_on;
        }
        

        for (i = 0; i < sizeof(poss)/sizeof(poss[0]); i++)
        {
            Chunk *chunk = set_Item_chunks_get(
                &win.wrd.chunks, 
                (Item_chunks){ .pos = poss[i].chunk }
            )->data;
            printf("connection -> (%+i, %+i).%02x -> %x\n", 
                poss[i].chunk.x, poss[i].chunk.y,
                poss[i].cell,
                chunk->arr[poss[i].cell]
            );
        }
        
        for (int y = H-1; y >= 0; y--)
        {
            for (int x = 0; x < W; x++)
                printf("%02x ", set_Item_chunks_get(&win.wrd.chunks, (Item_chunks){ .pos= (Pos){0} })->data->arr2D[x][y]);
            printf("\n");
        }
    }
    
    while (!WindowShouldClose())
    {
        Window_draw(&win);
        inputs(&win);
    } 
    
    Window_free(&win);
    return (0);
}
