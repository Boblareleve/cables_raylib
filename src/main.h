#ifndef MAIN_H
#define MAIN_H


/* raylib */
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

/* std */
#include <signal.h>
#include <limits.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

/* my_lib */
#include "alf.h"
#include "sets.h"
#define STRING_FILEIO
#include "Str.h"
#include "print.h"
#include "sa.h"


#include "../engin/engin.h"


/* value macro */

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

#define CHUNK_WORLD_SIZE 256



/* function like macro */

#define SWAP(a, b) do { __auto_type SWAP_tmp = a; a = b; b = SWAP_tmp; } while (0)
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

extern int _LATCH;
#define BegEnd(Name, BeginArg, EndArg)\
for ((_LATCH = 0, Begin ## Name BeginArg); _LATCH < 1; _LATCH = 1, End ## Name EndArg)
#define StartBodyEnd(start, end)\
for ((_LATCH = 0, start); _LATCH < 1; _LATCH = 1, end)


/* data structure */

DA_TYPEDEF_ARRAY(Texture2D);
SA_TYPEDEF_ARRAY(float);
typedef struct Texs {
    //tex index ; tex chunk
    // sa_float *tex_chunks_x; // for the variant off o on
    // sa_float *tex_chunks_y; // for the type
    float cell_size;
    Texture2D  tex;
} Texs;



// incusive
#define GET_WIDTH(pos1, pos2) (abs(pos1.x - pos2.x) + 1)
// incusive
#define GET_HEIGHT(pos1, pos2) (abs(pos1.y - pos2.y) + 1)
#define GET_MIN_WIDTH_HEIGHT(pos1, pos2)\
    MIN(GET_WIDTH(pos1, pos2), GET_HEIGHT(pos1, pos2))

#define SELECTION_COLOR (Color){ 0, 121, 241, 32 }
#define SELECTION_COLOR_OUTLINE (Color){ 0, 121, 241, 172 }
#define PASTE_SELECTION_COLOR (Color){ 230, 41, 55, 32 }
#define PASTE_SELECTION_COLOR_OUTLINE (Color){ 230, 41, 55, 128 }
#define MOUV_PADDING (1./4.)

typedef struct Select_ui
{
    Pos start;
    Pos end;

    
    Vector2 current_start;
    Vector2 current_end;
    
    enum Extend_enable {
        extend_sides_none = 0,
        extend_sides_vertical = 0b01,
        extend_sides_horizontal = 0b10,
        extend_sides_full = 0b11,
    } mode_extend_sides;
    struct Rounded_rectangle {
        Rectangle rec;
        float roundness;
        int segments;
    } sides_vertices[4]; // up right down left
    
    enum Extend_resize {
        extend_resize_up = up,
        extend_resize_right = right,
        extend_resize_down= down,
        extend_resize_left = left,
        extend_resize_none,
    } sides_resize;
    
    /* enum {
        extend_corner_none = 0,
        extend_corner_full = 1,
    } mode_extend_corner;
    Vector2 corners_vertices[4][3]; // up-left up-right down-right down-left
     */

    enum {
        selection_off,
        selection_waiting_to_select,
        selection_selecting,
        selection_seleted,
        selection_paste_preview,
        selection_resizing,
    } mode;

    Strb clipboard;
    Strb msg_clipboard;
} Select_ui;

typedef struct Edit_ui {
    Cell current_state;
    Direction current_direction;
} Edit_ui;

typedef struct Ui
{
    enum Interaction_mode {
        mode_idle, // no editing and anything
        mode_editing,
        mode_select
    } mode;

    Pos mouse_pos;

    Edit_ui edit;
    Select_ui select;
} Ui;




typedef struct Window
{
    World wrd;
    Camera2D cam;

    Texs texs;

    Ui ui;
} Window;



/* globale variables */

extern const float direction_to_rotation[4];



/* function inline */

static inline Pos screen_to_Pos(Camera2D cam, Vector2 pos)
{
    pos = GetScreenToWorld2D(pos, cam);
    pos.x /= W;
    pos.y /= H;

    return (Pos){
        .x = (int)( pos.x + (pos.x < 0 ? -1 : 0)),
        .y = (int)(-pos.y + (pos.y > 0 ? -1 : 0))
    };
}
static inline Pos_Globale screen_to_Pos_Globale(Camera2D cam, Vector2 pos)
{
    return Pos_to_Pos_Globale(
        screen_to_Pos(cam, pos)
    );
}
static inline void sort_Pos_Globale(Pos *pos1, Pos *pos2)
{
    if (pos1->x > pos2->x)
        SWAP(pos1->x, pos2->x);
    if (pos1->y < pos2->y)
        SWAP(pos1->y, pos2->y);
}
static inline void Rectangle_print(Rectangle rec)
{
    printf("x%f y%f w%f h%f", rec.x, rec.y, rec.width, rec.height);
}

/* function prototypes */

void inputs(Window *win);

Texs Texs_make(const char *atlas_name);
void Texs_free(Texs *obj);

void Window_draw(const Window *obj);

Ui Ui_make(Camera2D cam);
void Ui_free(Ui *ui);










#endif /* MAIN_H */


