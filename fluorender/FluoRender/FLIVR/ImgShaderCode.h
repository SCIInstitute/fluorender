//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

namespace flvr
{
#define IMG_SHDR_CODE_DRAW_THICK_LINES \
	"uniform vec4 loc0; //(vp0, vp1, thickness, 0.0)\n" \
	"layout(lines) in;\n" \
	"layout(triangle_strip, max_vertices = 4) out;\n" \
	"in vec3 OutColor[];\n" \
	"out vec3 OutColor2;\n" \
	"vec2 toScreenSpace(vec4 vertex)\n" \
	"{\n" \
	"	return vec2(vertex.xy / vertex.w) * loc0.xy;\n" \
	"}\n" \
	"float toZValue(vec4 vertex)\n" \
	"{\n" \
	"	return (vertex.z / vertex.w);\n" \
	"}\n" \
	"void main() {\n" \
	"	OutColor2 = OutColor[0];\n" \
	"	vec4 pps[2];\n" \
	"	pps[0] = gl_in[0].gl_Position;\n" \
	"	pps[1] = gl_in[1].gl_Position;\n" \
	"	vec2 sps[2];\n" \
	"	sps[0] = toScreenSpace(pps[0]);\n" \
	"	sps[1] = toScreenSpace(pps[1]);\n" \
	"	float zs[2];\n" \
	"	zs[0] = toZValue(pps[0]);\n" \
	"	zs[1] = toZValue(pps[1]);\n" \
	"	vec2 v0 = normalize(sps[1] - sps[0]);\n" \
	"	vec2 n0 = vec2(-v0.y, v0.x);\n" \
	"	vec2 tps[4];\n" \
	"	tps[0] = sps[0] + n0 * loc0.z / 2.0;\n" \
	"	tps[1] = sps[0] - n0 * loc0.z / 2.0;\n" \
	"	tps[2] = sps[1] + n0 * loc0.z / 2.0;\n" \
	"	tps[3] = sps[1] - n0 * loc0.z / 2.0;\n" \
	"	gl_Position = vec4(tps[0] / loc0.xy, zs[0], 1.0);\n" \
	"	EmitVertex();\n" \
	"	gl_Position = vec4(tps[1] / loc0.xy, zs[0], 1.0);\n" \
	"	EmitVertex();\n" \
	"	gl_Position = vec4(tps[2] / loc0.xy, zs[1], 1.0);\n" \
	"	EmitVertex();\n" \
	"	gl_Position = vec4(tps[3] / loc0.xy, zs[1], 1.0);\n" \
	"	EmitVertex();\n" \
	"	EndPrimitive();\n" \
	"}\n" 

#define IMG_FRG_CODE_DRAW_THICKLINES \
	"//IMG_FRG_CODE_DRAW_GEOMETRY_COLOR3\n" \
	"in vec3 OutColor2;\n" \
	"out vec4 FragColor;\n" \
	"\n" \
	"void main()\n" \
	"{\n" \
	"	FragColor = vec4(OutColor2, 1.0);\n" \
	"}\n"


}//namespace flvr