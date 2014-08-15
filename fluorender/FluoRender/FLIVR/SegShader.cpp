//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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

#include <string>
#include <sstream>
#include <iostream>
#include <FLIVR/SegShader.h>
#include <FLIVR/ShaderProgram.h>
#include <FLIVR/VolShaderCode.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{
#define SEG_UNIFORMS_LABEL_INT \
	"//SEG_UNIFORMS_LABEL_INT\n" \
	"uniform vec4 loc9;//(nx, ny, nz, 0)\n" \
	"\n"

#define SEG_UNIFORMS_LABEL_MIF \
	"//SEG_UNIFORMS_LABEL_MIF\n" \
	"uniform vec4 loc0;//(1/nx, 1/ny, 1/nz, 0)\n" \
	"\n"

#define SEG_UNIFORMS_WMAP_2D \
	"//SEG_UNIFORMS_WMAP_2D\n" \
	"uniform sampler2D tex4;//2d weight map (after tone mapping)\n" \
	"uniform sampler2D tex5;//2d weight map (before tone mapping)\n" \
	"\n"

#define SEG_UNIFORMS_MASK_2D \
	"//SEG_UNIFORMS_MASK_2D\n" \
	"uniform sampler2D tex6;//2d mask\n" \
	"\n"

#define SEG_UNIFORMS_MATRICES \
	"//SEG_UNIFORMS_MATRICES\n" \
	"uniform mat4 matrix0;//modelview matrix\n" \
	"uniform mat4 matrix1;//projection matrix\n" \
	"\n"

#define SEG_UNIFORM_MATRICES_INVERSE \
	"//SEG_UNIFORM_MATRICES_INVERSE\n" \
	"uniform mat4 matrix3;//modelview matrix inverse\n" \
	"uniform mat4 matrix4;//projection matrix inverse\n" \
	"\n"

#define SEG_UNIFORMS_LABEL_OUTPUT \
	"//SEG_UNIFORMS_LABEL_OUTPUT\n" \
	"out uint PixelColor;\n"\
	"\n"

#define SEG_UNIFORMS_PARAMS \
	"//SEG_UNIFORMS_PARAMS\n" \
	"uniform vec4 loc7;//(ini_thresh, gm_falloff, scl_falloff, scl_translate)\n" \
	"uniform vec4 loc8;//(weight_2d, post_bins, 0, 0)\n" \
	"\n"

#define SEG_UNIFORMS_PARAM_MEASURE \
	"//SEG_UNIFORMS_PARAM_MEASURE\n" \
	"uniform vec4 loc9;//(value_var_foff, angle_var_foff, 0, 0)\n" \
	"\n"
#define SEG_TAIL \
	"//SEG_TAIL\n" \
	"}\n"

#define SEG_BODY_WEIGHT \
	"	//SEG_BODY_WEIGHT\n" \
	"	vec4 cw2d;\n" \
	"	vec4 weight1 = texture2D(tex4, s.xy);\n" \
	"	vec4 weight2 = texture2D(tex5, s.xy);\n" \
	"	cw2d.x = weight2.x==0.0?0.0:weight1.x/weight2.x;\n" \
	"	cw2d.y = weight2.y==0.0?0.0:weight1.y/weight2.y;\n" \
	"	cw2d.z = weight2.z==0.0?0.0:weight1.z/weight2.z;\n" \
	"	float weight2d = max(cw2d.x, max(cw2d.y, cw2d.z));\n" \
	"\n"

#define SEG_BODY_BLEND_WEIGHT \
	"	//SEG_BODY_BLEND_WEIGHT\n" \
	"	c = loc8.x>1.0?c*loc8.x*weight2d:(1.0-loc8.x)*c+loc8.x*c*weight2d;\n" \
	"\n"

#define SEG_BODY_INIT_CLEAR \
	"	//SEG_BODY_INIT_CLEAR\n" \
	"	gl_FragColor = vec4(0.0);\n"\
	"\n"

#define SEG_BODY_INIT_2D_COORD \
	"	//SEG_BODY_INIT_2D_COORD\n" \
	"	vec4 s = matrix1 * matrix0 * matrix2 * t;\n"\
	"	s = s / s.w;\n"\
	"	s.xy = s.xy / 2.0 + 0.5;\n"\
	"\n"

#define SEG_BODY_INIT_CULL \
	"	//SEG_BODY_INIT_CULL\n" \
	"	float cmask2d = texture2D(tex6, s.xy).x;\n"\
	"	if (cmask2d < 0.95)\n"\
	"	{\n"\
	"		discard;//gl_FragColor = vec4(0.0);\n"\
	"		return;\n"\
	"	}\n"\
	"\n"

#define SEG_BODY_INIT_CULL_ERASE \
	"	//SEG_BODY_INIT_CULL_ERASE\n" \
	"	float cmask2d = texture2D(tex6, s.xy).x;\n"\
	"	if (cmask2d < 0.45)\n"\
	"	{\n"\
	"		discard;//gl_FragColor = vec4(0.0);\n"\
	"		return;\n"\
	"	}\n"\
	"\n"

#define SEG_BODY_INIT_BLEND_HEAD \
	"	//SEG_BODY_INIT_BLEND_HEAD\n" \
	"	c = c*l.w;\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_APPEND \
	"	//SEG_BODY_INIT_BLEND_APPEND\n" \
	"	gl_FragColor = vec4(c.x>0.0?(c.x>loc7.x?1.0:0.0):0.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_ERASE \
	"	//SEG_BODY_INIT_BLEND_ERASE\n" \
	"	gl_FragColor = vec4(0.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_DIFFUSE \
	"	//SEG_BODY_INIT_BLEND_DIFFUSE\n" \
	"	gl_FragColor = texture3D(tex2, t.stp);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_FLOOD \
	"	//SEG_BODY_INIT_BLEND_FLOOD\n" \
	"	gl_FragColor = vec4(c.x>0.0?(c.x>loc7.x?1.0:0.0):0.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_ALL \
	"	//SEG_BODY_INIT_BLEND_ALL\n" \
	"	gl_FragColor = vec4(1.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_HR_ORTHO \
	"	//SEG_BODY_INIT_BLEND_HR_ORTHO\n" \
	"	if (c.x <= loc7.x)\n" \
	"		discard;\n" \
	"	vec4 cv = matrix3 * vec4(0.0, 0.0, 1.0, 0.0);\n" \
	"	vec3 step = cv.xyz;\n" \
	"	step = normalize(step);\n" \
	"	step = step * length(step * loc4.xyz);\n" \
	"	vec3 ray = t.xyz;\n" \
	"	vec4 cray;\n" \
	"	bool flag = false;\n" \
	"	while (true)\n" \
	"	{\n" \
	"		ray += step;\n" \
	"		if (any(greaterThan(ray, vec3(1.0))) ||\n" \
	"				any(lessThan(ray, vec3(0.0))))\n" \
	"			break;\n" \
	"		if (vol_clip_func(vec4(ray, 1.0)))\n" \
	"			break;\n" \
	"		v.x = texture3D(tex0, ray).x;\n" \
	"		v.y = length(vol_grad_func(vec4(ray, 1.0), loc4).xyz);\n" \
	"		cray = vol_trans_sin_color_l(v);\n" \
	"		if (cray.x > loc7.x && flag)\n" \
	"		{\n" \
	"			gl_FragColor = vec4(0.0);\n" \
	"			return;\n" \
	"		}\n" \
	"		if (cray.x <= loc7.x)\n" \
	"			flag = true;\n" \
	"	}\n" \
	"	gl_FragColor = vec4(1.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_HR_PERSP \
	"	//SEG_BODY_INIT_BLEND_HR_PERSP\n" \
	"	if (c.x <= loc7.x)\n" \
	"		discard;\n" \
	"	vec4 cv = matrix3 * vec4(0.0, 0.0, 0.0, 1.0);\n" \
	"	cv = cv / cv.w;\n" \
	"	vec3 step = cv.xyz - t.xyz;\n" \
	"	step = normalize(step);\n" \
	"	step = step * length(step * loc4.xyz);\n" \
	"	vec3 ray = t.xyz;\n" \
	"	vec4 cray;\n" \
	"	bool flag = false;\n" \
	"	while (true)\n" \
	"	{\n" \
	"		ray += step;\n" \
	"		if (any(greaterThan(ray, vec3(1.0))) ||\n" \
	"				any(lessThan(ray, vec3(0.0))))\n" \
	"			break;\n" \
	"		if (vol_clip_func(vec4(ray, 1.0)))\n" \
	"			break;\n" \
	"		v.x = texture3D(tex0, ray).x;\n" \
	"		v.y = length(vol_grad_func(vec4(ray, 1.0), loc4).xyz);\n" \
	"		cray = vol_trans_sin_color_l(v);\n" \
	"		if (cray.x > loc7.x && flag)\n" \
	"		{\n" \
	"			gl_FragColor = vec4(0.0);\n" \
	"			return;\n" \
	"		}\n" \
	"		if (cray.x <= loc7.x)\n" \
	"			flag = true;\n" \
	"	}\n" \
	"	gl_FragColor = vec4(1.0);\n" \
	"\n"

#define SEG_BODY_INIT_POSTER \
	"	//SEG_BODY_INIT_POSTER\n" \
	"	gl_FragColor = vec4(ceil(c.x*loc8.y)/loc8.y);\n" \
	"\n"

#define SEG_BODY_DB_GROW_2D_COORD \
	"	//SEG_BODY_DB_GROW_2D_COORD\n" \
	"	vec4 s = matrix1 * matrix0 * matrix2 * t;\n"\
	"	s = s / s.w;\n"\
	"	s.xy = s.xy / 2.0 + 0.5;\n"\
	"	vec4 cc = texture3D(tex2, t.stp);\n"\
	"\n"

#define SEG_BODY_DB_GROW_CULL \
	"	//SEG_BODY_DB_GROW_CULL\n" \
	"	float cmask2d = texture2D(tex6, s.xy).x;\n"\
	"	if (cmask2d < 0.45 /*|| cmask2d > 0.55*/)\n"\
	"		discard;\n"\
	"\n"

#define SEG_BODY_DB_GROW_STOP_FUNC \
	"	//SEG_BODY_DB_GROW_STOP_FUNC\n" \
	"	v.x = c.x>1.0?1.0:c.x;\n" \
	"	float stop = \n" \
	"		(loc7.y>0.0?(v.y>sqrt(loc7.y)*2.12?0.0:exp(-v.y*v.y/loc7.y)):1.0)*\n" \
	"		(v.x>loc7.w?1.0:(loc7.z>0.0?(v.x<loc7.w-sqrt(loc7.z)*2.12?0.0:exp(-(v.x-loc7.w)*(v.x-loc7.w)/loc7.z)):0.0));\n" \
	"	if (stop == 0.0)\n" \
	"		discard;\n" \
	"\n"\

#define SEG_BODY_DB_GROW_BLEND_APPEND \
	"	//SEG_BODY_DB_GROW_BLEND_APPEND\n" \
	"	gl_FragColor = (1.0-stop) * cc;\n" \
	"	vec3 nb;\n" \
	"	vec3 max_nb = t.stp;\n" \
	"	float m;\n" \
	"	for (int i=-1; i<2; i++)\n"\
	"	for (int j=-1; j<2; j++)\n"\
	"	for (int k=-1; k<2; k++)\n"\
	"	{\n"\
	"		nb = vec3(t.s+float(i)*loc4.x, t.t+float(j)*loc4.y, t.p+float(k)*loc4.z);\n"\
	"		m = texture3D(tex2, nb).x;\n" \
	"		if (m > cc.x)\n" \
	"		{\n" \
	"			cc = vec4(m);\n" \
	"			max_nb = nb;\n" \
	"		}\n" \
	"	}\n"\
	"	if (loc7.y>0.0)\n" \
	"	{\n" \
	"		m = texture3D(tex0, max_nb).x + loc7.y;\n" \
	"		if (m < texture3D(tex0, t.stp).x)\n" \
	"			discard;\n" \
	"	}\n" \
	"	gl_FragColor += cc*stop;\n"\
	"\n"

#define SEG_BODY_DB_GROW_BLEND_ERASE0 \
	"	//SEG_BODY_DB_GROW_BLEND_ERASE\n" \
	"	for (int i=-1; i<2; i++)\n"\
	"	for (int j=-1; j<2; j++)\n"\
	"	for (int k=-1; k<2; k++)\n"\
	"	{\n"\
	"		vec3 nb = vec3(t.s+float(i)*loc4.x, t.t+float(j)*loc4.y, t.p+float(k)*loc4.z);\n"\
	"		cc = vec4(min(cc.x, texture3D(tex2, nb).x));\n"\
	"	}\n"\
	"	gl_FragColor = cc*clamp(1.0-stop, 0.0, 1.0);\n"\
	"\n"

#define SEG_BODY_DB_GROW_BLEND_ERASE \
	"	//SEG_BODY_DB_GROW_BLEND_ERASE\n" \
	"	gl_FragColor = vec4(0.0);\n"\
	"\n"

#define SEG_BODY_LABEL_INITIALIZE \
	"	//SEG_BODY_LABEL_INITIALIZE\n" \
	"	vec4 mask = texture(tex2, t.stp)*c.x;\n" \
	"	if (mask.x == 0.0 || mask.x < loc7.x)\n" \
	"	{\n" \
	"		PixelColor = uint(0);\n" \
	"		return;\n" \
	"	}\n" \
	"\n"

#define SEG_BODY_LABEL_INIT_ORDER \
	"	//SEG_BODY_LABEL_INIT_ORDER\n" \
	"	vec3 int_pos = vec3(t.x*loc9.x, t.y*loc9.y, t.z*loc9.z);\n" \
	"	uint index = uint(int_pos.z*loc9.x*loc9.y+int_pos.y*loc9.x+int_pos.x+1);\n" \
	"	PixelColor = index;\n" \
	"\n"

#define SEG_BODY_LABEL_INIT_COPY \
	"	//SEG_BODY_LABEL_INIT_COPY\n" \
	"	discard;\n" \
	"\n"

#define SEG_BODY_LABEL_INITIALIZE_NOMASK \
	"	//SEG_BODY_LABEL_INITIALIZE_NOMASK\n" \
	"	if (c.x ==0.0 || c.x <= loc7.x)\n" \
	"	{\n" \
	"		PixelColor = uint(0);\n" \
	"		return;\n" \
	"	}\n" \
	"\n"

#define SEG_BODY_LABEL_INIT_POSTER \
	"	//SEG_BODY_LABEL_INIT_POSTER\n" \
	"	vec4 mask = texture(tex2, t.stp);\n" \
	"	if (mask.x < 0.001)\n" \
	"	{\n" \
	"		PixelColor = uint(0);\n" \
	"		return;\n" \
	"	}\n" \
	"\n"

#define VOL_MEASURE_GM_LOOKUP \
	"	//VOL_MEASURE_GM_LOOKUP\n" \
	"	w = vol_grad_func(gl_TexCoord[0], loc4);\n" \
	"	n.xyz = clamp(normalize(w.xyz), vec3(0.0), vec3(1.0));\n" \
	"	v.y = length(w.xyz);\n" \
	"	v.y = 0.5 * (loc2.x<0.0?(1.0+v.y*loc2.x):v.y*loc2.x);\n" \
	"\n"

#define SEG_BODY_LABEL_MAX_FILTER \
	"	//SEG_BODY_LABEL_MAX_FILTER\n" \
	"	uint int_val = texture(tex3, t.stp).x;\n" \
	"	if (int_val == uint(0))\n" \
	"		discard;\n" \
	"	vec3 nb;\n" \
	"	vec3 max_nb = t.stp;\n" \
	"	uint m;\n" \
	"	for (int i=-1; i<2; i++)\n" \
	"	for (int j=-1; j<2; j++)\n" \
	"	for (int k=-1; k<2; k++)\n" \
	"	{\n" \
	"		if (k==0 && (i!=0 || j!=0))\n" \
	"			continue;\n" \
	"		nb = vec3(t.s+float(i)*loc4.x, t.t+float(j)*loc4.y, t.p+float(k)*loc4.z);\n" \
	"		m = texture(tex3, nb).x;\n" \
	"		if (m > int_val)\n" \
	"		{\n" \
	"			int_val = m;\n" \
	"			max_nb = nb;\n" \
	"		}\n" \
	"	}\n" \
	"	if (texture(tex0, max_nb).x+loc7.y < texture(tex0, t.stp).x)\n" \
	"		discard;\n" \
	"	PixelColor = int_val;\n" \
	"\n"

#define SEG_BODY_LABEL_MIF_POSTER \
	"	//SEG_BODY_LABEL_MIF_POSTER\n" \
	"	uint int_val = texture(tex3, t.stp).x;\n" \
	"	if (int_val == uint(0))\n" \
	"	{\n" \
	"		PixelColor = uint(0);\n" \
	"		return;\n" \
	"	}\n" \
	"	float cur_val = texture(tex2, t.stp).x;\n" \
	"	float nb_val;\n" \
	"	for (int i=-1; i<2; i++)\n" \
	"	for (int j=-1; j<2; j++)\n" \
	"	for (int k=-1; k<2; k++)\n" \
	"	{\n" \
	"		vec3 nb = vec3(t.s+float(i)*loc0.x, t.t+float(j)*loc0.y, t.p+float(k)*loc0.z);\n" \
	"		nb_val = texture(tex2, nb).x;\n" \
	"		if (abs(nb_val-cur_val) < 0.001)\n" \
	"			int_val = max(int_val, texture(tex3, nb).x);\n" \
	"	}\n" \
	"	PixelColor = int_val;\n" \
	"\n"

#define FLT_BODY_NR \
	"	//FLT_BODY_NR\n" \
	"	float vc = texture3D(tex0, t.stp).x;\n" \
	"	float mc = texture3D(tex2, t.stp).x;\n" \
	"	if (mc < 0.001)\n" \
	"		discard;\n" \
	"	float nb_val;\n" \
	"	float max_val = -0.1;\n" \
	"	for (int i=-2; i<3; i++)\n" \
	"	for (int j=-2; j<3; j++)\n" \
	"	for (int k=-2; k<3; k++)\n" \
	"	{\n" \
	"		vec3 nb = vec3(t.s+float(i)*loc4.x, t.t+float(j)*loc4.y, t.p+float(k)*loc4.z);\n" \
	"		nb_val = texture3D(tex0, nb).x;\n" \
	"		max_val = (nb_val<vc && nb_val>max_val)?nb_val:max_val;\n" \
	"	}\n" \
	"	if (max_val > 0.0)\n" \
	"		gl_FragColor = vec4(max_val);\n" \
	"	else\n" \
	"		discard;\n" \
	"\n"

	SegShader::SegShader(int type, int paint_mode, int hr_mode,
		bool use_2d, bool shading, int peel, bool clip, bool hiqual) :
	type_(type),
	paint_mode_(paint_mode),
	hr_mode_(hr_mode),
	use_2d_(use_2d),
	shading_(shading),
	peel_(peel),
	clip_(clip),
	hiqual_(hiqual),
	program_(0)
	{}

	SegShader::~SegShader()
	{
		delete program_;
	}

	bool SegShader::create()
	{
		string s;
		if (emit(s)) return true;
		program_ = new FragmentProgram(s);
		return false;
	}

	bool SegShader::emit(string& s)
	{
		ostringstream z;

		//uniforms
		switch (type_)
		{
		case SEG_SHDR_INITIALIZE:
		case SEG_SHDR_DB_GROW:
			z << VOL_UNIFORMS_COMMON;
			z << VOL_UNIFORMS_SIN_COLOR;
			z << VOL_UNIFORMS_MASK;
			z << SEG_UNIFORMS_WMAP_2D;
			z << SEG_UNIFORMS_MASK_2D;
			z << SEG_UNIFORMS_MATRICES;
			z << VOL_UNIFORMS_MATRICES;
			z << SEG_UNIFORMS_PARAMS;
			break;
		case LBL_SHDR_INITIALIZE:
			z << VOL_VERSION_130;
			z << VOL_UNIFORMS_COMMON;
			z << VOL_UNIFORMS_SIN_COLOR;
			z << SEG_UNIFORMS_LABEL_INT;
			if (use_2d_)
				z << VOL_UNIFORMS_MASK;
			z << SEG_UNIFORMS_LABEL_OUTPUT;
			z << SEG_UNIFORMS_PARAMS;
			break;
		case LBL_SHDR_MIF:
			z << VOL_VERSION_130;

			z << VOL_UNIFORMS_COMMON;
			z << VOL_UNIFORMS_SIN_COLOR;
			z << VOL_UNIFORMS_LABEL;
			if (paint_mode_==1)
				z << VOL_UNIFORMS_MASK;
			z << SEG_UNIFORMS_LABEL_OUTPUT;
			z << SEG_UNIFORMS_PARAMS;
			z << SEG_UNIFORMS_PARAM_MEASURE;
			break;
		case FLT_SHDR_NR:
			z << VOL_UNIFORMS_COMMON;
			z << VOL_UNIFORMS_MASK;
			z << SEG_UNIFORMS_MATRICES;
			z << VOL_UNIFORMS_MATRICES;
			break;
		}

		//uniforms for clipping
		if (paint_mode_!=6 && clip_)
			z << VOL_UNIFORMS_CLIP;

		//functions
		if (type_==SEG_SHDR_INITIALIZE &&
			(paint_mode_==1 || paint_mode_==2) &&
			hr_mode_>0)
		{
			z << SEG_UNIFORM_MATRICES_INVERSE;
			z << VOL_GRAD_COMPUTE_FUNC;
			z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC;
		}
		else if (type_==LBL_SHDR_MIF)
		{
			z << VOL_GRAD_COMPUTE_FUNC;
		}

		if (paint_mode_!=6 && clip_)
			z << VOL_CLIP_FUNC;

		//the common head
		z << VOL_HEAD;

		//head for clipping planes
		if (paint_mode_!=6 && clip_)
			z << VOL_HEAD_CLIP_FUNC;

		if (paint_mode_ == 6)
		{
			z << SEG_BODY_INIT_CLEAR;
		}
		else
		{
			//bodies
			switch (type_)
			{
			case SEG_SHDR_INITIALIZE:
				z << SEG_BODY_INIT_2D_COORD;
				if (paint_mode_==1 ||
					paint_mode_==2 ||
					paint_mode_==4)
					z << SEG_BODY_INIT_CULL;
				else if (paint_mode_==3)
					z << SEG_BODY_INIT_CULL_ERASE;

				if (paint_mode_ != 3)
				{
					z << VOL_HEAD_LIT;
					z << VOL_DATA_VOLUME_LOOKUP;
					z << VOL_GRAD_COMPUTE_HI;
					z << VOL_COMPUTED_GM_LOOKUP;
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;

					if (use_2d_)
					{
						z << SEG_BODY_WEIGHT;
						z << SEG_BODY_BLEND_WEIGHT;
					}
				}

				if (paint_mode_==1 ||
					paint_mode_==2)
				{
					if (hr_mode_ == 0)
						z << SEG_BODY_INIT_BLEND_APPEND;
					else if (hr_mode_ == 1)//ortho
						z << SEG_BODY_INIT_BLEND_HR_ORTHO;
					else if (hr_mode_ == 2)//persp
						z << SEG_BODY_INIT_BLEND_HR_PERSP;
				}
				else if (paint_mode_==3)
					z << SEG_BODY_INIT_BLEND_ERASE;
				else if (paint_mode_==4)
					z << SEG_BODY_INIT_BLEND_DIFFUSE;
				else if (paint_mode_==5)
					z << SEG_BODY_INIT_BLEND_FLOOD;
				else if (paint_mode_==7)
					z << SEG_BODY_INIT_BLEND_ALL;
				else if (paint_mode_==11)
					z << SEG_BODY_INIT_POSTER;
				break;
			case SEG_SHDR_DB_GROW:
				z << SEG_BODY_DB_GROW_2D_COORD;
				if (paint_mode_!=5)
					z << SEG_BODY_DB_GROW_CULL;

				if (paint_mode_ != 3)
				{
					z << VOL_HEAD_LIT;
					z << VOL_DATA_VOLUME_LOOKUP;
					z << VOL_GRAD_COMPUTE_HI;
					z << VOL_COMPUTED_GM_LOOKUP;
					z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;

					if (use_2d_)
					{
						z << SEG_BODY_WEIGHT;
						z << SEG_BODY_BLEND_WEIGHT;
					}

					z << SEG_BODY_DB_GROW_STOP_FUNC;
				}

				if (paint_mode_==1 ||
					paint_mode_==2 ||
					paint_mode_==4 ||
					paint_mode_==5)
					z << SEG_BODY_DB_GROW_BLEND_APPEND;
				else if (paint_mode_==3)
					z << SEG_BODY_DB_GROW_BLEND_ERASE;
				break;
			case LBL_SHDR_INITIALIZE:
				z << VOL_HEAD_LIT;
				z << VOL_DATA_VOLUME_LOOKUP_130;
				z << VOL_COMPUTED_GM_INVALIDATE;
				z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;
				if (paint_mode_==0)
				{
					if (use_2d_)
						z << SEG_BODY_LABEL_INITIALIZE;
					else
						z << SEG_BODY_LABEL_INITIALIZE_NOMASK;
					z << SEG_BODY_LABEL_INIT_ORDER;
				}
				else if (paint_mode_==1)
				{
					z << SEG_BODY_LABEL_INIT_POSTER;
					z << SEG_BODY_LABEL_INIT_ORDER;
				}
				else if (paint_mode_==2)
				{
					if (use_2d_)
						z << SEG_BODY_LABEL_INITIALIZE;
					else
						z << SEG_BODY_LABEL_INITIALIZE_NOMASK;
					z << SEG_BODY_LABEL_INIT_COPY;
				}
				else if (paint_mode_==3)
				{
					z << SEG_BODY_LABEL_INIT_POSTER;
					z << SEG_BODY_LABEL_INIT_COPY;
				}
				break;
			case LBL_SHDR_MIF:
				//if (paint_mode_==0 || paint_mode_==2)
				//	z << SEG_BODY_LABEL_MAX_FILTER;
				//else if (paint_mode_==1 || paint_mode_==3)
				//	z << SEG_BODY_LABEL_MIF_POSTER;
				z << VOL_HEAD_LIT;
				z << VOL_DATA_VOLUME_LOOKUP_130;
				z << VOL_MEASURE_GM_LOOKUP;
				z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L;
				//z << SEG_BODY_DB_GROW_MEASURE;
				//z << SEG_BODY_DB_GROW_STOP_FUNC_MEASURE;
					z << SEG_BODY_LABEL_MAX_FILTER;
				break;
			case FLT_SHDR_NR:
				z << FLT_BODY_NR;
				break;
			}
		}

		//tail
		z << SEG_TAIL;

		s = z.str();
		return false;
	}

	SegShaderFactory::SegShaderFactory()
		: prev_shader_(-1)
	{}

	SegShaderFactory::~SegShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			delete shader_[i];
		}
	}

	FragmentProgram* SegShaderFactory::shader(int type, int paint_mode, int hr_mode,
		bool use_2d, bool shading, int peel, bool clip, bool hiqual)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(type, paint_mode, hr_mode, use_2d, shading, peel, clip, hiqual)) 
			{
				return shader_[prev_shader_]->program();
			}
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(type, paint_mode, hr_mode, use_2d, shading, peel, clip, hiqual)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		SegShader* s = new SegShader(type, paint_mode, hr_mode, use_2d, shading, peel, clip, hiqual);
		if(s->create())
		{
			delete s;
			return 0;
		}
		shader_.push_back(s);
		prev_shader_ = (int)shader_.size()-1;
		return s->program();
	}

} // end namespace FLIVR

