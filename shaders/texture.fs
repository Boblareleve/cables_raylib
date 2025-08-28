#version 410 core
out vec4 FragColor;

in vec2 TexCoord;


// texture samplers
uniform sampler2D atlas;

uniform sampler2D ui_texture1;

void main()
{
	FragColor = 
		texture(atlas, TexCoord);
		// mix(
		// 	texture(atlas, TexCoord),
		// 	vec4(1.0, 0.0, 0.0, 1.0),
		// 	0.0
		// );
}
