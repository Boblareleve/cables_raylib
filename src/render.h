#ifndef RENDER_H
#define RENDER_H

#include "gl.h"
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include "stb_image.h"
#include "da.h"
#include "../utils.h"
#include "print.h"
#define STRING_FILEIO
#include "Str.h"


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

typedef struct Color
{
    float r, g, b, a;
} Color;

// Some Basic Colors stolen from raylib (thanks)
// NOTE: Custom raylib color palette for amazing visuals on WHITE background
#define LIGHTGRAY  (Color){ 1./255. * 200, 1./255. * 200, 1./255. * 200, 1./255. * 255 }   // Light Gray
#define GRAY       (Color){ 1./255. * 130, 1./255. * 130, 1./255. * 130, 1./255. * 255 }   // Gray
#define DARKGRAY   (Color){ 1./255. * 80,  1./255. * 80,  1./255. * 80,  1./255. * 255 }   // Dark Gray
#define YELLOW     (Color){ 1./255. * 253, 1./255. * 249, 1./255. * 0,   1./255. * 255 }   // Yellow
#define GOLD       (Color){ 1./255. * 255, 1./255. * 203, 1./255. * 0,   1./255. * 255 }   // Gold
#define ORANGE     (Color){ 1./255. * 255, 1./255. * 161, 1./255. * 0,   1./255. * 255 }   // Orange
#define PINK       (Color){ 1./255. * 255, 1./255. * 109, 1./255. * 194, 1./255. * 255 }   // Pink
#define RED        (Color){ 1./255. * 230, 1./255. * 41,  1./255. * 55,  1./255. * 255 }   // Red
#define MAROON     (Color){ 1./255. * 190, 1./255. * 33,  1./255. * 55,  1./255. * 255 }   // Maroon
#define GREEN      (Color){ 1./255. * 0,   1./255. * 228, 1./255. * 48,  1./255. * 255 }   // Green
#define LIME       (Color){ 1./255. * 0,   1./255. * 158, 1./255. * 47,  1./255. * 255 }   // Lime
#define DARKGREEN  (Color){ 1./255. * 0,   1./255. * 117, 1./255. * 44,  1./255. * 255 }   // Dark Green
#define SKYBLUE    (Color){ 1./255. * 102, 1./255. * 191, 1./255. * 255, 1./255. * 255 }   // Sky Blue
#define BLUE       (Color){ 1./255. * 0,   1./255. * 121, 1./255. * 241, 1./255. * 255 }   // Blue
#define DARKBLUE   (Color){ 1./255. * 0,   1./255. * 82,  1./255. * 172, 1./255. * 255 }   // Dark Blue
#define PURPLE     (Color){ 1./255. * 200, 1./255. * 122, 1./255. * 255, 1./255. * 255 }   // Purple
#define VIOLET     (Color){ 1./255. * 135, 1./255. * 60,  1./255. * 190, 1./255. * 255 }   // Violet
#define DARKPURPLE (Color){ 1./255. * 112, 1./255. * 31,  1./255. * 126, 1./255. * 255 }   // Dark Purple
#define BEIGE      (Color){ 1./255. * 211, 1./255. * 176, 1./255. * 131, 1./255. * 255 }   // Beige
#define BROWN      (Color){ 1./255. * 127, 1./255. * 106, 1./255. * 79,  1./255. * 255 }   // Brown
#define DARKBROWN  (Color){ 1./255. * 76,  1./255. * 63,  1./255. * 47,  1./255. * 255 }   // Dark Brown

#define WHITE      (Color){ 1./255. * 255, 1./255. * 255, 1./255. * 255, 1./255. * 255 }   // White
#define BLACK      (Color){ 1./255. * 0,   1./255. * 0,   1./255. * 0,   1./255. * 255 }   // Black
#define BLANK      (Color){ 1./255. * 0,   1./255. * 0,   1./255. * 0,   1./255. * 0   }   // Blank (Transparent)
#define MAGENTA    (Color){ 1./255. * 255, 1./255. * 0,   1./255. * 255, 1./255. * 255 }   // Magenta
#define RAYWHITE   (Color){ 1./255. * 245, 1./255. * 245, 1./255. * 245, 1./255. * 255 }   // My own White (raylib logo)


typedef struct Button
{
    Rect bounds;
    enum {
        rd_normal = 0,
        rd_focused,
        rd_pressed,
        rd_disabled
    } state;

    const char *text;
    // bool is_in_world_space;
} Button;

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
} Shader;

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

// Shader_id loadShader();
// char *, shader_names
// Shader_programme_id get_Shader_programme(const char *shader_name, ...);


Render *rd_init(const char *title, int width, int height, rd_resize_callback_t callback);
void rd_deinit();

void rd_set_clear_color(Color clear_color);


bool rd_should_close();
void rd_set_to_close();
// function to be call each frame
// swapbuffers -> calculate frame time -> poll inputs
void rd_end_frame();

Texture rd_load_texture(const char *tex_name, int tex_unit_index);

// inputs

// keyboard
Key_state rd_get_key_state(int key);
bool rd_is_key_down(int key);
bool rd_is_key_up(int key);
bool rd_is_key_pressed(int key);
bool rd_is_key_released(int key);


// mouse
Vec2 rd_get_cursor_pos();

Key_state rd_get_mouse_button_state(int button);
bool rd_is_mouse_button_down(int button);
bool rd_is_mouse_button_up(int button);
bool rd_is_mouse_button_pressed(int button);
bool rd_is_mouse_button_released(int button);

bool rd_Button(Button *button);
void draw_Button(Button button);

int rd_get_fps();
float rd_get_delta_time();
float rd_get_target_fps();
float rd_get_screen_width();
float rd_get_screen_height();
float rd_get_screen_ratio();
Vec2 rd_get_screen_to_world(Camera cam, Vec2 pos);

Rect Rect_transform_by_Camera(Camera cam, Rect rec);
bool rd_collision_Vec2_Rect(Vec2 vec, Rect rec);

#endif /* RENDER_H */