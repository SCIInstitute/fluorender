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

#ifndef MESH_SHADER_CODE_H
#define MESH_SHADER_CODE_H

inline constexpr const char* MSH_VERTEX_INPUTS_V = R"GLSHDR(
//MSH_VERTEX_INPUTS_V
layout(location = 0) in vec3 InVertex;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_INPUTS_N = R"GLSHDR(
//MSH_VERTEX_INPUTS_N
layout(location = 1) in vec3 InNormal;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_INPUTS_T1 = R"GLSHDR(
//MSH_VERTEX_INPUTS_T1
layout(location = 1) in vec2 InTexcoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_INPUTS_T2 = R"GLSHDR(
//MSH_VERTEX_INPUTS_T1
layout(location = 2) in vec2 InTexcoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_N = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_N
out vec3 OutNormal;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_T = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_T
out vec2 OutTexcoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_FOG = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_FOG
out vec4 OutFogCoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_UNIFORM_MATRIX = R"GLSHDR(
//MSH_VERTEX_UNIFORM_MATRIX
uniform mat4 matrix0;//projection
uniform mat4 matrix1;//model view
)GLSHDR";

inline constexpr const char* MSH_VERTEX_UNIFORM_MATRIX_NORMAL = R"GLSHDR(
//MSH_VERTEX_UNIFORM_MATRIX_NORMAL
uniform mat4 matrix2;//normal
)GLSHDR";

inline constexpr const char* MSH_HEAD = R"GLSHDR(
// MSH_HEAD
void main()
{
)GLSHDR";

inline constexpr const char* MSH_VERTEX_BODY_POS = R"GLSHDR(
//MSH_VERTEX_BODY_POS
	gl_Position = matrix0 * matrix1 * vec4(InVertex, 1.0);
)GLSHDR";

inline constexpr const char* MSH_VERTEX_BODY_NORMAL = R"GLSHDR(
//MSH_VERTEX_BODY_NORMAL
	OutNormal = normalize((matrix2 * vec4(InNormal, 0.0)).xyz);
)GLSHDR";

inline constexpr const char* MSH_VERTEX_BODY_TEX = R"GLSHDR(
//MSH_VERTEX_BODY_TEX
	OutTexcoord = InTexcoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_BODY_FOG = R"GLSHDR(
//MSH_VERTEX_BODY_FOG
	OutFogCoord = matrix1 * vec4(InVertex,1.);
)GLSHDR";

inline constexpr const char* MSH_FRAG_OUTPUTS = R"GLSHDR(
//MSH_FRAG_OUTPUTS
out vec4 FragColor;
)GLSHDR";

inline constexpr const char* MSH_FRAG_OUTPUTS_INT = R"GLSHDR(
//MSH_FRAG_OUTPUTS_INT
out uint FragUint;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_N = R"GLSHDR(
//MSH_FRAG_INPUTS_N
in vec3 OutNormal;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_T = R"GLSHDR(
//MSH_FRAG_INPUTS_T
in vec2 OutTexcoord;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_FOG = R"GLSHDR(
//MSH_FRAG_INPUTS_FOG
in vec4 OutFogCoord;
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_COLOR = R"GLSHDR(
//MSH_FRAG_UNIFORMS_COLOR
uniform vec4 loc0;//color
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_NOMAT = R"GLSHDR(
//MSH_FRAG_UNIFORMS_NOMAT
uniform vec4 loc0;//color
uniform vec4 loc3;//(0, alpha, 0, 0)
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_TEX = R"GLSHDR(
// MSH_FRAG_UNIFORMS_TEX
uniform sampler2D tex0;
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_MATERIAL = R"GLSHDR(
//MSH_FRAG_UNIFORMS_MATERIAL
uniform vec4 loc0;//ambient color
uniform vec4 loc1;//diffuse color
uniform vec4 loc2;//specular color
uniform vec4 loc3;//(shine, alpha, 0, 0)
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_DP = R"GLSHDR(
// MSH_FRAG_UNIFORMS_DP
uniform vec4 loc7;//(1/vx, 1/vy, 0, 0)
uniform sampler2D tex13;
uniform sampler2D tex14;
uniform sampler2D tex15;
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_INT = R"GLSHDR(
//MSH_FRAG_UNIFORMS_INT
uniform uint loci0;//name
)GLSHDR";

//1: draw depth after 15 (15)
inline constexpr const char* MSH_FRAG_BODY_DP_1 = R"GLSHDR(
	// MSH_FRAG_BODY_DP_1
	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);
	if (texture(tex15, t).r >= gl_FragCoord.z-1e-6) discard;
)GLSHDR";

//2: draw mesh after 14 (14, 15)
inline constexpr const char* MSH_FRAG_BODY_DP_2 = R"GLSHDR(
	// MSH_FRAG_BODY_DP_2
	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);
	if (texture(tex14, t).r >= gl_FragCoord.z-1e-6) discard;
)GLSHDR";

//3: draw mesh after 13 and before 15 (13, 14, 15)
inline constexpr const char* MSH_FRAG_BODY_DP_3 = R"GLSHDR(
	// MSH_FRAG_BODY_DP_3
	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);
	if (texture(tex15, t).r <= gl_FragCoord.z+1e-6) discard;
	else if (texture(tex13, t).r >= gl_FragCoord.z-1e-6) discard;
)GLSHDR";

//4: draw mesh before 15 (at 14) (14, 15)
inline constexpr const char* MSH_FRAG_BODY_DP_4 = R"GLSHDR(
	// MSH_FRAG_BODY_DP_4
	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);
	if (texture(tex15, t).r <= gl_FragCoord.z+1e-6) discard;
)GLSHDR";

//5: draw mesh at 15 (15)
inline constexpr const char* MSH_FRAG_BODY_DP_5 = R"GLSHDR(
	// MSH_FRAG_BODY_DP_5
	vec2 t = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);
	if (texture(tex15, t).r <= gl_FragCoord.z-1e-6) discard;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_COLOR = R"GLSHDR(
	//MSH_FRAG_BODY_COLOR
	vec4 c = vec4(1.0);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_COLOR_OUT = R"GLSHDR(
	// MSH_FRAG_BODY_COLOR_OUT
	FragColor = vec4(c.xyz, c.w*loc3.y);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_SIMPLE = R"GLSHDR(
	//MSH_FRAG_BODY_SIMPLE
	c = loc0;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_COLOR_LIGHT = R"GLSHDR(
	//MSH_FRAG_BODY_COLOR_LIGHT
	vec4 spec = vec4(0.0);
	vec3 eye = vec3(0.0, 0.0, 1.0);
	vec3 l_dir = vec3(0.0, 0.0, 1.0);
	vec3 n = normalize(OutNormal);
	float intensity = abs(dot(n, l_dir));
	if (intensity > 0.0)
	{
		vec3 h = normalize(l_dir+eye);
		float intSpec = max(dot(h, n), 0.0);
		spec = loc2 * pow(intSpec, loc3.x);
	}
	c.xyz = max(intensity * loc1 + spec, loc0).xyz;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_TEXTURE = R"GLSHDR(
	//MSH_FRAG_BODY_TEXTURE
	c = c * texture(tex0, OutTexcoord);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_FOG_V = R"GLSHDR(
	// MSH_FRAG_BODY_FOG
	vec4 v;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_INT = R"GLSHDR(
	//MSH_FRAG_BODY_INT
	FragUint = loci0;
)GLSHDR";

inline constexpr const char* MSH_TAIL = R"GLSHDR(
}
)GLSHDR";

#endif//MESH_SHADER_CODE_H