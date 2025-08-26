#version 410 core
out vec4 FragColor;

in vec4 color;
in vec2 TexCoord;


// texture samplers
uniform sampler2D atlas;
uniform sampler2D ui_texture1;

void main()
{
	FragColor = vec4(1.0, 0.0, 0.0, 1.0); // vec4(color.xyz, 0.5);
		// mix(
		// 	vec4(color.xyz, 0.5), 
		// 	texture(atlas, TexCoord), 
		// 	0.7
		// );
}
