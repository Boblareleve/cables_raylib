#define STB_IMAGE_IMPLEMENTATION
#define PRINT_IMPLEMENTATION
#define STRING_IMPLEMENTATION
#define STRING_FILEIO
#include "render.h"

#include <GLFW/glfw3.h>


int main()
{
    Render *render = rd_init("test", 400, 400, NULL);

    rd_set_clear_color(DARKGRAY);


    float vvv[3][2] = {
        {0, 0},
        {0.5, 0},
        {0, 0.5}
    };
    Shader debug_shader = {0};
    GLuint VAO = 0;
    GLuint VBO = 0;
    {
        if (!rd_load_shader(
            &debug_shader,
            NULL,
            NULL,
            "shaders/texture.vs",
            GL_VERTEX_SHADER,
            "shaders/texture.gs",
            GL_GEOMETRY_SHADER,
            "shaders/texture.fs",
            GL_FRAGMENT_SHADER
        )) assert(0);



        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vvv), vvv, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);
        glEnableVertexAttribArray(0);
    }


    while (!rd_should_close())
    {
        if (rd_is_key_pressed(GLFW_KEY_SPACE))  
            if (!rd_reload_shader(&debug_shader, NULL)) (void)0;
        if (rd_is_key_down(GLFW_KEY_ESCAPE)) rd_set_to_close();

        glUseProgram(debug_shader.program);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, ARRAY_LEN(vvv));
        rd_end_frame();
    }

    rd_deinit();
}