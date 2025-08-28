#define GLAD_GL_IMPLEMENTATION
#include "render.h"

static Render *current_render = NULL;

void _rd_log(Rd_log_level level, int line, const char *file, const char *msg, ...)
{
    static const char *level_msg[rd_count] = {
        [rd_info]    = "[INFO]",
        [rd_warning] = "[WARNING]",
        [rd_error]   = "[ERROR]"
    };
    printf("%s %s:%d: ", level_msg[level], file, line);
    va_list args;
    va_start(args, msg);
        vprintf(msg, args);
    va_end(args);
    printf("\n");
}


#ifdef DEBUG
static void debugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    UNUSED(userParam);
    // Some debug messages are just annoying informational messages
    if (id == 131185) // glBufferData success weird NVIDIA shit aparenly 
        return ;
    
    printf("[");
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH_ARB:    printf("ERROR");    break;
    case GL_DEBUG_SEVERITY_MEDIUM_ARB:  printf("WARNING");  break;
    case GL_DEBUG_SEVERITY_LOW_ARB:     printf("NOTICE");   break;
    // case GL_DEBUG_SEVERITY_NOTIFICATION_ARB: printf("INFO"); break;
    default: UNREACHABLE("unkown error severity");
    }
    printf("] from ");
    switch (source)
    {
    case GL_DEBUG_SOURCE_API_ARB:               printf("API");              break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:     printf("Window System");    break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:   printf("Shader Compiler");  break;
    case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:       printf("Third Party");      break;
    case GL_DEBUG_SOURCE_APPLICATION_ARB:       printf("Application");      break;
    case GL_DEBUG_SOURCE_OTHER_ARB:             printf("Other");            break;
    default: UNREACHABLE("unkown error source");
    }
    printf(" (ID: %d)", id);
    printf(": %.*s\n", length, message);
    printf("\t");
    printf("Type: ");
    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR_ARB:               printf("Error"); break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: printf("Deprecated Behavior"); break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  printf("Undefined Behavior"); break;
    case GL_DEBUG_TYPE_PORTABILITY_ARB:         printf("Portability"); break;
    case GL_DEBUG_TYPE_PERFORMANCE_ARB:         printf("Performance"); break;
    // case GL_DEBUG_TYPE_MARKER_ARB:              printf("Marker"); break;
    // case GL_DEBUG_TYPE_PUSH_GROUP_ARB:          printf("Push Group"); break;
    // case GL_DEBUG_TYPE_POP_GROUP_ARB:           printf("Pop Group"); break;
    case GL_DEBUG_TYPE_OTHER_ARB:               printf("Other"); break;
    default: UNREACHABLE("unkown error type");
    }
    printf("\n");
}
static void error_callback(int error_code, const char* description)
{
    // error key used to make the input array
    if (error_code == 65539) 
        return ;
    rd_log(rd_error, "(%d): %s\n", error_code, description);
}
#endif

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    UNUSED(window);
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    
    if (current_render)
    {
        current_render->screen_width = width;
        current_render->screen_height = height;
        current_render->screen_ratio = (float)width / (float)height;
        if (current_render->resize_callback)
            current_render->resize_callback(current_render, width, height);
    }
}

static GLFWwindow *init_GLFWwindow(const char *title, int width, int height)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    

#ifdef DEBUG 
    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_DEBUG, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    
    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        rd_log(rd_error, "Failed to create GLFW window");
        glfwTerminate();
        return NULL;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    
    if (!gladLoaderLoadGL())
    {
        rd_log(rd_error, "Failed to initialize GLAD");
        glfwTerminate();
        return NULL;
    }
    

#ifdef DEBUG 

    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

    //if (1 || (flags & 0 /* GL_CONTEXT_FLAG_DEBUG_BIT */))
    {
        // glEnable(GL_DEBUG_OUTPUT_ARB);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
        glDebugMessageCallbackARB(debugMessage, NULL);
        glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    }
#endif
    
    glViewport(0, 0, width, height);
    rd_set_clear_color(GRAY);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);

    return window;
}
bool rd_should_close(void)
{
    return (glfwWindowShouldClose(current_render->glfw) || current_render->to_close);
}
void rd_set_to_close(void)
{
    current_render->to_close = true;
}

// {0} on error
// do not set uniform loc
Texture rd_load_texture(const char *tex_name, int tex_unit_index)
{
    assert(0 <= tex_unit_index && tex_unit_index < 16);

    int width, height, nrChannels = 0;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    uint8_t *data = stbi_load(tex_name, &width, &height, &nrChannels, 0); 
    if (!data) return (Texture){0};


    // GL_INVALID_VALUE; // ?? 
    // Texture_id texture = 0;
    Texture res = { 
        .texture_unit  = tex_unit_index,
        .loc = -1
    };
    
    glGenTextures(1, &res.id); 
    glActiveTexture(GL_TEXTURE0 + tex_unit_index); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, res.id);
    
    if (1)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    else 
    {   
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, (float[]){ 0.0f, 1.0f, 0.0f, 1.0f });
    }
    
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    

    glTexImage2D(GL_TEXTURE_2D, 
        0, GL_RGBA, width, height, 0,
        (nrChannels == 3)
            ? GL_RGB 
            : ((nrChannels == 4)
                ? GL_RGBA 
                : (abort(), 0)
            ),
        GL_UNSIGNED_BYTE, data
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    assert(glIsTexture(res.id));

    stbi_image_free(data);
    return res;
}



static void rd_poll_inputs(void)
{
    glfwPollEvents();
    foreach_static (size_t, i, current_render->inputs.keys)
        switch (current_render->inputs.keys[i])
        {
        case key_normal:
            current_render->inputs.keys[i] = (glfwGetKey(current_render->glfw, i) == GLFW_PRESS)
                                        ? key_pressed
                                        : key_normal;
            break;
        case key_down:
        case key_pressed:
            current_render->inputs.keys[i] = (glfwGetKey(current_render->glfw, i) == GLFW_PRESS)
                                        ? key_down
                                        : key_released;
            break;
        case key_released:
            current_render->inputs.keys[i] = (glfwGetKey(current_render->glfw, i) == GLFW_PRESS)
                                        ? key_pressed
                                        : key_normal;
            break;
        case key_repeat:
            assert(((void)"implement key repeat", 0));
            break;
        case key_not_a_key: break;
        default: break;
        }
    
    foreach_static (size_t, i, current_render->inputs.mouse_button)
        switch (current_render->inputs.mouse_button[i])
        {
        case key_normal:
            current_render->inputs.mouse_button[i] = 
                (glfwGetMouseButton(current_render->glfw, i) == GLFW_PRESS)
                                        ? key_pressed
                                        : key_normal;
            break;
        case key_down:
        case key_pressed:
            current_render->inputs.keys[i] = 
                (glfwGetMouseButton(current_render->glfw, i) == GLFW_PRESS)
                                        ? key_down
                                        : key_released;
            break;
        case key_released:
            current_render->inputs.keys[i] = 
                (glfwGetMouseButton(current_render->glfw, i) == GLFW_PRESS)
                                        ? key_pressed
                                        : key_normal;
            break;

        case key_repeat: TODO("implement mouse button repeat"); break;
        case key_not_a_key: break;
        default: break;
        }
}
static void rd_calculate_frame_time(void)
{
    current_render->frame_time.start_frame_time = current_render->frame_time.end_frame_time;
    current_render->frame_time.end_frame_time = glfwGetTime();
    current_render->frame_time.delta_time = current_render->frame_time.end_frame_time - current_render->frame_time.start_frame_time;
    current_render->frame_time.fps = (int)(1. / current_render->frame_time.delta_time);
}

static void fun_swap_buffers(GLFWwindow *window)
{
    glfwSwapBuffers(window);
}
static void fun_glClear(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // glClearDepth();
}

void rd_end_frame(void)
{
    fun_swap_buffers(current_render->glfw);
    rd_calculate_frame_time();

    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_DST_COLOR);
    // glBlendFunc(GL_SRC_ALPHA, GL_DST_COLOR);
    fun_glClear();
    rd_poll_inputs();
}
void rd_set_clear_color(Color clear_color)
{
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
}



/* #pragma region // ui */
/* #pragma region // inputs */

// use GLFW key defines
Key_state rd_get_key_state(int key)
{
    if (0 <= key && key < KEY_COUNT) 
        return (Key_state)current_render->inputs.keys[key];
    return (Key_state)key_not_a_key;
}
bool rd_is_key_down(int key)
{
    if (0 <= key && key < KEY_COUNT) 
        return (current_render->inputs.keys[key] & key_down) != 0;
    return false;
}
bool rd_is_key_up(int key)
{
    if (0 <= key && key < KEY_COUNT) 
        return current_render->inputs.keys[key] == key_normal;
    return false;
}
bool rd_is_key_pressed(int key)
{
    if (0 <= key && key < KEY_COUNT) 
        return current_render->inputs.keys[key] == key_pressed;
    return false;
}
bool rd_is_key_released(int key)
{
    if (0 <= key && key < KEY_COUNT) 
        return current_render->inputs.keys[key] == key_released;
    return false;
}

Vec2 rd_get_cursor_pos(void)
{
    double dx = 0.0;
    double dy = 0.0;
    glfwGetCursorPos(current_render->glfw, &dx, &dy);
    
    return (Vec2){ dx, dy };
}

// use GLFW key defines [0;2]
Key_state rd_get_mouse_button_state(int button)
{
    if (0 <= button && button < MOUSE_BOUTTON_COUNT)
        return (Key_state)current_render->inputs.mouse_button[button];
    return (Key_state)key_not_a_key;
}
bool rd_is_mouse_button_down(int button)
{
    if (0 <= button && button < MOUSE_BOUTTON_COUNT) 
        return (current_render->inputs.mouse_button[button] & key_down) != 0;
    return false;
}
bool rd_is_mouse_button_up(int button)
{
    if (0 <= button && button < MOUSE_BOUTTON_COUNT) 
        return current_render->inputs.mouse_button[button] == key_normal;
    return false;
}
bool rd_is_mouse_button_pressed(int button)
{
    if (0 <= button && button < MOUSE_BOUTTON_COUNT) 
        return current_render->inputs.mouse_button[button] == key_pressed;
    return false;
}
bool rd_is_mouse_button_released(int button)
{
    if (0 <= button && button < MOUSE_BOUTTON_COUNT) 
        return current_render->inputs.mouse_button[button] == key_released;
    return false;
}




/* #pragma end_region // inputs */


bool rd_Button(Button *button)
{
    if (button->state != rd_disabled)
    {
        if (is_Vec2_colliding_Rect(
            rd_get_cursor_pos(),
            Rect_transform_by_Camera(current_render->ui_camera, button->bounds))
        ) {
            if (rd_is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT))
                button->state = rd_pressed;
            else button->state = rd_focused;
        }
        else button->state = rd_normal;
    }
    
    current_render->is_mouse_input_button_captured = button->state == rd_pressed 
                                                  || button->state == rd_focused;
    return button->state == rd_pressed;
}
void draw_Button(Button button)
{
    // TODO
    UNUSED(button); 
    // GuiDrawButton(button.bounds, button.text, button.state);
}


/* #pragma end_region // ui */



/* #pragma region // coordinate */


int rd_get_fps(void)
{
    return current_render->frame_time.fps;
}
float rd_get_delta_time(void)
{
    return current_render->frame_time.delta_time;
}
float rd_get_target_fps(void)
{
    return current_render->frame_time.target_fps;
}
float rd_get_screen_width(void)
{
    return current_render->screen_width;
}
float rd_get_screen_height(void)
{
    return current_render->screen_height;
}
float rd_get_screen_ratio(void)
{
    return current_render->screen_ratio;
}
Vec2 rd_get_screen_to_world(Camera cam, Vec2 pos)
{
    return (Vec2){
        (pos.x - cam.target.x) * cam.zoom,
        (pos.y - cam.target.y) * cam.zoom
    };
}
Rect Rect_transform_by_Camera(Camera cam, Rect rec)
{
    return (Rect){
        .x = (rec.x - cam.target.x) * cam.zoom,
        .y = (rec.y - cam.target.y) * cam.zoom,
        .width = rec.width * cam.zoom,
        .height = rec.height * cam.zoom
    };
}

bool rd_collision_Vec2_Rect(Vec2 vec, Rect rec)
{
    return (
        rec.x <= vec.x && vec.x <= rec.x + rec.width
     && rec.y <= vec.y && vec.y <= rec.y + rec.height
    );
}

/* #pragma end_region // coordinate */




Render *rd_init(const char *title, int width, int height, rd_resize_callback_t callback)
{
    Render *res = calloc(1, sizeof(Render));
    if (!res) return NULL;
    current_render = res;
    
    *res = (Render){
        .glfw = init_GLFWwindow(title, width, height),
        .inputs = { .keys = {0} },
        .resize_callback = callback,
    };
    
    glfwSwapInterval(1);
    if (0) printf("window refreshRate: %i\n", glfwGetVideoMode(glfwGetPrimaryMonitor())->refreshRate);
    
    res->screen_width = width;
    res->screen_height = height;

    res->frame_time.target_fps = 60;
    res->frame_time.start_frame_time = glfwGetTime();
    rd_calculate_frame_time();

    if (!res->glfw)
    {
        free(res);
        glfwTerminate(); 
        return NULL;
    }

    // if (!glfwSetKeyCallback(res.glfw, key_callback))
    //     printf("[ERROR] failed to set the key callback\n");
    foreach_static (size_t, i, res->inputs.keys)
    {
        glfwGetKey(res->glfw, i);
        int err = glfwGetError(NULL);
        if (err == GLFW_INVALID_ENUM) res->inputs.keys[i] = key_not_a_key;
        else if (err) abort();
    }

    return res;
}

void rd_deinit(void)
{
    glfwDestroyWindow(current_render->glfw);
    glfwTerminate();
    free(current_render);
    current_render = NULL;
}




static Shader_id rd_load_a_shader(Shader_metadata data)
{
    // reset log anyway
    print_buffer[0] = '\0';
    print_buffer_size = 0;

    Strb shader_string = {0};
    

    if (!Strb_cat_file(&shader_string, data.file_path))
    {
        Strb_free(shader_string);
        rd_log(rd_error, "can't open shader: %s", data.file_path);
        return 0;
    }
    Strb_cat_null(&shader_string);

    Shader_id gl_shader = glCreateShader(data.type);
    glShaderSource(gl_shader, 1, (const char* const*)&shader_string.arr, &shader_string.size);


    glCompileShader(gl_shader);
    Strb_free(shader_string);
    

    // check for shader compile errors
    int success = 0;
    glGetShaderiv(gl_shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(gl_shader, sizeof(print_buffer), NULL, print_buffer);
        rd_log(rd_error, "shader failed to compile: %s:\n\t%s", data.file_path, print_buffer);
        glDeleteShader(gl_shader);
        return 0;
    }
    return gl_shader;
}

#define CALL_IF_NNULL(fun, ...) (fun ? fun(__VA_ARGS__) : true)
// static_uniform_setter -> for uniform that don't change often
static bool rd_dummy_reload_shader(Shader *shader, void *uniform_setter_arg)
{
    // assert(0 <= shader_count && shader_count < 20);
    
    da_Shader_id compile_shaders = {0};
    
    foreach_ptr (Shader_metadata, sd, &shader->files)
    {
        da_push(&compile_shaders, rd_load_a_shader(*sd));
        if (!da_top(&compile_shaders))
        {
            foreach_ptr (Shader_id, id, &compile_shaders)
                glDeleteShader(*id);
            da_free(&compile_shaders);
            return false; // no state change
        }
    }
    
    // link shaders
    Shader_programme_id new_shader_program = glCreateProgram();
    
    foreach_ptr (Shader_id, id, &compile_shaders)
        glAttachShader(new_shader_program, *id);
    
    glLinkProgram(new_shader_program);

    // maybe too early
    foreach_ptr (Shader_id, id, &compile_shaders)
        glDeleteShader(*id);
    da_free(&compile_shaders);
    
    int success = 0;
    // check for linking errors
    glGetProgramiv(new_shader_program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(new_shader_program, sizeof(print_buffer), NULL, print_buffer);
        rd_log(rd_error, "shader linking failed:\n%s", print_buffer);

        return false; // no state change
    }

    glValidateProgram(new_shader_program);
    
    // only if function exist
    if (!CALL_IF_NNULL(shader->static_uniform_setter, new_shader_program, uniform_setter_arg))
    {
        rd_log(rd_warning, "failed to set uniform, no reload");
        return false;
    }

    if (shader->program) glDeleteProgram(shader->program);
    shader->program = new_shader_program;
    return true;
}

static void rd_successful_shader_load(const Shader *shader, bool is_reload)
{              
    if (is_reload) rd_log(rd_info, "shader reloaded successfuly");
    else rd_log(rd_info, "shader loaded successfuly");
    
    foreach_ptr (Shader_metadata, it, &shader->files)
    {
        printf("\t%s: ", it->file_path);
        switch (it->type)
        {
        case GL_VERTEX_SHADER: printf("vertex shader"); break;
        case GL_GEOMETRY_SHADER: printf("geometry shader"); break;
        case GL_FRAGMENT_SHADER: printf("fragment shader"); break;
        default: UNREACHABLE("");
        }
        printf("\n");
    }
}

bool rd_reload_shader(Shader *shader, void *uniform_setter_arg)
{
    bool res = rd_dummy_reload_shader(shader, uniform_setter_arg);
    if (res) rd_successful_shader_load(shader, true);//rd_log(rd_info, "shader reload successfuly");
    return res;
}

// va_list: pair of const char* shader name and shader type 
bool _rd_load_shader(Shader *res, Uniform_setter_fun_t uniform_setter, void *uniform_setter_arg, ...)
{
    res->static_uniform_setter = uniform_setter;

    va_list args;
    va_start(args, uniform_setter_arg);

    char const* shader_path = NULL;
    GLenum type = 0;
    
    while (1)
    {
        shader_path = va_arg(args, char const*);
        if (shader_path == NULL) break;

        type = va_arg(args, GLenum);
        if (type == 0)
        {
            da_free(&res->files);
            *res = (Shader){0};
            rd_log(rd_error, "exptected a type not a NULL in va_list");
            return false;
        }
        if (type != GL_VERTEX_SHADER 
         && type != GL_GEOMETRY_SHADER
         && type != GL_FRAGMENT_SHADER
        ) {
            da_free(&res->files);
            *res = (Shader){0};
            rd_log(rd_error, "exptected a type in the following GL_VERTEX_SHADER, GL_GEOMETRY_SHADER or GL_FRAGMENT_SHADER got %x", type);
            return false;
        }
        da_push(&res->files, ((Shader_metadata){
            .file_path = shader_path,
            .type = type
        }));
    }
    if (res->files.size == 0)
    {
        da_free(&res->files);
        *res = (Shader){0};
        rd_log(rd_warning, "no shader path provided");
        return false;
    }

    va_end(args);
    bool no_err = rd_dummy_reload_shader(res, uniform_setter_arg);
    if (no_err) rd_successful_shader_load(res, false);

    return no_err;
}

#define RD_MAKE_GET_EASY(ret_T, gl_get_fun, gl_enum)\
    static ret_T rd_get_ ## gl_enum()\
    {\
        ret_T res = 0;\
        gl_get_fun(gl_enum, &res);\
        return res;\
    }

RD_MAKE_GET_EASY(int, glGetIntegerv, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
RD_MAKE_GET_EASY(int, glGetIntegerv, GL_MAX_TEXTURE_IMAGE_UNITS)
RD_MAKE_GET_EASY(int, glGetIntegerv, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS)
RD_MAKE_GET_EASY(int, glGetIntegerv, GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS)

// -1 on error
int rd_dispatch_texture_unit(Shader *shader, GLenum stage)
{
    static int max_combined_tiu = -1;
    if (max_combined_tiu == -1) max_combined_tiu = rd_get_GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS();
    static int max_vertex_tiu   = -1;
    if (max_vertex_tiu == -1) max_vertex_tiu = rd_get_GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS();
    static int max_geometry_tiu = -1;
    if (max_geometry_tiu == -1) max_geometry_tiu = rd_get_GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS();
    static int max_fragment_tiu = -1;
    if (max_fragment_tiu == -1) max_fragment_tiu = rd_get_GL_MAX_TEXTURE_IMAGE_UNITS();

    if (max_combined_tiu >= shader->tiu_vertex_dispatcher
                          + shader->tiu_geometry_dispatcher 
                          + shader->tiu_fragment_dispatcher)
        return -1;
    
    switch (stage)
    {
    case GL_VERTEX_SHADER:
        if (max_vertex_tiu >= shader->tiu_vertex_dispatcher)
            return -1;
        return shader->tiu_vertex_dispatcher++;

    case GL_GEOMETRY_SHADER:
        if (max_geometry_tiu >= shader->tiu_geometry_dispatcher)
            return -1;
        return shader->tiu_geometry_dispatcher++;

    case GL_FRAGMENT_SHADER:
        if (max_fragment_tiu >= shader->tiu_fragment_dispatcher)
            return -1;
        return shader->tiu_fragment_dispatcher++;

    default: UNREACHABLE("[ERROR] invalide shader stage");
    }
}

void rd_unload_shader(Shader *shader)
{
    glDeleteProgram(shader->program);
}
