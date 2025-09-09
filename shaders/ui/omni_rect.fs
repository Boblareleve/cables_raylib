#version 410 core
out vec4 FragColor;

in GS_OUT {
	vec2 	  	tex_pos;
    flat vec4   fill_color;
    flat vec4   border_color;
	flat vec4   on_hover_filter;
    flat float  border_thickness;
    flat float  texture_index;
} fs_in;

uniform sampler2D atlas;
const float atlas_width = 1280;
const float atlas_stride = 20;
const float atlas_texture_count = 2;
const float atlas_texture_width = 16;

vec4 blend(vec4 a, vec4 b)
{
	return a * (1.0 - b.a) + b * b.a;
}

void main()
{
	if (        abs(fs_in.tex_pos.x) <= fs_in.border_thickness 
	 || 1.0 - fs_in.border_thickness <= abs(fs_in.tex_pos.x)
	 ||         abs(fs_in.tex_pos.y) <= fs_in.border_thickness 
	 || 1.0 - fs_in.border_thickness <= abs(fs_in.tex_pos.y))
		FragColor = fs_in.border_color;
	else 
		FragColor = fs_in.fill_color;
	if (fs_in.texture_index > 0.)
	{
		// top left corner in normalize space
		vec2 tex_offset = vec2(
			// 0.0,
			// float(fs_in.texture_index-5),
			// (atlas_stride * float(fs_in.texture_index-1)) / atlas_width,
			(atlas_stride * (fs_in.texture_index - 1)) / atlas_width,
			0.0
		);

		vec4 tex_color = texture(
			atlas,
			tex_offset + vec2( 
				fs_in.tex_pos.x * atlas_texture_width / atlas_width, 
				fs_in.tex_pos.y
			)
		);
		FragColor = blend(FragColor, tex_color);

		// FragColor = vec4(
		// 	mix(
		// 		fs_in.fill_color.rgb, 
		// 		tex_color.rgb, 
		// 		tex_color.a
		// 	),
		// 	clamp(tex_color.a + fs_in.fill_color.a, 0.0, 1.0)
		// 	// tex_color.rgb * tex_color.a + fs_in.fill_color.rgb * fs_in.fill_color.a,
		// 	// tex_color.a + fs_in.fill_color.a
		// );
	}
	
	FragColor += fs_in.on_hover_filter; // use forth channels as bright scale
	// FragColor.rgb *= fs_in.on_hover_filter.w + 1.0; // use forth channels as bright scale
	// FragColor.rgb += fs_in.on_hover_filter.rgb;     // use .rgb as added color
}
