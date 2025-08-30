#version 410 core
out vec4 FragColor;

in GS_OUT {
    vec2 tex_pos;
    flat vec4 fill_color;
    flat vec4 border_color;
    flat float border_thickness;
    flat uint texture_index;
} fs_in;

uniform sampler2D atlas;
const float atlas_width = 0;
const float atlas_stride = 0;
const float atlas_texture_count = 0;
const float atlas_texture_width = 0;

void main()
{
	// FragColor = vec4(0.0, 1.0, 0.0, 1.0);
	// FragColor = fs_in.fill_color;
	// return ;


	if (fs_in.tex_pos.x <= fs_in.border_thickness 
	 || 1.0 - fs_in.border_thickness <= fs_in.tex_pos.x
	 || fs_in.tex_pos.y <= fs_in.border_thickness 
	 || 1.0 - fs_in.border_thickness <= fs_in.tex_pos.y)
		FragColor = fs_in.border_color;
	else if (fs_in.texture_index == 0)
		FragColor = fs_in.fill_color;
	else {
		// top left corner in normalize space
		vec2 tex_offset = vec2(
			(atlas_stride * float(fs_in.texture_index-1)) / atlas_width,
			0.0
		);
		
		vec4 tex_color = texture(
			atlas, 
			tex_offset + vec2( 
				fs_in.tex_pos.x * atlas_texture_width / atlas_width, 
				fs_in.tex_pos.y
			)
		);
		FragColor = vec4(
			tex_color.rgb * tex_color.a + fs_in.fill_color.rgb * fs_in.fill_color.a,
			tex_color.a + fs_in.fill_color.a
		);
		FragColor /= FragColor.a;
		// else
		// {
		// 	vec2 tc = fs_in.tex_pos * vec2(16.0 / 4096.0, 1.0);
    	// 	tc.x += float(fs_in.texture_index) * 16.0 / 4096.0;
		// 	FragColor = texture(atlas, tc)
		// }
	}
}
