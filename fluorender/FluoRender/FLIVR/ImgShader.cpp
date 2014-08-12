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
#include <FLIVR/ImgShader.h>
#include <FLIVR/ShaderProgram.h>

using std::string;
using std::vector;
using std::ostringstream;

namespace FLIVR
{

#define IMG_SHADER_CODE_BRIGHTNESS_CONTRAST \
	"// IMG_SHADER_CODE_BRIGHTNESS_CONTRAST\n" \
	"uniform vec4 loc0; //(r_gamma, g_gamma, b_gamma, 1.0)\n" \
	"uniform vec4 loc1; //(r_brightness, g_brightness, b_brightness, 1.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	vec4 b = loc1;\n" \
	"	b.x = b.x>1.0?1.0/(2.0-b.x):loc1.x;\n" \
	"	b.y = b.y>1.0?1.0/(2.0-b.y):loc1.y;\n" \
	"	b.z = b.z>1.0?1.0/(2.0-b.z):loc1.z;\n" \
	"	gl_FragColor = pow(c, loc0)*b;\n" \
	"}\n"

#define IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR \
	"// IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR\n" \
	"uniform vec4 loc0; //(r_gamma, g_gamma, b_gamma, 1.0)\n" \
	"uniform vec4 loc1; //(r_brightness, g_brightness, b_brightness, 1.0)\n" \
	"uniform vec4 loc2; //(r_hdr, g_hdr, b_hdr, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	vec4 c_avg_1 = max(texture2DLod(tex0, t.xy, 1), 0.001);\n" \
	"	vec4 c_avg_2 = max(texture2DLod(tex0, t.xy, 2), 0.001);\n" \
	"	vec4 c_avg_3 = max(texture2DLod(tex0, t.xy, 3), 0.001);\n" \
	"	vec4 c_avg_4 = max(texture2DLod(tex0, t.xy, 4), 0.001);\n" \
	"	vec4 c_avg_5 = max(texture2DLod(tex0, t.xy, 5), 0.001);\n" \
	"	vec4 b = loc1;\n" \
	"	b.x = b.x>1.0?(b.x<2.0?1.0/(2.0-b.x):256.0):loc1.x;\n" \
	"	b.y = b.y>1.0?(b.y<2.0?1.0/(2.0-b.y):256.0):loc1.y;\n" \
	"	b.z = b.z>1.0?(b.z<2.0?1.0/(2.0-b.z):256.0):loc1.z;\n" \
	"	c = pow(c, loc0);\n" \
	"	c_avg_1 = c/pow(c_avg_1, loc0);\n" \
	"	c_avg_2 = c/pow(c_avg_2, loc0);\n" \
	"	c_avg_3 = c/pow(c_avg_3, loc0);\n" \
	"	c_avg_4 = c/pow(c_avg_4, loc0);\n" \
	"	c_avg_5 = c/pow(c_avg_5, loc0);\n" \
	"	c *= b;\n" \
	"	vec4 c_avg = 0.25*c_avg_1+0.3*c_avg_2+0.25*c_avg_3+0.1*c_avg_4+0.1*c_avg_5;\n" \
	"	gl_FragColor = c*(vec4(1.0)-loc2)+loc2*c_avg;\n" \
	"}\n"

#define IMG_SHADER_CODE_GRADIENT_MAP \
	"// IMG_SHADER_CODE_GRADIENT_MAP\n" \
	"uniform vec4 loc0; //(lo, hi, hi-lo, alpha) \n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"   vec4 color;\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	color.a = (loc0.w>0.5?loc0.w:c.x)*c.a;\n" \
	"	c.w = (c.r + c.g + c.b)/3.0;\n" \
	"	c.w = (c.w-loc0.x)/loc0.z;\n" \
	"	color.b = clamp(-4.0*c.w+2.0, 0.0, 1.0);\n" \
	"	color.g = clamp(c.w<0.5 ? 4.0*c.w : -4.0*c.w+4.0, 0.0, 1.0);\n" \
	"	color.r = clamp(4.0*c.w - 2.0, 0.0, 1.0);\n" \
	"	gl_FragColor = vec4(color.rgb*color.a, color.a);\n" \
	"}\n"

#define IMG_SHADER_CODE_FILTER_MIN \
	"// IMG_SHADER_CODE_FILTER_MIN\n" \
	"uniform vec4 loc0; //(width, height, thresh, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	if (min(c.r, min(c.g, c.b)) >= loc0.z)\n" \
	"	{\n" \
	"		gl_FragColor = c;\n" \
	"		return;\n" \
	"	}\n" \
	"	vec4 c1 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c2 = texture2D(tex0, vec2(t.x, t.y-loc0.y));\n" \
	"	vec4 c3 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c4 = texture2D(tex0, vec2(t.x-loc0.x, t.y));\n" \
	"	vec4 c5 = texture2D(tex0, vec2(t.x+loc0.x, t.y));\n" \
	"	vec4 c6 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	vec4 c7 = texture2D(tex0, vec2(t.x, t.y+loc0.y));\n" \
	"	vec4 c8 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	c = min(c, c1);\n" \
	"	c = min(c, c2);\n" \
	"	c = min(c, c3);\n" \
	"	c = min(c, c4);\n" \
	"	c = min(c, c5);\n" \
	"	c = min(c, c6);\n" \
	"	c = min(c, c7);\n" \
	"	c = min(c, c8);\n" \
	"	gl_FragColor = c;\n" \
	"}\n"

#define IMG_SHADER_CODE_FILTER_MAX \
	"// IMG_SHADER_CODE_FILTER_MAX\n" \
	"uniform vec4 loc0; //(width, height, scale, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	float s = pow(1.0-max(c.r, max(c.g, c.b)), 3.0)*loc0.z;\n" \
	"	vec4 c1 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c2 = texture2D(tex0, vec2(t.x, t.y-loc0.y));\n" \
	"	vec4 c3 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c4 = texture2D(tex0, vec2(t.x-loc0.x, t.y));\n" \
	"	vec4 c5 = texture2D(tex0, vec2(t.x+loc0.x, t.y));\n" \
	"	vec4 c6 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	vec4 c7 = texture2D(tex0, vec2(t.x, t.y+loc0.y));\n" \
	"	vec4 c8 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	c = max(c, c1);\n" \
	"	c = max(c, c2);\n" \
	"	c = max(c, c3);\n" \
	"	c = max(c, c4);\n" \
	"	c = max(c, c5);\n" \
	"	c = max(c, c6);\n" \
	"	c = max(c, c7);\n" \
	"	c = max(c, c8);\n" \
	"	gl_FragColor = c;\n" \
	"}\n"

#define IMG_SHADER_CODE_FILTER_BLUR \
	"// IMG_SHADER_CODE_FILTER_BLUR\n" \
	"uniform vec4 loc0; //(width, height, dx, dy)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	vec4 c1 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c2 = texture2D(tex0, vec2(t.x, t.y-loc0.y));\n" \
	"	vec4 c3 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c4 = texture2D(tex0, vec2(t.x-loc0.x, t.y));\n" \
	"	vec4 c5 = texture2D(tex0, vec2(t.x+loc0.x, t.y));\n" \
	"	vec4 c6 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	vec4 c7 = texture2D(tex0, vec2(t.x, t.y+loc0.y));\n" \
	"	vec4 c8 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	//float x = avg(c.x, c1.x, c2.x, c3.x, c4.x, c5.x, c6.x, c7.x, c8.x).x;\n" \
	"	//float y = avg(c.y, c1.y, c2.y, c3.y, c4.y, c5.y, c6.y, c7.y , c8.y).y;\n" \
	"	//float z = avg(c.z, c1.z, c2.z, c3.z, c4.z, c5.z, c6.z, c7.z, c8.z).z;\n" \
	"	gl_FragColor = (c1+c2+c3+c4+c5+c6+c7+c8)/8.0;\n" \
	"}\n"
#define IMG_SHADER_CODE_FILTER_SHARPEN \
	"// IMG_SHADER_CODE_FILTER_SHARPEN\n" \
	"uniform vec4 loc0; //(width, height, 0.0, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	vec4 c1 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c2 = texture2D(tex0, vec2(t.x, t.y-loc0.y));\n" \
	"	vec4 c3 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c4 = texture2D(tex0, vec2(t.x-loc0.x, t.y));\n" \
	"	vec4 c5 = texture2D(tex0, vec2(t.x+loc0.x, t.y));\n" \
	"	vec4 c6 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	vec4 c7 = texture2D(tex0, vec2(t.x, t.y+loc0.y));\n" \
	"	vec4 c8 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	c = c*9.0 - (c1+c2+c3+c4+c5+c6+c7+c8);\n" \
	"	gl_FragColor = c;\n" \
	"}\n"

#define IMG_SHADER_CODE_DEPTH_TO_OUTLINES \
	"//IMG_SHADER_CODE_DEPTH_TO_OUTLINES\n" \
	"uniform vec4 loc0; //(width, height, 0.0, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	//vec4 c1 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c2 = texture2D(tex0, vec2(t.x, t.y-loc0.y));\n" \
	"	//vec4 c3 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y-0.70711*loc0.y));\n" \
	"	vec4 c4 = texture2D(tex0, vec2(t.x-loc0.x, t.y));\n" \
	"	vec4 c5 = texture2D(tex0, vec2(t.x+loc0.x, t.y));\n" \
	"	//vec4 c6 = texture2D(tex0, vec2(t.x-0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	vec4 c7 = texture2D(tex0, vec2(t.x, t.y+loc0.y));\n" \
	"	//vec4 c8 = texture2D(tex0, vec2(t.x+0.70711*loc0.x, t.y+0.70711*loc0.y));\n" \
	"	c = (c5-c4)*(c5-c4)+(c7-c2)*(c7-c2);\n" \
	"	c = clamp(c*2e6, 0.0, 1.0);\n" \
	"	gl_FragColor = vec4(1.0-c.r, 1.0-c.g, 1.0-c.b, 1.0);\n" \
	"}\n" \
	"\n"

#define IMG_SHADER_CODE_DEPTH_TO_GRADIENT \
	"//IMG_SHADER_CODE_DEPTH_TO_GRADIENT\n" \
	"uniform vec4 loc0; //(width, height, scale, 0.0)\n" \
	"uniform vec4 loc1; //(pert.x, pert.y, 0.0, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	float c;\n" \
	"	vec4 c2 = texture2D(tex0, vec2(t.x, t.y-loc0.y));\n" \
	"	vec4 c4 = texture2D(tex0, vec2(t.x-loc0.x, t.y));\n" \
	"	vec4 c5 = texture2D(tex0, vec2(t.x+loc0.x, t.y));\n" \
	"	vec4 c7 = texture2D(tex0, vec2(t.x, t.y+loc0.y));\n" \
	"	vec2 grad = vec2(c5.r-c4.r, c7.r-c2.r);\n" \
	"	vec2 pert = loc1.xy;\n" \
	"	float ang = max(dot(normalize(grad), pert)*2.0, -1.0);\n" \
	"	pert = grad*ang;\n" \
	"	grad += pert;\n" \
	"	c = grad.x*grad.x+grad.y*grad.y;\n" \
	"	c = clamp(c*loc0.z, 0.0, 1.0);\n" \
	"	gl_FragColor = vec4(grad, c, 1.0);\n" \
	"}\n" \
	"\n"

#define IMG_SHADER_CODE_GRADIENT_TO_SHADOW \
	"//IMG_SHADER_CODE_GRADIENT_TO_SHADOW\n" \
	"uniform vec4 loc0; //(1/width, 1/height, zoom, 0.0)\n" \
	"uniform vec4 loc1; //(darkness, 0.0, 0.0, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"uniform sampler2D tex1;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c0 = texture2D(tex1, t.xy);\n" \
	"	if (c0.w == 0.0)\n" \
	"	{\n" \
	"		gl_FragColor = vec4(1.0);\n" \
	"		return;\n" \
	"	}\n" \
	"	float c = 0.0;\n" \
	"	vec2 d = loc0.xy;\n" \
	"	vec2 nb;\n" \
	"	vec2 delta;\n" \
	"	vec4 c_nb;\n" \
	"	float ang;\n" \
	"	float dist;\n" \
	"	float dense;\n" \
	"	for (int i=-15; i<15; i++)\n" \
	"	for (int j=-15; j<15; j++)\n" \
	"	{\n" \
	"		delta = vec2(float(i)*d.x, float(j)*d.y);\n" \
	"		nb = t.st + delta;\n" \
	"		c_nb = texture2D(tex0, nb);\n" \
	"		ang = dot(normalize(delta), normalize(c_nb.xy));\n" \
	"		dist = pow(float(i)*float(i)+float(j)*float(j), 0.8);\n" \
	"		dist = dist==0.0?0.0:1.0/dist*clamp(loc0.z, 0.4, 3.0);\n" \
	"		dense = clamp(0.02+(3.0-loc0.z)*0.015, 0.02, 0.05);\n" \
	"		c += dense*(ang<-0.3?1.0:max(-ang+0.7, 0.0))*c_nb.z*dist;\n" \
	"	}\n" \
	"	c = clamp(1.0-clamp(c, 0.0, 1.0)*loc1.x, 0.01, 1.0);\n" \
	"	gl_FragColor = vec4(vec3(c), 1.0);\n" \
	"}\n" \
	"\n"

#define IMG_SHADER_CODE_GRADIENT_TO_SHADOW_MESH \
	"//IMG_SHADER_CODE_GRADIENT_TO_SHADOW_MESH\n" \
	"uniform vec4 loc0; //(1/width, 1/height, zoom, 0.0)\n" \
	"uniform vec4 loc1; //(darkness, 0.0, 0.0, 0.0)\n" \
	"uniform sampler2D tex0;\n" \
	"uniform sampler2D tex1;\n" \
	"uniform sampler2D tex2;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c1 = texture2D(tex1, t.xy);\n" \
	"	vec4 c2 = texture2D(tex2, t.xy);\n" \
	"	if (c1.x==1.0 && (c1.x<1.0 || c2.w==0.0))\n" \
	"	{\n" \
	"		gl_FragColor = vec4(1.0);\n" \
	"		return;\n" \
	"	}\n" \
	"	float c = 0.0;\n" \
	"	vec2 d = loc0.xy;\n" \
	"	vec2 nb;\n" \
	"	vec2 delta;\n" \
	"	vec4 c_nb;\n" \
	"	float ang;\n" \
	"	float dist;\n" \
	"	float dense;\n" \
	"	for (int i=-15; i<15; i++)\n" \
	"	for (int j=-15; j<15; j++)\n" \
	"	{\n" \
	"		delta = vec2(float(i)*d.x, float(j)*d.y);\n" \
	"		nb = t.st + delta;\n" \
	"		c_nb = texture2D(tex0, nb);\n" \
	"		ang = dot(normalize(delta), normalize(c_nb.xy));\n" \
	"		dist = pow(float(i)*float(i)+float(j)*float(j), 0.8);\n" \
	"		dist = dist==0.0?0.0:1.0/dist*clamp(loc0.z, 0.4, 3.0);\n" \
	"		dense = clamp(0.02+(3.0-loc0.z)*0.015, 0.02, 0.05);\n" \
	"		c += dense*(ang<-0.3?1.0:max(-ang+0.7, 0.0))*c_nb.z*dist;\n" \
	"	}\n" \
	"	c = clamp(1.0-clamp(c, 0.0, 1.0)*loc1.x, 0.01, 1.0);\n" \
	"	gl_FragColor = vec4(vec3(c), 1.0);\n" \
	"}\n" \
	"\n"

#define IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND\
	"// IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND\n" \
	"uniform sampler2D tex0;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 t = gl_TexCoord[0];\n" \
	"	vec4 c = texture2D(tex0, t.xy);\n" \
	"	float alpha = clamp(c.a, 0.0, 1.0);\n" \
	"	alpha = clamp(2.0*alpha - alpha*alpha, 0.0, 1.0);\n" \
	"	//alpha = alpha>0.0?1.0:0.0;\n" \
	"	gl_FragColor = vec4(c.rgb,alpha);\n" \
	"}\n"


	ImgShader::ImgShader(int type) : 
	type_(type),
	program_(0)
	{}

	ImgShader::~ImgShader()
	{
		delete program_;
	}

	bool
		ImgShader::create()
	{
		string s;
		if (emit(s)) return true;
		program_ = new FragmentProgram(s);
		return false;
	}

	bool
		ImgShader::emit(string& s)
	{
		ostringstream z;

		switch (type_)
		{
		case IMG_SHDR_BRIGHTNESS_CONTRAST:
			z << IMG_SHADER_CODE_BRIGHTNESS_CONTRAST;
			break;
		case IMG_SHDR_BRIGHTNESS_CONTRAST_HDR:
			z << IMG_SHADER_CODE_BRIGHTNESS_CONTRAST_HDR;
			break;
		case IMG_SHDR_GRADIENT_MAP:
			z << IMG_SHADER_CODE_GRADIENT_MAP;
			break;
		case IMG_SHDR_FILTER_BLUR:
			z << IMG_SHADER_CODE_FILTER_BLUR;
			break;
		case IMG_SHDR_FILTER_MAX:
			z << IMG_SHADER_CODE_FILTER_MAX;
			break;
		case IMG_SHDR_FILTER_SHARPEN:
			z << IMG_SHADER_CODE_FILTER_SHARPEN;
			break;
		case IMG_SHDR_DEPTH_TO_OUTLINES:
			z << IMG_SHADER_CODE_DEPTH_TO_OUTLINES;
			break;
		case IMG_SHDR_DEPTH_TO_GRADIENT:
			z << IMG_SHADER_CODE_DEPTH_TO_GRADIENT;
			break;
		case IMG_SHDR_GRADIENT_TO_SHADOW:
			z << IMG_SHADER_CODE_GRADIENT_TO_SHADOW;
			break;
		case IMG_SHDR_GRADIENT_TO_SHADOW_MESH:
			z << IMG_SHADER_CODE_GRADIENT_TO_SHADOW_MESH;
			break;
		case IMG_SHDR_BLEND_BRIGHT_BACKGROUND:
			z << IMG_SHADER_CODE_BLEND_BRIGHT_BACKGROUND;
		}

		s = z.str();
		//std::cerr << s << std::endl;
		return false;
	}


	ImgShaderFactory::ImgShaderFactory()
		: prev_shader_(-1)
	{}

	ImgShaderFactory::~ImgShaderFactory()
	{
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			delete shader_[i];
		}
	}

	FragmentProgram*
		ImgShaderFactory::shader(int type)
	{
		if(prev_shader_ >= 0)
		{
			if(shader_[prev_shader_]->match(type)) 
			{
				return shader_[prev_shader_]->program();
			}
		}
		for(unsigned int i=0; i<shader_.size(); i++)
		{
			if(shader_[i]->match(type)) 
			{
				prev_shader_ = i;
				return shader_[i]->program();
			}
		}

		ImgShader* s = new ImgShader(type);
		if(s->create())
		{
			delete s;
			return 0;
		}
		shader_.push_back(s);
		prev_shader_ = int(shader_.size())-1;
		return s->program();
	}

} // end namespace FLIVR

