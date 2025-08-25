#version 410 core
#version 410 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in uint index;


// layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec3 aColor;
// layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;

void main()
{
	int a = 0;
    int b = 0;
    a = a & b;
    a = a | b;

	gl_Position = vec4(aPos, 1.0);
	ourColor = aColor;
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}

