#version 410 core
out vec4 FragColor;

out GS_OUT {
    vec2 TexCoord;
    uint type;
    uint texture_index;
    vec4 color;
} fs_in;

// texture samplers
uniform sampler2D atlas;
uniform sampler2D ui_texture[15];

void main()
{
	if (fs_in.type == 0 // rectangle
	 || fs_in.type == 1 // rectangle lignes
	) {
		FragColor = fs_in.color;
	}
	else if (fs_in.type == 2) // texture
	{
		if (fs_in.type <= 0)
			FragColor = texture(ui_texture[fs_in.texture_index], fs_in.TexCoord);
		else 
		{
			vec2 tc = fs_in.TexCoord * vec2(16.0 / 4096.0, 1.0);
    		tc.x += float(fs_in.texture_index) * 16.0 / 4096.0;
			FragColor = texture(atlas, tc)
		}
	}
}
