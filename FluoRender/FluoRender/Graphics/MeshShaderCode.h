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

inline constexpr const char* MSH_VERTEX_INPUTS_T = R"GLSHDR(
//MSH_VERTEX_INPUTS_T
layout(location = 2) in vec2 InTexcoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_INPUTS_C = R"GLSHDR(
//MSH_VERTEX_INPUTS_C
layout(location = 3) in vec4 InColor;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_VPOS = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_VPOS
layout(location = 0) out vec3 VertexPos;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_N = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_N
layout(location = 1) out vec3 OutNormal;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_T = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_T
layout(location = 2) out vec2 OutTexcoord;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_C = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_C
layout(location = 3) out vec4 OutColor;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_OUTPUTS_FOG = R"GLSHDR(
//MSH_VERTEX_OUTPUTS_FOG
layout(location = 4) out vec4 OutFogCoord;
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

inline constexpr const char* MSH_VERTEX_BODY_VPOS = R"GLSHDR(
	//MSH_VERTEX_BODY_VPOS
	VertexPos = InVertex;
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

inline constexpr const char* MSH_VERTEX_BODY_COLOR = R"GLSHDR(
	//MSH_VERTEX_BODY_COLOR
	OutColor = InColor;
)GLSHDR";

inline constexpr const char* MSH_VERTEX_BODY_FOG = R"GLSHDR(
	//MSH_VERTEX_BODY_FOG
	OutFogCoord = matrix1 * vec4(InVertex,1.);
)GLSHDR";

inline constexpr const char* MSH_FRAG_OUTPUTS = R"GLSHDR(
//MSH_FRAG_OUTPUTS
layout(location = 0) out vec4 FragColor;
)GLSHDR";

inline constexpr const char* MSH_FRAG_OUTPUTS_DEPTH = R"GLSHDR(
//MSH_FRAG_OUTPUTS_DEPTH
layout(location = 1) out float FragDepth;
)GLSHDR";

inline constexpr const char* MSH_FRAG_OUTPUTS_INT = R"GLSHDR(
//MSH_FRAG_OUTPUTS_INT
layout(location = 0) out uint FragUint;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_N = R"GLSHDR(
//MSH_FRAG_INPUTS_N
layout(location = 1) in vec3 OutNormal;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_T = R"GLSHDR(
//MSH_FRAG_INPUTS_T
layout(location = 2) in vec2 OutTexcoord;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_C = R"GLSHDR(
//MSH_FRAG_INPUTS_C
layout(location = 3) in vec4 OutColor;
)GLSHDR";

inline constexpr const char* MSH_FRAG_INPUTS_FOG = R"GLSHDR(
//MSH_FRAG_INPUTS_FOG
layout(location = 4) in vec4 OutFogCoord;
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_SHADING = R"GLSHDR(
//MSH_FRAG_UNIFORMS_SHADING
uniform vec4 loc0;//(r, g, b, a) base color
uniform vec4 loc1;//(strength, shine, dirx, diry)
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_TEX = R"GLSHDR(
// MSH_FRAG_UNIFORMS_TEX
uniform sampler2D tex0;
)GLSHDR";

inline constexpr const char* MSH_FRAG_UNIFORMS_FOG = R"GLSHDR(
// MSH_FRAG_UNIFORMS_FOG
uniform vec4 loc8;//(int, start, end, 0.0) fog loc
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
	FragColor = vec4(c.xyz, c.w*loc0.a);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_DEPTH_OUT = R"GLSHDR(
	// MSH_FRAG_BODY_DEPTH_OUT
	float curz = (fp.y-fp.w)/(fp.y-fp.z);
	//curz = clamp(curz, 0.0, 1.0);
	FragDepth = curz;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_SIMPLE = R"GLSHDR(
	//MSH_FRAG_BODY_SIMPLE
	c = loc0;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_VERTEX_COLOR = R"GLSHDR(
	//MSH_FRAG_BODY_VERTEX_COLOR
	c *= OutColor;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_SHADING = R"GLSHDR(
	//MSH_FRAG_BODY_SHADING
	vec3 n = normalize(OutNormal);
	vec3 eye = vec3(0.0, 0.0, 1.0);

	// Key light direction
	vec3 l_dir = normalize(vec3(loc1.z, loc1.w, 1.0));
	vec3 l_diff = normalize(vec3(-loc1.z, -loc1.w, 0.2));
	if (dot(n, l_dir) < 0.0) n = -n;

	// Lambert diffuse with frosted gradient modulation
	float lambert = max(dot(n, l_dir), 0.0);
	float front = smoothstep(0.3, 1.0, lambert);
	float back  = 1.0 - smoothstep(0.0, 0.3, lambert);
	float shade = 0.5*back + 0.2*(1.0-front) + front;
	float frost = mix(0.7, 1.3, 1.0 - abs(dot(n, eye)));
	vec3 diffuse = loc0.rgb * shade * frost;

	// Key light highlight (sharp, white)
	vec3 h = normalize(l_dir + eye);
	float keySpec = pow(max(dot(h, n), 0.0), loc1.y * 20.0);
	vec3 keyHighlight = vec3(1.0) * keySpec;

	// Diffuser highlight (opposite direction, softer)
	vec3 h_diff = normalize(l_diff + eye);
	float diffSpec = pow(abs(dot(h_diff, n)), mix(1.0, 10.0, loc1.y));
	vec3 diffHighlight = vec3(0.8) * diffSpec;

	// Combine
	c.xyz *= diffuse + 0.0 * keyHighlight + loc1.x * diffHighlight;
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_TEXTURE = R"GLSHDR(
	//MSH_FRAG_BODY_TEXTURE
	c *= texture(tex0, OutTexcoord);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_FOG = R"GLSHDR(
	// MSH_FRAG_BODY_FOG
	vec4 v;
	v.x = (fp.z-fp.w)/(fp.z-fp.y);
	v.x = 1.0-clamp(v.x, 0.0, 1.0);
	v.x = 1.0-exp(-pow(v.x*2.5, 2.0));
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_INT = R"GLSHDR(
	//MSH_FRAG_BODY_INT
	FragUint = loci0;
)GLSHDR";

inline constexpr const char* MSH_TAIL = R"GLSHDR(
}
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_INPUTS = R"GLSHDR(
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_INPUTS_VPOS = R"GLSHDR(
// receive from vertex shader
layout(location = 0) in vec3 VertexPos[];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_INPUTS_T = R"GLSHDR(
layout(location = 2) in vec2 InTexcoord[];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_INPUTS_C = R"GLSHDR(
layout(location = 3) in vec4 InColor[];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_INPUTS_FOG = R"GLSHDR(
layout(location = 4) in vec4 InFogCoord[];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_OUTPUTS_N = R"GLSHDR(
layout(location = 1) out vec3 OutNormal;
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_OUTPUTS_T = R"GLSHDR(
layout(location = 2) out vec2 OutTexcoord;
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_OUTPUTS_C = R"GLSHDR(
layout(location = 3) out vec4 OutColor;
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_OUTPUTS_FOG = R"GLSHDR(
layout(location = 4) out vec4 OutFogCoord;
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_HEAD = R"GLSHDR(
void main()
{
	vec3 v0 = VertexPos[0];
	vec3 v1 = VertexPos[1];
	vec3 v2 = VertexPos[2];

	vec3 faceNormal = normalize(cross(v1 - v0, v2 - v0));

	// Transform to eye space
	vec3 transformedNormal = normalize((matrix2 * vec4(faceNormal, 0.0)).xyz);

	for (int i = 0; i < 3; ++i)
	{
		gl_Position = gl_in[i].gl_Position;

		// Use computed face normal
		OutNormal = transformedNormal;
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_BODY_T = R"GLSHDR(
		OutTexcoord = InTexcoord[i];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_BODY_C = R"GLSHDR(
		OutColor = InColor[i];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_BODY_FOG = R"GLSHDR(
		OutFogCoord = InFogCoord[i];
)GLSHDR";

inline constexpr const char* MSH_GEOM_NORMALS_TAIL = R"GLSHDR(
		EmitVertex();
	}
	EndPrimitive();
}
)GLSHDR";

#endif//MESH_SHADER_CODE_H