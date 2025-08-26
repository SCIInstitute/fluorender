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

#ifndef VOL_CAL_SHADER_CODE_H
#define VOL_CAL_SHADER_CODE_H

inline constexpr const char* CAL_OUTPUTS = R"GLSHDR(
//CAL_OUTPUTS
out vec4 FragColor;
)GLSHDR";

inline constexpr const char* CAL_VERTEX_CODE = R"GLSHDR(
//CAL_VERTEX_CODE
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

inline constexpr const char* CAL_UNIFORMS_COMMON = R"GLSHDR(
//CAL_UNIFORMS_COMMON
uniform sampler3D tex1;//operand A
uniform sampler3D tex2;//operand B
uniform vec4 loc0;//(scale_A, scale_B, 0.0, 0.0)
)GLSHDR";

inline constexpr const char* CAL_UNIFORMS_WITH_MASK = R"GLSHDR(
//CAL_UNIFORMS_WITH_MASK
uniform sampler3D tex1;//operand A
uniform sampler3D tex3;//mask of A
uniform sampler3D tex2;//operand B
uniform sampler3D tex4;//mask of B
uniform vec4 loc0;//(scale_a, scale_b, use_mask_a, use_mask_b)
)GLSHDR";

inline constexpr const char* CAL_HEAD = R"GLSHDR(
//CAL_HEAD
void main()
{
	vec4 t = vec4(OutTexture, 1.0);//position in the result volume
	vec4 t1 = t;//position in the operand A
	vec4 t2 = t;//position in the operand B
)GLSHDR";

inline constexpr const char* CAL_TEX_LOOKUP = R"GLSHDR(
	//CAL_TEX_LOOKUP
	vec4 c1 = texture(tex1, t1.stp);
	vec4 c2 = texture(tex2, t2.stp);
)GLSHDR";

inline constexpr const char* CAL_TEX_LOOKUP_WITH_MASK = R"GLSHDR(
	//CAL_TEX_LOOKUP_WITH_MASK
	vec4 c1 = texture3D(tex1, t1.stp);
	vec4 m1 = loc0.z>0.0?texture(tex3, t1.stp):vec4(1.0);
	vec4 c2 = texture(tex2, t2.stp);
	vec4 m2 = loc0.w>0.0?texture(tex4, t2.stp):vec4(1.0);
)GLSHDR";

inline constexpr const char* CAL_BODY_SUBSTRACTION = R"GLSHDR(
	//CAL_BODY_SUBSTRACTION
	vec4 c = vec4(clamp(c1.x-c2.x, 0.0, 1.0));
)GLSHDR";

inline constexpr const char* CAL_BODY_ADDITION = R"GLSHDR(
	//CAL_BODY_ADDITION
	vec4 c = vec4(clamp(c1.x+c2.x, 0.0, 1.0));
)GLSHDR";

inline constexpr const char* CAL_BODY_DIVISION = R"GLSHDR(
	//CAL_BODY_DIVISION
	vec4 c = vec4(0.0);
	if (c1.x>1e-5 && c2.x>1e-5)
		c = vec4(clamp(c1.x/c2.x, 0.0, 1.0));
)GLSHDR";

inline constexpr const char* CAL_BODY_INTERSECTION = R"GLSHDR(
	//CAL_BODY_INTERSECTION
	vec4 c = vec4(min(c1.x, c2.x));
)GLSHDR";

inline constexpr const char* CAL_BODY_INTERSECTION_WITH_MASK = R"GLSHDR(
	//CAL_BODY_INTERSECTION_WITH_MASK
	vec4 c = vec4(min(c1.x*m1.x, c2.x*m2.x));
)GLSHDR";

inline constexpr const char* CAL_BODY_APPLYMASK = R"GLSHDR(
	//CAL_BODY_APPLYMASK
	vec4 c = vec4((loc0.z<0.0?(1.0-c1.x):c1.x)*c2.x);
)GLSHDR";

inline constexpr const char* CAL_BODY_APPLYMASKINV = R"GLSHDR(
	//CAL_BODY_APPLYMASKINV
	vec4 c = vec4(c1.x*(1.0-c2.x));
)GLSHDR";

inline constexpr const char* CAL_RESULT = R"GLSHDR(
	//CAL_RESULT
	FragColor = c;
)GLSHDR";

inline constexpr const char* CAL_TAIL = R"GLSHDR(
//CAL_TAIL
}
)GLSHDR";

#endif//VOL_CAL_SHADER_CODE_H