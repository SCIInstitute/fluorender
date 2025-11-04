//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#ifndef ImgShaderCode_h
#define ImgShaderCode_h

inline constexpr const char* IMG_VERTEX_CODE = R"GLSHDR(
//IMG_VERTEX_CODE
layout(location = 0) in vec3 InVertex;
layout(location = 1) in vec3 InTexCoord;
out vec3 OutVertex;
out vec3 OutTexCoord;
	
void main()
{
	gl_Position = vec4(InVertex, 1.0);
	OutTexCoord = InTexCoord;
	OutVertex = InVertex;
}
)GLSHDR";

inline constexpr const char* IMG_VTX_CODE_DRAW_GEOMETRY = R"GLSHDR(
//IMG_VTX_CODE_DRAW_GEOMETRY
layout(location = 0) in vec3 InVertex;
uniform mat4 matrix0;//transformation
	
void main()
{
	gl_Position = matrix0 * vec4(InVertex, 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_VTX_CODE_DRAW_GEOMETRY_COLOR_UNI = R"GLSHDR(
//IMG_VTX_CODE_DRAW_GEOMETRY_COLOR3
layout(location = 0) in vec3 InVertex;
out vec3 OutColor;
uniform mat4 matrix0;//transformation
uniform vec4 loc1; //(color, 1.0)
	
void main()
{
	gl_Position = matrix0 * vec4(InVertex, 1.0);
	OutColor = loc1.xyz;
}
)GLSHDR";

inline constexpr const char* IMG_VTX_CODE_DRAW_GEOMETRY_COLOR3 = R"GLSHDR(
//IMG_VTX_CODE_DRAW_GEOMETRY_COLOR3
layout(location = 0) in vec3 InVertex;
layout(location = 1) in vec3 InColor;
out vec3 OutColor;
uniform mat4 matrix0;//transformation
	
void main()
{
	gl_Position = matrix0 * vec4(InVertex, 1.0);
	OutColor = InColor;
}
)GLSHDR";

inline constexpr const char* IMG_VTX_CODE_DRAW_GEOMETRY_COLOR4 = R"GLSHDR(
//IMG_VTX_CODE_DRAW_GEOMETRY_COLOR4
layout(location = 0) in vec3 InVertex;
layout(location = 1) in vec4 InColor;
out vec4 OutColor;
uniform mat4 matrix0;//transformation
	
void main()
{
	gl_Position = matrix0 * vec4(InVertex, 1.0);
	OutColor = InColor;
}
)GLSHDR";

inline constexpr const char* IMG_VTX_CODE_DRAW_TEXT = R"GLSHDR(
//IMG_VTX_CODE_DRAW_TEXT
layout(location = 0) in vec4 coord;
out vec2 texcoord;
uniform mat4 matrix0;//transformation
	
void main(void)
{
	gl_Position = matrix0 * vec4(coord.xy, 0, 1);
	texcoord = coord.zw;
}
)GLSHDR";

inline constexpr const char* IMG_FRG_CODE_DRAW_GEOMETRY = R"GLSHDR(
//IMG_FRG_CODE_DRAW_GEOMETRY
out vec4 FragColor;
uniform vec4 loc0;
	
void main()
{
	FragColor = loc0;
}
)GLSHDR";

inline constexpr const char* IMG_FRG_CODE_DRAW_GEOMETRY_COLOR3 = R"GLSHDR(
//IMG_FRG_CODE_DRAW_GEOMETRY_COLOR3
in vec3 OutColor;
out vec4 FragColor;
	
void main()
{
	FragColor = vec4(OutColor, 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_FRG_CODE_DRAW_GEOMETRY_COLOR4 = R"GLSHDR(
//IMG_FRG_CODE_DRAW_GEOMETRY_COLOR4
in vec4 OutColor;
out vec4 FragColor;
	
void main()
{
	FragColor = OutColor;
}
)GLSHDR";

inline constexpr const char* IMG_FRG_CODE_DRAW_TEXT = R"GLSHDR(
//IMG_FRG_CODE_DRAW_TEXT
in vec2 texcoord;
out vec4 FragColor;
uniform sampler2D tex0;
uniform vec4 loc0;//color
	
void main(void)
{
	FragColor = vec4(1, 1, 1, texture(tex0, texcoord).r) * loc0;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_TEXTURE_LOOKUP = R"GLSHDR(
//IMG_SHADER_CODE_TEXTURE_LOOKUP
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_TEXTURE_LOOKUP
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	FragColor = c;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_TEXTURE_FLIP = R"GLSHDR(
//IMG_SHADER_CODE_TEXTURE_FLIP
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_TEXTURE_FLIP
uniform sampler2D tex0;
	
void main()
{
	vec2 t = vec2(OutTexCoord.x, 1.0 - OutTexCoord.y);
	vec4 c = texture(tex0, t);
	FragColor = c;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_BRIGHTNESS_CONTRAST = R"GLSHDR(
//IMG_SHADER_CODE_BRIGHTNESS_CONTRAST
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_BRIGHTNESS_CONTRAST
uniform vec4 loc0; //(r_gamma, g_gamma, b_gamma, 1.0)
uniform vec4 loc1; //(r_brightness, g_brightness, b_brightness, 1.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	vec4 b = loc1;
	b.x = b.x>1.0?1.0/(2.0-b.x):loc1.x;
	b.y = b.y>1.0?1.0/(2.0-b.y):loc1.y;
	b.z = b.z>1.0?1.0/(2.0-b.z):loc1.z;
	FragColor = pow(c, loc0)*b;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR = R"GLSHDR(
//IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR
uniform vec4 loc0; //(r_gamma, g_gamma, b_gamma, 1.0)
uniform vec4 loc1; //(r_brightness, g_brightness, b_brightness, 1.0)
uniform vec4 loc2; //(r_hdr, g_hdr, b_hdr, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	vec4 c_avg_1 = max(textureLod(tex0, t.xy, 1), 0.001);
	vec4 c_avg_2 = max(textureLod(tex0, t.xy, 2), 0.001);
	vec4 c_avg_3 = max(textureLod(tex0, t.xy, 3), 0.001);
	vec4 c_avg_4 = max(textureLod(tex0, t.xy, 4), 0.001);
	vec4 c_avg_5 = max(textureLod(tex0, t.xy, 5), 0.001);
	vec4 b = loc1;
	b.x = b.x>1.0?(b.x<2.0?1.0/(2.0-b.x):256.0):loc1.x;
	b.y = b.y>1.0?(b.y<2.0?1.0/(2.0-b.y):256.0):loc1.y;
	b.z = b.z>1.0?(b.z<2.0?1.0/(2.0-b.z):256.0):loc1.z;
	c = pow(c, loc0);
	c_avg_1 = c/pow(c_avg_1, loc0);
	c_avg_2 = c/pow(c_avg_2, loc0);
	c_avg_3 = c/pow(c_avg_3, loc0);
	c_avg_4 = c/pow(c_avg_4, loc0);
	c_avg_5 = c/pow(c_avg_5, loc0);
	c *= b;
	vec4 c_avg = 0.43*c_avg_1+0.26*c_avg_2+0.17*c_avg_3+0.1*c_avg_4+0.04*c_avg_5;
	vec4 unit = vec4(1.0);
	c = c*(unit-loc2)+loc2*c_avg;
	FragColor = c*(unit-loc2)+loc2*(unit-unit/(c+unit));
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRADIENT_MAP = R"GLSHDR(
//IMG_SHADER_CODE_GRADIENT_MAP
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_GRADIENT_MAP
uniform vec4 loc0; //(lo, hi, hi-lo, alpha) 
uniform vec4 loc6; //(r, g, b, inv)
uniform vec4 loc9; //(r, g, b, 0)
uniform sampler2D tex0;
	
void main()
{
	vec4 rb;
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	rb.a = (loc0.w>0.5?loc0.w:c.x)*c.a;
	float valu = (c.r + c.g + c.b)/3.0;
	valu = (valu-loc0.x)/loc0.z;
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRADIENT_MAP_RESULT = R"GLSHDR(
	//IMG_SHADER_CODE_GRADIENT_MAP_RESULT
	FragColor = vec4(rb.rgb*rb.a, rb.a);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRADIENT_PROJ_MAP = R"GLSHDR(
//IMG_SHADER_CODE_GRADIENT_MAP
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_GRADIENT_MAP
uniform vec4 loc6; //(lo, hi, hi-lo, inv)
uniform vec4 loc9; //(main color)
uniform vec4 loc16; //(alt color)
uniform vec4 loc18;//(alpha, alpha_power, luminance, 0)
uniform sampler2D tex0;
	
void main()
{
	vec4 rb;
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	rb.a = c.a;
	float l1 = c.r - c.b * 247.3;
	float l2 = c.g - c.b * 53.1;
	float valu = 0.7 * l1 + 0.3 * l2;
	valu = (valu-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRADIENT_PROJ_MAP_RESULT = R"GLSHDR(
	//IMG_SHADER_CODE_GRADIENT_MAP_RESULT
	float alpha = pow(loc18.x, loc18.y);
	FragColor = vec4(rb.rgb*loc18.z*alpha, rb.a);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_FILTER_MIN = R"GLSHDR(
//IMG_SHADER_CODE_FILTER_MIN
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_FILTER_MIN
uniform vec4 loc0; //(width, height, thresh, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	if (min(c.r, min(c.g, c.b)) >= loc0.z)
	{
		FragColor = c;
		return;
	}
	vec4 c1 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c2 = texture(tex0, vec2(t.x, t.y-loc0.y));
	vec4 c3 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c4 = texture(tex0, vec2(t.x-loc0.x, t.y));
	vec4 c5 = texture(tex0, vec2(t.x+loc0.x, t.y));
	vec4 c6 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));
	vec4 c7 = texture(tex0, vec2(t.x, t.y+loc0.y));
	vec4 c8 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));
	c = min(c, c1);
	c = min(c, c2);
	c = min(c, c3);
	c = min(c, c4);
	c = min(c, c5);
	c = min(c, c6);
	c = min(c, c7);
	c = min(c, c8);
	FragColor = c;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_FILTER_MAX = R"GLSHDR(
//IMG_SHADER_CODE_FILTER_MAX
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_FILTER_MAX
uniform vec4 loc0; //(width, height, scale, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	float s = pow(1.0-max(c.r, max(c.g, c.b)), 3.0)*loc0.z;
	vec4 c1 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c2 = texture(tex0, vec2(t.x, t.y-loc0.y));
	vec4 c3 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c4 = texture(tex0, vec2(t.x-loc0.x, t.y));
	vec4 c5 = texture(tex0, vec2(t.x+loc0.x, t.y));
	vec4 c6 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));
	vec4 c7 = texture(tex0, vec2(t.x, t.y+loc0.y));
	vec4 c8 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));
	c = max(c, c1);
	c = max(c, c2);
	c = max(c, c3);
	c = max(c, c4);
	c = max(c, c5);
	c = max(c, c6);
	c = max(c, c7);
	c = max(c, c8);
	FragColor = c;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_FILTER_BLUR = R"GLSHDR(
//IMG_SHADER_CODE_FILTER_BLUR
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_FILTER_BLUR
uniform vec4 loc0; //(width, height, dx, dy)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	vec4 c1 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c2 = texture(tex0, vec2(t.x, t.y-loc0.y));
	vec4 c3 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c4 = texture(tex0, vec2(t.x-loc0.x, t.y));
	vec4 c5 = texture(tex0, vec2(t.x+loc0.x, t.y));
	vec4 c6 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));
	vec4 c7 = texture(tex0, vec2(t.x, t.y+loc0.y));
	vec4 c8 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));
	FragColor = (c1+c2+c3+c4+c5+c6+c7+c8)/8.0;
	//FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_FILTER_SHARPEN = R"GLSHDR(
//IMG_SHADER_CODE_FILTER_SHARPEN
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_FILTER_SHARPEN
uniform vec4 loc0; //(width, height, 0.0, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	vec4 c1 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c2 = texture(tex0, vec2(t.x, t.y-loc0.y));
	vec4 c3 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c4 = texture(tex0, vec2(t.x-loc0.x, t.y));
	vec4 c5 = texture(tex0, vec2(t.x+loc0.x, t.y));
	vec4 c6 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));
	vec4 c7 = texture(tex0, vec2(t.x, t.y+loc0.y));
	vec4 c8 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));
	c = c*9.0 - (c1+c2+c3+c4+c5+c6+c7+c8);
	FragColor = c;
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_DEPTH_TO_OUTLINES = R"GLSHDR(
//IMG_SHADER_CODE_DEPTH_TO_OUTLINES
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
//IMG_SHADER_CODE_DEPTH_TO_OUTLINES
uniform vec4 loc0; //(width, height, 0.0, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	//vec4 c1 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c2 = texture(tex0, vec2(t.x, t.y-loc0.y));
	//vec4 c3 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));
	vec4 c4 = texture(tex0, vec2(t.x-loc0.x, t.y));
	vec4 c5 = texture(tex0, vec2(t.x+loc0.x, t.y));
	//vec4 c6 = texture(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));
	vec4 c7 = texture(tex0, vec2(t.x, t.y+loc0.y));
	//vec4 c8 = texture(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));
	c = (c5-c4)*(c5-c4)+(c7-c2)*(c7-c2);
	c = clamp(c*2e6, 0.0, 1.0);
	FragColor = vec4(1.0-c.r, 1.0-c.g, 1.0-c.b, 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_DEPTH_TO_GRADIENT = R"GLSHDR(
//IMG_SHADER_CODE_DEPTH_TO_GRADIENT
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
//IMG_SHADER_CODE_DEPTH_TO_GRADIENT
uniform vec4 loc0; //(width, height, scale, 0.0)
uniform vec4 loc1; //(pert.x, pert.y, 0.0, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	float c;
	vec4 c2 = texture(tex0, vec2(t.x, t.y-loc0.y));
	vec4 c4 = texture(tex0, vec2(t.x-loc0.x, t.y));
	vec4 c5 = texture(tex0, vec2(t.x+loc0.x, t.y));
	vec4 c7 = texture(tex0, vec2(t.x, t.y+loc0.y));
	vec2 grad = vec2(c5.r-c4.r, c7.r-c2.r);
	vec2 pert = loc1.xy;
	float ang = max(dot(normalize(grad), pert)*2.0, -1.0);
	pert = grad*ang;
	grad += pert;
	c = grad.x*grad.x+grad.y*grad.y;
	c = clamp(c*loc0.z, 0.0, 1.0);
	FragColor = vec4(grad, c, 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRAD2SHADOW_INPUT = R"GLSHDR(
//IMG_SHADER_CODE_GRADIENT_TO_SHADOW
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
//IMG_SHADER_CODE_GRADIENT_TO_SHADOW
uniform vec4 loc0; //(1/width, 1/height, zoom, 0.0)
uniform vec4 loc1; //(darkness, 0.0, 0.0, 0.0)
uniform sampler2D tex0;
uniform sampler2D tex1;

)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRAD2SHADOW_INPUT_MESH = R"GLSHDR(
uniform sampler2D tex2;

)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRAD2SHADOW_HEAD = R"GLSHDR(
void main()
{
	vec2 uv = OutTexCoord.xy;
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRAD2SHADOW_HEAD_VOL = R"GLSHDR(
	vec4 c0 = texture(tex1, uv);
	if (c0.w == 0.0)
	{
		FragColor = vec4(1.0);
		return;
	}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRAD2SHADOW_HEAD_MESH = R"GLSHDR(
	vec4 c1 = texture(tex1, uv);
	vec4 c2 = texture(tex2, uv);

	if (c1.x == 1.0 && (c1.x < 1.0 || c2.w == 0.0)) {
		FragColor = vec4(1.0);
		return;
	}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRAD2SHADOW_BODY = R"GLSHDR(

	vec2 texel = loc0.xy;
	float zoom = clamp(loc0.z, 0.4, 3.0);
	float darkness = loc1.x;

	float c = 0.0;
	vec2 delta, sample_uv;
	vec4 grad_sample;
	float ang, dist, dense;

	const int mip_levels = 6;
	const int base_radius = 10;

	for (int level = 0; level < mip_levels; ++level) {
		float scale = pow(2.0, float(level));
		float lod = float(level);
		vec2 scaled_texel = texel * scale;

		int radius = int(float(base_radius) / (float(level) / 2.0 + 1.0));

		for (int i = -radius; i <= radius; ++i)
		for (int j = -radius; j <= radius; ++j)
		{
			delta = vec2(float(i), float(j)) * scaled_texel;
			sample_uv = uv + delta;
			grad_sample = textureLod(tex0, sample_uv, lod);

			if (grad_sample.z < 0.01) continue;

			ang = dot(normalize(delta), normalize(grad_sample.xy));
			if (ang > 0.9) continue;

			dist = pow(i*i + j*j + 1.0, 0.5); // gentler decay
			dist = dist == 0.0 ? 0.0 : 1.0 / dist * zoom;
			dense = clamp(0.02 + (3.0 - zoom) * 0.02, 0.02, 0.08);

			c += dense * (ang < -0.3 ? 1.0 : max(-ang + 0.7, 0.0)) * grad_sample.z * dist;
		}
	}

	c = clamp(1.0 - clamp(c, 0.0, 1.0) * darkness, 0.01, 1.0);
	FragColor = vec4(vec3(c), 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND = R"GLSHDR(
//IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	float alpha = clamp(c.a, 0.0, 1.0);
	alpha = clamp(2.0*alpha - alpha*alpha, 0.0, 1.0);
	//alpha = alpha>0.0?1.0:0.0;
	FragColor = vec4(c.rgb,alpha);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND_HDR = R"GLSHDR(
//IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND_HDR
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND_HDR
uniform vec4 loc0; //(r_gamma, g_gamma, b_gamma, 1.0)
uniform vec4 loc1; //(r_brightness, g_brightness, b_brightness, 1.0)
uniform vec4 loc2; //(r_hdr, g_hdr, b_hdr, 0.0)
uniform sampler2D tex0;
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec4 c = texture(tex0, t.xy);
	float alpha = clamp(c.a, 0.0, 1.0);
	alpha = clamp(2.0*alpha - alpha*alpha, 0.0, 1.0);
	vec4 b = loc1;
	b.x = b.x>1.0?1.0/(2.0-b.x):loc1.x;
	b.y = b.y>1.0?1.0/(2.0-b.y):loc1.y;
	b.z = b.z>1.0?1.0/(2.0-b.z):loc1.z;
	c = pow(c, loc0)*b;
	vec4 unit = vec4(1.0);
	FragColor = c*(unit-loc2)+loc2*(unit-unit/(c+unit));
	FragColor.a = alpha;
	//FragColor = vec4(0.4, 1.0, 0.0, 1.0);
}
)GLSHDR";

inline constexpr const char* PAINT_SHADER_CODE = R"GLSHDR(
//PAINT_SHADER_CODE
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
// PAINT_SHADER_CODE
uniform vec4 loc0; //(mouse_x, mouse_y, radius1, radius2)
uniform vec4 loc1; //(width, height, 0, 0)
	
void main()
{
	vec4 t = vec4(OutTexCoord, 1.0);
	vec2 center = vec2(loc0.x, loc0.y);
	vec2 pos = vec2(t.x*loc1.x, t.y*loc1.y);
	float d = length(pos - center);
	FragColor = vec4(0.0);
	if (d<loc0.w) FragColor = vec4(0.5);
	if (d<loc0.z) FragColor = vec4(1.0, 1.0, 1.0, 0.5);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_GRADIENT_BACKGROUND = R"GLSHDR(
//IMG_SHADER_CODE_GRADIENT_BACKGROUND
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
uniform vec4 loc0; //bg color
uniform vec4 loc1; //color1
uniform vec4 loc2; //color2
uniform vec4 loc3; //(v0, v1, v2, v3)
uniform vec4 loc4;//cam_right, r
uniform vec4 loc5;//cam_up, aspect
uniform vec4 loc6;//cam_forward, horizon
	
void main()
{
	vec2 ndc = OutTexCoord.xy;
	ndc = ndc * 2.0 - vec2(1.0);
	ndc.x *= loc5.w;
	ndc.y += loc4.w * loc6.w * 0.2;
	vec3 origin = ndc.x * loc4.xyz + ndc.y * loc5.xyz;
	vec3 dir = -normalize(loc6.xyz);
	vec3 hit = origin + loc4.w * dir;
	float pitch = degrees(atan(hit.y, length(vec2(hit.x, hit.z))));
	vec4 color;
	if (pitch < loc3.y)
		color = mix(loc2, loc0, (loc3.y - pitch) / (loc3.y - loc3.x));
	else if (pitch < loc3.z)
		color = mix(loc1, loc2, (loc3.z - pitch) / (loc3.z - loc3.y));
	else
		color = mix(loc1, loc0, (pitch - loc3.z) / (loc3.w - loc3.z));
	FragColor = vec4(color.rgb, 1.0);
}
)GLSHDR";

inline constexpr const char* IMG_SHADER_CODE_FILTER_LANCZOS_BICUBIC = R"GLSHDR(
//IMG_SHADER_CODE_FILTER_LANCZOS_BICUBIC
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 FragColor;
	
uniform sampler2D tex0;
uniform vec4 loc0; //(zoom/w, zoom/h, zoom, 0)
const int a = 5; //Lanczos window size
const int ns = 8; //bicubic jitter samples
	
float sinc(float x)
{
	if (abs(x) < 1e-5)
		return 1.0;
	x *= 3.14159265;
	return sin(x) / x;
}
	
float lanczos(float x)
{
	if (abs(x) >= float(a)) return 0.0;
	return sinc(x) * sinc(x / float(a));
}
	
vec4 lanczosFilter(vec2 uv)
{
	vec4 color = vec4(0.0);
	float totalWeight = 0.0;
for (int i = -a + 1; i <= a; ++i)
{
	for (int j = -a + 1; j <= a; ++j)
	{
		vec2 offset = vec2(i, j);
		float weight = lanczos(offset.x * loc0.z) * lanczos(offset.y * loc0.z);
		offset *= loc0.xy;
		color += texture(tex0, uv + offset) * weight;
		totalWeight += weight;
	}
}
	
	return color / totalWeight;
}
	
float catmullRom(float x)
{
	x = abs(x);
	if (x <= 1.0)
		return 1.5 * x * x * x - 2.5 * x * x + 1.0;
	else if (x < 2.0)
		return -0.5 * x * x * x + 2.5 * x * x - 4.0 * x + 2.0;
	return 0.0;
}
	
vec2 jitterOffsets[ns] = vec2[ns](
	vec2( 0.125,  0.375),
	vec2(-0.375,  0.125),
	vec2( 0.375, -0.125),
	vec2(-0.125, -0.375),
	vec2( 0.25,   0.25),
	vec2(-0.25,   0.25),
	vec2( 0.25,  -0.25),
	vec2(-0.25,  -0.25)
);
vec4 bicubicFilter(vec2 uv)
{
	vec2 texSize = loc0.xy;
	vec4 color = vec4(0.0);
	float totalWeight = 0.0;
	
	for (int s = 0; s < ns; ++s)
	{
		vec2 jitteredUV = uv + jitterOffsets[s] * texSize;
	
		vec2 pixelCoord = jitteredUV / texSize;
		vec2 base = floor(pixelCoord);
		vec2 f = fract(pixelCoord);
	
		vec4 sampleColor = vec4(0.0);
		float sampleWeight = 0.0;
	
		for (int j = -1; j <= 2; ++j)
		{
			for (int i = -1; i <= 2; ++i)
			{
				vec2 offset = vec2(i, j);
				float w = catmullRom(offset.x - f.x) * catmullRom(offset.y - f.y);
				vec2 sampleUV = (base + offset) * texSize;
				sampleColor += texture(tex0, sampleUV) * w;
				sampleWeight += w;
			}
		}
	
		color += sampleColor / sampleWeight;
		totalWeight += 1.0;
	}
	
	color /= totalWeight;
	vec4 old_color = texture(tex0, uv);
	float blend = smoothstep(0.0, 1.0, max(max(old_color.r, old_color.g), old_color.b));
	return mix(color, max(old_color, color), blend);
}
	
void main()
{
	float blend = smoothstep(0.6, 1.6, loc0.z);
	vec4 lanczosColor = lanczosFilter(OutTexCoord.xy);
	vec4 bicubicColor = bicubicFilter(OutTexCoord.xy);
	FragColor = mix(lanczosColor, bicubicColor, blend);
	FragColor = max(FragColor, 0.0);
}
)GLSHDR";

inline constexpr const char* IMG_SHDR_CODE_DRAW_THICK_LINES = R"GLSHDR(
uniform vec4 loc0; //(vp0, vp1, thickness, 0.0)
layout(lines) in;
layout(triangle_strip, max_vertices = 4) out;
in vec3 OutColor[];
out vec3 OutColor2;
vec2 toScreenSpace(vec4 vertex)
{
	return vec2(vertex.xy / vertex.w) * loc0.xy;
}
float toZValue(vec4 vertex)
{
	return (vertex.z / vertex.w);
}
void main() {
	OutColor2 = OutColor[0];
	vec4 pps[2];
	pps[0] = gl_in[0].gl_Position;
	pps[1] = gl_in[1].gl_Position;
	vec2 sps[2];
	sps[0] = toScreenSpace(pps[0]);
	sps[1] = toScreenSpace(pps[1]);
	float zs[2];
	zs[0] = toZValue(pps[0]);
	zs[1] = toZValue(pps[1]);
	vec2 v0 = normalize(sps[1] - sps[0]);
	vec2 n0 = vec2(-v0.y, v0.x);
	vec2 tps[4];
	tps[0] = sps[0] + n0 * loc0.z / 2.0;
	tps[1] = sps[0] - n0 * loc0.z / 2.0;
	tps[2] = sps[1] + n0 * loc0.z / 2.0;
	tps[3] = sps[1] - n0 * loc0.z / 2.0;
	gl_Position = vec4(tps[0] / loc0.xy, zs[0], 1.0);
	EmitVertex();
	gl_Position = vec4(tps[1] / loc0.xy, zs[0], 1.0);
	EmitVertex();
	gl_Position = vec4(tps[2] / loc0.xy, zs[1], 1.0);
	EmitVertex();
	gl_Position = vec4(tps[3] / loc0.xy, zs[1], 1.0);
	EmitVertex();
	EndPrimitive();
}
)GLSHDR";

inline constexpr const char* IMG_FRG_CODE_DRAW_THICKLINES = R"GLSHDR(
//IMG_FRG_CODE_DRAW_GEOMETRY_COLOR3
in vec3 OutColor2;
out vec4 FragColor;
	
void main()
{
	FragColor = vec4(OutColor2, 1.0);
}
)GLSHDR";

#endif//ImgShaderCode_h