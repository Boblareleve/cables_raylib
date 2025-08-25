#version 410 core
layout (location = 0) in vec4 rect;
layout (location = 1) in vec4 color;
layout (location = 2) in uint type;
layout (location = 3) in uint texture_index;


uniform float ui_zoom;

in VS_OUT {
    vec4 color;
    uint texture_index;
    uint type;
    vec2 dim;
} vs_out;

void main()
{
    gl_Position = vec4(rect.xy, 0.0, 1.0); 
}
