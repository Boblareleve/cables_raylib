#version 410 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
	FragColor = 
	// texture(texture1, TexCoord);
	mix(
		mix(
			vec4(ourColor, 1.0),
			texture(texture1, TexCoord), 
			0.5 	
		),
		texture(texture2, TexCoord), 
		1.0 / 3.0
	);
}
