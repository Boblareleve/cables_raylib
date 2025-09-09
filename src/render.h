#ifndef RENDER_H
#define RENDER_H

#include "gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "stb_image.h"
#include "da.h"
#include "utils.h"
#include "print.h"
#define STRING_FILEIO
#include "Str.h"

#include "utils.h"
#include "geometry.h"


typedef GLint Uniform_loc;
typedef uint32_t Texture_id;
typedef uint32_t Shader_programme_id;
typedef uint32_t Shader_id;
typedef uint32_t VAO_id;
typedef uint32_t VBO_id;
typedef uint32_t EBO_id;
typedef uint32_t TBO_id;

DA_TYPEDEF_ARRAY(Shader_id);


typedef struct Render Render;
typedef void (*rd_resize_callback_t)(Render*, int width, int height);

// typedef struct Color
// {
//     union {
//         struct { // little eldian
//             uint8_t a;
//             uint8_t b;
//             uint8_t g;
//             uint8_t r;
//         };
//         uint32_t packed;
//         uint8_t array[4];
//     };
// } Color;
// static_assert(sizeof(Color) == 4);
typedef struct Color
{
    float r, g, b, a;
} Color;
static inline void Color_print(Color c)
{
    printf("r%f g%f b%f a%f", c.r, c.g, c.b, c.a);
}
static inline char *Color_sprint(Color c)
{
    static char buffer[64] = {0};
    snprintf(buffer, 64, "r%0.2f g%0.2f b%0.2f a%0.2f", c.r, c.g, c.b, c.a);
    return buffer;
}


#define UNORMF(x) ((x) / 255.)
#define HEX_TO_COLOR(hex, alpha)\
    ((Color){\
        UNORMF((hex >> 8*2) & 0xFF),\
        UNORMF((hex >> 8*1) & 0xFF),\
        UNORMF((hex >> 8*0) & 0xFF),\
        alpha\
    })

// Some Basic Colors stolen from raylib (thanks)
// NOTE: Custom raylib color palette for amazing visuals on WHITE background
#define LIGHTGRAY  (Color){ .r = UNORMF(200), .g = UNORMF(200), .b = UNORMF(200), .a = UNORMF(255) }   // Light Gray
#define GRAY       (Color){ .r = UNORMF(130), .g = UNORMF(130), .b = UNORMF(130), .a = UNORMF(255) }   // Gray
#define DARKGRAY   (Color){ .r = UNORMF(80) , .g = UNORMF(80) , .b = UNORMF(80) , .a = UNORMF(255) }   // Dark Gray
#define YELLOW     (Color){ .r = UNORMF(253), .g = UNORMF(249), .b = UNORMF(0)  , .a = UNORMF(255) }   // Yellow
#define GOLD       (Color){ .r = UNORMF(255), .g = UNORMF(203), .b = UNORMF(0)  , .a = UNORMF(255) }   // Gold
#define ORANGE     (Color){ .r = UNORMF(255), .g = UNORMF(161), .b = UNORMF(0)  , .a = UNORMF(255) }   // Orange
#define PINK       (Color){ .r = UNORMF(255), .g = UNORMF(109), .b = UNORMF(194), .a = UNORMF(255) }   // Pink
#define RED        (Color){ .r = UNORMF(230), .g = UNORMF(41) , .b = UNORMF(55) , .a = UNORMF(255) }   // Red
#define MAROON     (Color){ .r = UNORMF(190), .g = UNORMF(33) , .b = UNORMF(55) , .a = UNORMF(255) }   // Maroon
#define GREEN      (Color){ .r = UNORMF(0)  , .g = UNORMF(228), .b = UNORMF(48) , .a = UNORMF(255) }   // Green
#define LIME       (Color){ .r = UNORMF(0)  , .g = UNORMF(158), .b = UNORMF(47) , .a = UNORMF(255) }   // Lime
#define DARKGREEN  (Color){ .r = UNORMF(0)  , .g = UNORMF(117), .b = UNORMF(44) , .a = UNORMF(255) }   // Dark Green
#define SKYBLUE    (Color){ .r = UNORMF(102), .g = UNORMF(191), .b = UNORMF(255), .a = UNORMF(255) }   // Sky Blue
#define BLUE       (Color){ .r = UNORMF(0)  , .g = UNORMF(121), .b = UNORMF(241), .a = UNORMF(255) }   // Blue
#define DARKBLUE   (Color){ .r = UNORMF(0)  , .g = UNORMF(82) , .b = UNORMF(172), .a = UNORMF(255) }   // Dark Blue
#define PURPLE     (Color){ .r = UNORMF(200), .g = UNORMF(122), .b = UNORMF(255), .a = UNORMF(255) }   // Purple
#define VIOLET     (Color){ .r = UNORMF(135), .g = UNORMF(60) , .b = UNORMF(190), .a = UNORMF(255) }   // Violet
#define DARKPURPLE (Color){ .r = UNORMF(112), .g = UNORMF(31) , .b = UNORMF(126), .a = UNORMF(255) }   // Dark Purple
#define BEIGE      (Color){ .r = UNORMF(211), .g = UNORMF(176), .b = UNORMF(131), .a = UNORMF(255) }   // Beige
#define BROWN      (Color){ .r = UNORMF(127), .g = UNORMF(106), .b = UNORMF(79) , .a = UNORMF(255) }   // Brown
#define DARKBROWN  (Color){ .r = UNORMF(76) , .g = UNORMF(63) , .b = UNORMF(47) , .a = UNORMF(255) }   // Dark Brown

#define WHITE      (Color){ .r = UNORMF(255), .g = UNORMF(255), .b = UNORMF(255), .a = UNORMF(255) }   // White
#define BLACK      (Color){ .r = UNORMF(0)  , .g = UNORMF(0)  , .b = UNORMF(0)  , .a = UNORMF(255) }   // Black
#define BLANK      (Color){ .r = UNORMF(0)  , .g = UNORMF(0)  , .b = UNORMF(0)  , .a = UNORMF(0)   }   // Blank (Transparent)
#define MAGENTA    (Color){ .r = UNORMF(255), .g = UNORMF(0)  , .b = UNORMF(255), .a = UNORMF(255) }   // Magenta
#define RAYWHITE   (Color){ .r = UNORMF(245), .g = UNORMF(245), .b = UNORMF(245), .a = UNORMF(255) }   // My own White (raylib logo)



typedef struct Texture
{
    Texture_id  id;
    Uniform_loc loc;
    int texture_unit;
} Texture;



#define MOUSE_BOUTTON_COUNT (GLFW_MOUSE_BUTTON_LAST + 1)
#define KEY_COUNT (GLFW_KEY_LAST + 1)
typedef enum Key_state
{
    key_not_a_key = 0b10000,
    key_normal   = 0b0000,
    key_down     = 0b0001,
    key_pressed  = 0b0010 | key_down,
    key_repeat   = 0b0100 | key_down, // key_pressed & key_repeat == false bc sometime on repeat you won't have a precise count of perssed so no match count of released
    key_released = 0b1000,
    Key_state_count
} Key_state;

typedef struct Render
{
    GLFWwindow *glfw;
    bool    to_close;

    int screen_width;
    int screen_height;
    float screen_ratio; // width / height

    bool is_mouse_input_button_captured;
    Camera ui_camera;

    struct Inputs
    {
        Key_state keys[KEY_COUNT];
        Key_state mouse_button[MOUSE_BOUTTON_COUNT];
    } inputs;

    rd_resize_callback_t resize_callback;
    
    struct Frame_time {
        double start_frame_time;
        double end_frame_time;
        double delta_time;
        double target_fps;
        int fps;
    } frame_time;
} Render;

/* Shader */

typedef struct Shader_metadata
{
    GLenum type;
    char const* file_path;
} Shader_metadata;
DA_TYPEDEF_ARRAY(Shader_metadata);
typedef bool (*Uniform_setter_fun_t)(Shader_programme_id, void*);

typedef struct Shader
{
    Shader_programme_id program;
    da_Shader_metadata files;
    Uniform_setter_fun_t static_uniform_setter;

    int tiu_vertex_dispatcher;
    int tiu_geometry_dispatcher;
    int tiu_fragment_dispatcher;
} Shader;


// 256 * 56 = 14.336Kb
#define BASE_MAX_UI_ELEMENT_COUNT 256 // maybe not enough for option menu ?

typedef struct rd_ui_rect
{
    Rect  box;
    Color fill_color;
    Color border_color;
    Color on_hover_filter;
    // uint8_t _padding[4];
    float border_thickness;
    float texture_index; // 0 for none start at 1
} rd_ui_rect;
// static_assert(sizeof(rd_ui_Omni_rect_vertex) == 36 + 4 /* padding */); // /2 = 18
static_assert(sizeof(rd_ui_rect) == 72);

DA_TYPEDEF_ARRAY(rd_ui_rect);
// DA_TYPEDEF_ARRAY(da_rd_ui_rect);

typedef enum Button_state
{
    rd_normal = 0,
    rd_focused,
    rd_pressed, // on rd_mouse_button_pressed
    rd_down,    // on rd_mouse_button_down
    rd_up,      // on rd_mouse_button_up
    rd_disabled = 0b1000000,
    rd_hiden    = 0b1000001,
} Button_state;
typedef struct Button
{
    Rect bounds;
    Button_state state;
    uint32_t rect_indexe;
    // const char *text;
    // uint32_t texture_index; // 
    // bool is_in_world_space;
} Button;

static_assert( (rd_hiden   & rd_disabled));
static_assert(!(rd_normal  & rd_disabled));
static_assert(!(rd_focused & rd_disabled));
static_assert(!(rd_pressed & rd_disabled));
static_assert(!(rd_down    & rd_disabled));
static_assert(!(rd_up      & rd_disabled));

// typedef struct rd_ui_rect_group
// {
//     da_rd_ui_rect vertices;
//     bool enable;
//     int offset; // in VBO
// } rd_ui_rect_group;
// DA_TYPEDEF_ARRAY(rd_ui_rect_group);

DA_TYPEDEF_ARRAY(uint32_t);
typedef struct rd_ui_Handel
{
    // da_rd_ui_rect_group rects_groups;
    da_rd_ui_rect vertices;
    da_uint32_t indices;
    
    float ui_scale;
    float ui_virtual_width;
    float ui_virtual_height;

    Texture atlas;

    VAO_id VAO;
    EBO_id EBO;
    
    VBO_id VBO;
    int VBO_capacity; // in number of vertex

    Shader shader;
    struct rd_ui_Shaders_locs
    {
        // GLint campos;
        // GLint camzoom;
        // GLint ratio;
        GLint window_width;
        GLint window_height;
    } loc;
    
} rd_ui_Handel;




typedef enum Rd_log_level
{
    rd_info,
    rd_warning,
    rd_error,
    rd_count
} Rd_log_level;
#define rd_log(level, msg, ...) _rd_log(level, __LINE__, __FILE__, msg, ##__VA_ARGS__)
void _rd_log(Rd_log_level level, int line, const char *file, const char *msg, ...);


bool rd_reload_shader(Shader *shader, void *uniform_setter_arg);
#define rd_load_shader(shader_ptr, static_uniform_setter, uniform_setter_arg, ...) _rd_load_shader(shader_ptr, static_uniform_setter, uniform_setter_arg, __VA_ARGS__, NULL)
bool _rd_load_shader(Shader *res, Uniform_setter_fun_t uniform_setter, void *uniform_setter_arg, ...);
void rd_unload_shader(Shader *shader);
int rd_dispatch_texture_unit(Shader *shader, GLenum stage);

// Shader_id loadShader();
// char *, shader_names
// Shader_programme_id get_Shader_programme(const char *shader_name, ...);


Render *rd_init(const char *title, Color background_color, int width, int height, rd_resize_callback_t callback);
void rd_deinit(void);

void rd_set_clear_color(Color clear_color);


bool rd_should_close(void);
void rd_set_to_close(void);
// function to be call each frame
// swapbuffers -> calculate frame time -> poll inputs
void rd_end_frame(void);

Texture rd_load_texture(const char *tex_name, int tex_unit_index);
void rd_unload_texture(Texture *tex);


// inputs

// keyboard
Key_state rd_get_key_state(int key);
bool rd_is_key_down(int key);
bool rd_is_key_up(int key);
bool rd_is_key_pressed(int key);
bool rd_is_key_released(int key);


// mouse
Vec2 rd_get_cursor_pos(void);

Key_state rd_get_mouse_button_state(int button);
bool rd_is_mouse_button_down(int button);
bool rd_is_mouse_button_up(int button);
bool rd_is_mouse_button_pressed(int button);
bool rd_is_mouse_button_released(int button);

bool rd_Button(Button *button);
void draw_Button(Button button);

int rd_get_fps(void);
float rd_get_delta_time(void);
float rd_get_target_fps(void);
float rd_get_screen_width(void);
float rd_get_screen_height(void);
float rd_get_screen_ratio(void);
Vec2 rd_get_screen_to_world(Camera cam, Vec2 pos);

Rect Rect_transform_by_Camera(Camera cam, Rect rec);
bool rd_collision_Vec2_Rect(Vec2 vec, Rect rec);

//ui
rd_ui_Handel rd_ui_make_Handel(float scale);
void rd_ui_free_Handel(rd_ui_Handel *handel);
bool rd_ui_set_uniforms(Shader_programme_id program, rd_ui_Handel *rd_ui_handel);
void rd_ui_draw(rd_ui_Handel *handel);


#endif /* RENDER_H */
