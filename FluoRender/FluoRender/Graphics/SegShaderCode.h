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

#ifndef SEG_SHADER_CODE_H
#define SEG_SHADER_CODE_H

inline constexpr const char* SEG_OUTPUTS = R"GLSHDR(
//SEG_OUTPUTS
out vec4 FragColor;
)GLSHDR";

inline constexpr const char* SEG_VERTEX_CODE = R"GLSHDR(
//SEG_VERTEX_CODE
layout(location = 0) in vec3 InVertex;
layout(location = 1) in vec3 InTexture;
out vec3 OutVertex;
out vec3 OutTexture;
	
void main()
{
	gl_Position = vec4(InVertex,1.);
	OutTexture = InTexture;
	OutVertex  = InVertex;
}
)GLSHDR";

inline constexpr const char* SEG_UNIFORMS_WMAP_2D = R"GLSHDR(
//SEG_UNIFORMS_WMAP_2D
uniform sampler2D tex4;//2d weight map (after tone mapping)
uniform sampler2D tex5;//2d weight map (before tone mapping)
)GLSHDR";

inline constexpr const char* SEG_UNIFORMS_MASK_2D = R"GLSHDR(
//SEG_UNIFORMS_MASK_2D
uniform sampler2D tex6;//2d mask
)GLSHDR";

inline constexpr const char* SEG_UNIFORMS_MATRICES = R"GLSHDR(
//SEG_UNIFORMS_MATRICES
uniform mat4 matrix0;//modelview matrix
uniform mat4 matrix1;//projection matrix
)GLSHDR";

inline constexpr const char* SEG_UNIFORM_MATRICES_INVERSE = R"GLSHDR(
//SEG_UNIFORM_MATRICES_INVERSE
uniform mat4 matrix3;//modelview matrix inverse
uniform mat4 matrix4;//projection matrix inverse
)GLSHDR";

inline constexpr const char* SEG_UNIFORMS_LABEL_OUTPUT = R"GLSHDR(
//SEG_UNIFORMS_LABEL_OUTPUT
out uint FragUint;
)GLSHDR";

inline constexpr const char* SEG_UNIFORMS_PARAMS = R"GLSHDR(
//SEG_UNIFORMS_PARAMS
uniform vec4 loc20;//(ini_thresh, gm_falloff, scl_falloff, scl_translate)
uniform vec4 loc21;//(weight_2d, post_bins, zoom, 0)
)GLSHDR";

inline constexpr const char* SEG_TAIL = R"GLSHDR(
//SEG_TAIL
}
)GLSHDR";

inline constexpr const char* SEG_BODY_WEIGHT = R"GLSHDR(
	//SEG_BODY_WEIGHT
	vec4 cw2d;
	vec4 weight1 = texture(tex4, s.xy);
	vec4 weight2 = texture(tex5, s.xy);
	cw2d.x = weight2.x==0.0?0.0:weight1.x/weight2.x;
	cw2d.y = weight2.y==0.0?0.0:weight1.y/weight2.y;
	cw2d.z = weight2.z==0.0?0.0:weight1.z/weight2.z;
	float weight2d = max(cw2d.x, max(cw2d.y, cw2d.z));
)GLSHDR";

inline constexpr const char* SEG_BODY_BLEND_WEIGHT = R"GLSHDR(
	//SEG_BODY_BLEND_WEIGHT
	c = loc21.x>1.0?c*loc21.x*weight2d:(1.0-loc21.x)*c+loc21.x*c*weight2d;
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_CLEAR = R"GLSHDR(
	//SEG_BODY_INIT_CLEAR
	FragColor = vec4(0.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_2D_COORD = R"GLSHDR(
	//SEG_BODY_INIT_2D_COORD
	vec4 s = matrix1 * matrix0 * matrix2 * vec4(texCoord, 1.0);
	s = s / s.w;
	s.xy = s.xy / 2.0 + 0.5;
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_CULL = R"GLSHDR(
	//SEG_BODY_INIT_CULL
	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||
		any(greaterThan(s.xy, vec2(1.0, 1.0))))
		discard;
	float cmask2d = texture(tex6, s.xy).x;
	if (cmask2d < 0.95)
		discard;
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_CULL_ERASE = R"GLSHDR(
	//SEG_BODY_INIT_CULL_ERASE
	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||
		any(greaterThan(s.xy, vec2(1.0, 1.0))))
		discard;
	float cmask2d = texture(tex6, s.xy).x;
	if (cmask2d < 0.45)
		discard;
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_CULL_POINT = R"GLSHDR(
	//SEG_BODY_INIT_CULL_POINT
	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||
		any(greaterThan(s.xy, vec2(1.0, 1.0))))
		discard;
	float dist = length(s.xy*loc6.zw - loc6.xy);
	if (dist > loc21.w * 0.6)
		discard;
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_HEAD = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_HEAD
	c = c*l.w;
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_APPEND = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_APPEND
	FragColor = vec4(c.x>0.0?(c.x>loc20.x?1.0:0.0):0.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_ERASE = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_ERASE
	FragColor = vec4(0.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_DIFFUSE = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_DIFFUSE
	FragColor = texture(tex2, texCoord);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_FLOOD = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_FLOOD
	FragColor = vec4(c.x>0.0?(c.x>loc20.x?1.0:0.0):0.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_ALL = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_ALL
	FragColor = vec4(1.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_HR_ORTHO = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_HR_ORTHO
	if (c.x <= loc20.x)
		discard;
	vec4 cv = matrix3 * vec4(0.0, 0.0, 1.0, 0.0);
	vec3 step = cv.xyz;
	step = normalize(step);
	step = step * length(step * loc4.xyz);
	vec3 ray = texCoord;
	vec4 cray;
	bool flag = false;
	float th = loc20.x<0.01?0.01:loc20.x;
	while (true)
	{
		ray += step;
		if (any(greaterThan(ray, vec3(1.0))) ||
				any(lessThan(ray, vec3(0.0))))
			break;
		if (vol_clip_func(vec4(ray, 1.0)))
			break;
		v.x = texture(tex0, ray).x;
		v.y = length(vol_grad_func(vec4(ray, 1.0), loc4).xyz);
		cray = vol_trans_sin_color_l(v);
		if (cray.x > th && flag)
			discard;
		if (cray.x <= th)
			flag = true;
	}
	FragColor = vec4(1.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_INIT_BLEND_HR_PERSP = R"GLSHDR(
	//SEG_BODY_INIT_BLEND_HR_PERSP
	if (c.x <= loc20.x)
		discard;
	vec4 cv = matrix3 * vec4(0.0, 0.0, 0.0, 1.0);
	cv = cv / cv.w;
	vec3 step = cv.xyz - texCoord;
	step = normalize(step);
	step = step * length(step * loc4.xyz);
	vec3 ray = texCoord;
	vec4 cray;
	bool flag = false;
	float th = loc20.x<0.01?0.01:loc20.x;
	while (true)
	{
		ray += step;
		if (any(greaterThan(ray, vec3(1.0))) ||
				any(lessThan(ray, vec3(0.0))))
			break;
		if (vol_clip_func(vec4(ray, 1.0)))
			break;
		v.x = texture(tex0, ray).x;
		v.y = length(vol_grad_func(vec4(ray, 1.0), loc4).xyz);
		cray = vol_trans_sin_color_l(v);
		if (cray.x > th && flag)
		{
			FragColor = vec4(0.0);
			return;
		}
		if (cray.x <= th)
			flag = true;
	}
	FragColor = vec4(1.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_2D_COORD = R"GLSHDR(
	//SEG_BODY_DB_GROW_2D_COORD
	vec4 s = matrix1 * matrix0 * matrix2 * vec4(texCoord, 1.0);
	s = s / s.w;
	s.xy = s.xy / 2.0 + 0.5;
	vec4 cc = texture(tex2, texCoord);
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_CULL = R"GLSHDR(
	//SEG_BODY_DB_GROW_CULL
	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||
		any(greaterThan(s.xy, vec2(1.0, 1.0))))
		discard;
	float cmask2d = texture(tex6, s.xy).x;
	if (cmask2d < 0.45 /*|| cmask2d > 0.55*/)
		discard;
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_STOP_FUNC = R"GLSHDR(
	//SEG_BODY_DB_GROW_STOP_FUNC
	if (c.x == 0.0)
		discard;
	v.x = c.x>1.0?1.0:c.x;
	float stop = 
		(loc20.y>=1.0?1.0:(v.y>sqrt(loc20.y)*2.12?0.0:exp(-v.y*v.y/loc20.y)))*
		(v.x>loc20.w?1.0:(loc20.z>0.0?(v.x<loc20.w-sqrt(loc20.z)*2.12?0.0:exp(-(v.x-loc20.w)*(v.x-loc20.w)/loc20.z)):0.0));
	if (stop == 0.0)
		discard;
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_BLEND_APPEND_HEAD = R"GLSHDR(
	//SEG_BODY_DB_GROW_BLEND_APPEND
	FragColor = (1.0-stop) * cc;
	vec3 nb;
	vec3 max_nb = texCoord;
	float m;
	float mx;
	for (int i=-1; i<2; i++)
	for (int j=-1; j<2; j++)
	for (int k=-1; k<2; k++)
	{
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_BLEND_APPEND_DIR = R"GLSHDR(
		//SEG_BODY_DB_GROW_BLEND_APPEND_DIR
		vec3 ndir = vec3(i, j, k);
		ndir = normalize(ndir);
		float ang = dot(ndir, loc9.xyz);
		if (ang < 0.5) continue;
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_BLEND_APPEND_BODY = R"GLSHDR(
		//SEG_BODY_DB_GROW_BLEND_APPEND_BODY
		nb = vec3(texCoord.s+float(i)*loc4.x, texCoord.t+float(j)*loc4.y, texCoord.p+float(k)*loc4.z);
		m = texture(tex2, nb).x;
		if (m > cc.x)
		{
			cc = vec4(m);
			max_nb = nb;
		}
	}
	if (loc20.y>0.0)
	{
		m = texture(tex0, max_nb).x + loc20.y;
		mx = texture(tex0, texCoord).x;
		if (m < mx || m - mx > 2.0*loc20.y)
			discard;
	}
	FragColor += cc*stop;
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_BLEND_ERASE0 = R"GLSHDR(
	//SEG_BODY_DB_GROW_BLEND_ERASE
	for (int i=-1; i<2; i++)
	for (int j=-1; j<2; j++)
	for (int k=-1; k<2; k++)
	{
		vec3 nb = vec3(texCoord.s+float(i)*loc4.x, texCoord.t+float(j)*loc4.y, texCoord.p+float(k)*loc4.z);
		cc = vec4(min(cc.x, texture(tex2, nb).x));
	}
	FragColor = cc*clamp(1.0-stop, 0.0, 1.0);
)GLSHDR";

inline constexpr const char* SEG_BODY_DB_GROW_BLEND_ERASE = R"GLSHDR(
	//SEG_BODY_DB_GROW_BLEND_ERASE
	FragColor = vec4(0.0);
)GLSHDR";

#endif//SEG_SHADER_CODE_H