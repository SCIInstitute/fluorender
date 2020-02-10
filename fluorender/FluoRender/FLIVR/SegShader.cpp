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
#define SEG_OUTPUTS \
	"//SEG_OUTPUTS\n" \
	"out vec4 FragColor;\n" \
	"\n"

#define SEG_VERTEX_CODE \
	"//SEG_VERTEX_CODE\n" \
	"layout(location = 0) in vec3 InVertex;\n" \
	"layout(location = 1) in vec3 InTexture;\n" \
	"out vec3 OutVertex;\n" \
	"out vec3 OutTexture;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	gl_Position = vec4(InVertex,1.);\n" \
	"	OutTexture = InTexture;\n" \
	"	OutVertex  = InVertex;\n" \
	"}\n" 

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
	"out uint FragUint;\n"\
	"\n"

#define SEG_UNIFORMS_PARAMS \
	"//SEG_UNIFORMS_PARAMS\n" \
	"uniform vec4 loc7;//(ini_thresh, gm_falloff, scl_falloff, scl_translate)\n" \
	"uniform vec4 loc8;//(weight_2d, post_bins, 0, 0)\n" \
	"\n"

#define SEG_TAIL \
	"//SEG_TAIL\n" \
	"}\n"

#define SEG_BODY_WEIGHT \
	"	//SEG_BODY_WEIGHT\n" \
	"	vec4 cw2d;\n" \
	"	vec4 weight1 = texture(tex4, s.xy);\n" \
	"	vec4 weight2 = texture(tex5, s.xy);\n" \
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
	"	FragColor = vec4(0.0);\n"\
	"\n"

#define SEG_BODY_INIT_2D_COORD \
	"	//SEG_BODY_INIT_2D_COORD\n" \
	"	vec4 s = matrix1 * matrix0 * matrix2 * t;\n"\
	"	s = s / s.w;\n"\
	"	s.xy = s.xy / 2.0 + 0.5;\n"\
	"\n"

#define SEG_BODY_INIT_CULL \
	"	//SEG_BODY_INIT_CULL\n" \
	"	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||\n"\
	"		any(greaterThan(s.xy, vec2(1.0, 1.0))))\n"\
	"		discard;\n"\
	"	float cmask2d = texture(tex6, s.xy).x;\n"\
	"	if (cmask2d < 0.95)\n"\
	"		discard;\n"\
	"\n"

#define SEG_BODY_INIT_CULL_ERASE \
	"	//SEG_BODY_INIT_CULL_ERASE\n" \
	"	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||\n"\
	"		any(greaterThan(s.xy, vec2(1.0, 1.0))))\n"\
	"		discard;\n"\
	"	float cmask2d = texture(tex6, s.xy).x;\n"\
	"	if (cmask2d < 0.45)\n"\
	"		discard;\n"\
	"\n"

#define SEG_BODY_INIT_CULL_POINT \
	"	//SEG_BODY_INIT_CULL_POINT\n" \
	"	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||\n"\
	"		any(greaterThan(s.xy, vec2(1.0, 1.0))))\n"\
	"		discard;\n"\
	"	float dist = length(s.xy*loc6.zw - loc6.xy);\n" \
	"	if (dist > 1.0)\n" \
	"		discard;\n"\
	"\n"

#define SEG_BODY_INIT_BLEND_HEAD \
	"	//SEG_BODY_INIT_BLEND_HEAD\n" \
	"	c = c*l.w;\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_APPEND \
	"	//SEG_BODY_INIT_BLEND_APPEND\n" \
	"	FragColor = vec4(c.x>0.0?(c.x>loc7.x?1.0:0.0):0.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_ERASE \
	"	//SEG_BODY_INIT_BLEND_ERASE\n" \
	"	FragColor = vec4(0.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_DIFFUSE \
	"	//SEG_BODY_INIT_BLEND_DIFFUSE\n" \
	"	FragColor = texture(tex2, t.stp);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_FLOOD \
	"	//SEG_BODY_INIT_BLEND_FLOOD\n" \
	"	FragColor = vec4(c.x>0.0?(c.x>loc7.x?1.0:0.0):0.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_ALL \
	"	//SEG_BODY_INIT_BLEND_ALL\n" \
	"	FragColor = vec4(1.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_HR_ORTHO \
	"	//SEG_BODY_INIT_BLEND_HR_ORTHO\n" \
	"	if (c.x <= loc7.x)\n" \
	"		discard;\n"\
	"	vec4 cv = matrix3 * vec4(0.0, 0.0, 1.0, 0.0);\n" \
	"	vec3 step = cv.xyz;\n" \
	"	step = normalize(step);\n" \
	"	step = step * length(step * loc4.xyz);\n" \
	"	vec3 ray = t.xyz;\n" \
	"	vec4 cray;\n" \
	"	bool flag = false;\n" \
	"	float th = loc7.x<0.01?0.01:loc7.x;\n" \
	"	while (true)\n" \
	"	{\n" \
	"		ray += step;\n" \
	"		if (any(greaterThan(ray, vec3(1.0))) ||\n" \
	"				any(lessThan(ray, vec3(0.0))))\n" \
	"			break;\n" \
	"		if (vol_clip_func(vec4(ray, 1.0)))\n" \
	"			break;\n" \
	"		v.x = texture(tex0, ray).x;\n" \
	"		v.y = length(vol_grad_func(vec4(ray, 1.0), loc4).xyz);\n" \
	"		cray = vol_trans_sin_color_l(v);\n" \
	"		if (cray.x > th && flag)\n" \
	"			discard;\n"\
	"		if (cray.x <= th)\n" \
	"			flag = true;\n" \
	"	}\n" \
	"	FragColor = vec4(1.0);\n" \
	"\n"

#define SEG_BODY_INIT_BLEND_HR_PERSP \
	"	//SEG_BODY_INIT_BLEND_HR_PERSP\n" \
	"	if (c.x <= loc7.x)\n" \
	"		discard;\n"\
	"	vec4 cv = matrix3 * vec4(0.0, 0.0, 0.0, 1.0);\n" \
	"	cv = cv / cv.w;\n" \
	"	vec3 step = cv.xyz - t.xyz;\n" \
	"	step = normalize(step);\n" \
	"	step = step * length(step * loc4.xyz);\n" \
	"	vec3 ray = t.xyz;\n" \
	"	vec4 cray;\n" \
	"	bool flag = false;\n" \
	"	float th = loc7.x<0.01?0.01:loc7.x;\n" \
	"	while (true)\n" \
	"	{\n" \
	"		ray += step;\n" \
	"		if (any(greaterThan(ray, vec3(1.0))) ||\n" \
	"				any(lessThan(ray, vec3(0.0))))\n" \
	"			break;\n" \
	"		if (vol_clip_func(vec4(ray, 1.0)))\n" \
	"			break;\n" \
	"		v.x = texture(tex0, ray).x;\n" \
	"		v.y = length(vol_grad_func(vec4(ray, 1.0), loc4).xyz);\n" \
	"		cray = vol_trans_sin_color_l(v);\n" \
	"		if (cray.x > th && flag)\n" \
	"		{\n" \
	"			FragColor = vec4(0.0);\n" \
	"			return;\n" \
	"		}\n" \
	"		if (cray.x <= th)\n" \
	"			flag = true;\n" \
	"	}\n" \
	"	FragColor = vec4(1.0);\n" \
	"\n"

#define SEG_BODY_DB_GROW_2D_COORD \
	"	//SEG_BODY_DB_GROW_2D_COORD\n" \
	"	vec4 s = matrix1 * matrix0 * matrix2 * t;\n"\
	"	s = s / s.w;\n"\
	"	s.xy = s.xy / 2.0 + 0.5;\n"\
	"	vec4 cc = texture(tex2, t.stp);\n"\
	"\n"

#define SEG_BODY_DB_GROW_CULL \
	"	//SEG_BODY_DB_GROW_CULL\n" \
	"	if (any(lessThan(s.xy, vec2(0.0, 0.0))) ||\n"\
	"		any(greaterThan(s.xy, vec2(1.0, 1.0))))\n"\
	"		discard;\n"\
	"	float cmask2d = texture(tex6, s.xy).x;\n"\
	"	if (cmask2d < 0.45 /*|| cmask2d > 0.55*/)\n"\
	"		discard;\n"\
	"\n"

#define SEG_BODY_DB_GROW_STOP_FUNC \
	"	//SEG_BODY_DB_GROW_STOP_FUNC\n" \
	"	if (c.x == 0.0)\n" \
	"		discard;\n"\
	"	v.x = c.x>1.0?1.0:c.x;\n" \
	"	float stop = \n" \
	"		(loc7.y>=1.0?1.0:(v.y>sqrt(loc7.y)*2.12?0.0:exp(-v.y*v.y/loc7.y)))*\n" \
	"		(v.x>loc7.w?1.0:(loc7.z>0.0?(v.x<loc7.w-sqrt(loc7.z)*2.12?0.0:exp(-(v.x-loc7.w)*(v.x-loc7.w)/loc7.z)):0.0));\n" \
	"	if (stop == 0.0)\n" \
	"		discard;\n"\
	"\n"

#define SEG_BODY_DB_GROW_BLEND_APPEND_HEAD \
	"	//SEG_BODY_DB_GROW_BLEND_APPEND\n" \
	"	FragColor = (1.0-stop) * cc;\n" \
	"	vec3 nb;\n" \
	"	vec3 max_nb = t.stp;\n" \
	"	float m;\n" \
	"	float mx;\n" \
	"	for (int i=-1; i<2; i++)\n"\
	"	for (int j=-1; j<2; j++)\n"\
	"	for (int k=-1; k<2; k++)\n"\
	"	{\n"

#define SEG_BODY_DB_GROW_BLEND_APPEND_DIR \
	"		//SEG_BODY_DB_GROW_BLEND_APPEND_DIR\n" \
	"		vec3 ndir = vec3(i, j, k);\n" \
	"		ndir = normalize(ndir);\n" \
	"		float ang = dot(ndir, loc9.xyz);\n" \
	"		if (ang < 0.5) continue;\n"

#define SEG_BODY_DB_GROW_BLEND_APPEND_BODY \
	"		//SEG_BODY_DB_GROW_BLEND_APPEND_BODY\n" \
	"		nb = vec3(t.s+float(i)*loc4.x, t.t+float(j)*loc4.y, t.p+float(k)*loc4.z);\n"\
	"		m = texture(tex2, nb).x;\n" \
	"		if (m > cc.x)\n" \
	"		{\n" \
	"			cc = vec4(m);\n" \
	"			max_nb = nb;\n" \
	"		}\n" \
	"	}\n"\
	"	if (loc7.y>0.0)\n" \
	"	{\n" \
	"		m = texture(tex0, max_nb).x + loc7.y;\n" \
	"		mx = texture(tex0, t.stp).x;\n" \
	"		if (m < mx || m - mx > 2.0*loc7.y)\n" \
	"			discard;\n"\
	"	}\n" \
	"	FragColor += cc*stop;\n"\
	"\n"

#define SEG_BODY_DB_GROW_BLEND_ERASE0 \
	"	//SEG_BODY_DB_GROW_BLEND_ERASE\n" \
	"	for (int i=-1; i<2; i++)\n"\
	"	for (int j=-1; j<2; j++)\n"\
	"	for (int k=-1; k<2; k++)\n"\
	"	{\n"\
	"		vec3 nb = vec3(t.s+float(i)*loc4.x, t.t+float(j)*loc4.y, t.p+float(k)*loc4.z);\n"\
	"		cc = vec4(min(cc.x, texture(tex2, nb).x));\n"\
	"	}\n"\
	"	FragColor = cc*clamp(1.0-stop, 0.0, 1.0);\n"\
	"\n"

#define SEG_BODY_DB_GROW_BLEND_ERASE \
	"	//SEG_BODY_DB_GROW_BLEND_ERASE\n" \
	"	FragColor = vec4(0.0);\n"\
	"\n"

	SegShader::SegShader(int type, int paint_mode, int hr_mode,
		bool use_2d, bool shading, int peel,
		bool clip, bool hiqual, bool use_dir) :
	type_(type),
	paint_mode_(paint_mode),
	hr_mode_(hr_mode),
	use_2d_(use_2d),
	shading_(shading),
	peel_(peel),
	clip_(clip),
	hiqual_(hiqual),
	use_dir_(use_dir),
	program_(0)
	{}

	SegShader::~SegShader()
	{
		delete program_;
	}

	bool SegShader::create()
	{
		string vs = ShaderProgram::glsl_version_ +
			ShaderProgram::glsl_unroll_ +
			SEG_VERTEX_CODE;
		string fs;
		if (emit(fs)) return true;
		program_ = new ShaderProgram(vs, fs);
		return false;
	}

	bool SegShader::emit(string& s)
	{
		ostringstream z;

		z << ShaderProgram::glsl_version_;
		z << ShaderProgram::glsl_unroll_;
		z << VOL_INPUTS;

		//uniforms
		z << SEG_OUTPUTS;
		z << VOL_UNIFORMS_COMMON;
		z << VOL_UNIFORMS_SIN_COLOR;
		//for paint_mode==9, loc6 = (px, py, view_nx, view_ny)
		z << VOL_UNIFORMS_MASK;
		z << SEG_UNIFORMS_WMAP_2D;
		z << SEG_UNIFORMS_MASK_2D;
		z << SEG_UNIFORMS_MATRICES;
		z << VOL_UNIFORMS_MATRICES;
		z << SEG_UNIFORMS_PARAMS;

		//uniforms for clipping
		if (paint_mode_!=6 && clip_)
			z << VOL_UNIFORMS_CLIP;

		//for hidden removal
		if (type_==SEG_SHDR_INITIALIZE &&
			(paint_mode_==1 || paint_mode_==2 ||
			paint_mode_==9) &&
			hr_mode_>0)
		{
			z << SEG_UNIFORM_MATRICES_INVERSE;
			z << VOL_GRAD_COMPUTE_FUNC;
			z << VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC;
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
				if (paint_mode_ == 1 ||
					paint_mode_ == 2 ||
					paint_mode_ == 4 ||
					paint_mode_ == 8)
					z << SEG_BODY_INIT_CULL;
				else if (paint_mode_ == 3)
					z << SEG_BODY_INIT_CULL_ERASE;
				else if (paint_mode_ == 9)
					z << SEG_BODY_INIT_CULL_POINT;

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
					paint_mode_==2 ||
					paint_mode_==9)
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
				else if (paint_mode_==8)
					z << SEG_BODY_INIT_BLEND_ALL;
				break;
			case SEG_SHDR_DB_GROW:
				z << SEG_BODY_DB_GROW_2D_COORD;

				if (paint_mode_!=5 &&
					paint_mode_!=9)
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

				if (paint_mode_ == 1 ||
					paint_mode_ == 2 ||
					paint_mode_ == 4 ||
					paint_mode_ == 5 ||
					paint_mode_ == 9)
				{
					z << SEG_BODY_DB_GROW_BLEND_APPEND_HEAD;
					if (use_dir_)
						z << SEG_BODY_DB_GROW_BLEND_APPEND_DIR;
					z << SEG_BODY_DB_GROW_BLEND_APPEND_BODY;
				}
				else if (paint_mode_==3)
					z << SEG_BODY_DB_GROW_BLEND_ERASE;
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
			delete shader_[i];
	}

	void SegShaderFactory::clear()
	{
		for (unsigned int i = 0; i<shader_.size(); i++)
			delete shader_[i];
		shader_.clear();
		prev_shader_ = -1;
	}

	ShaderProgram* SegShaderFactory::shader(
		int type, int paint_mode, int hr_mode,
		bool use_2d, bool shading, int peel,
		bool clip, bool hiqual, bool use_dir)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(
				type, paint_mode, hr_mode,
				use_2d, shading, peel,
				clip, hiqual, use_dir)) 
				return shader_[prev_shader_]->program();
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(
				type, paint_mode, hr_mode,
				use_2d, shading, peel,
				clip, hiqual, use_dir)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		SegShader* s = new SegShader(
			type, paint_mode, hr_mode,
			use_2d, shading, peel,
			clip, hiqual, use_dir);
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

