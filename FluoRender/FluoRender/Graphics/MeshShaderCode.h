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

inline constexpr const char* MSH_FRAG_INPUTS_VPO = R"GLSHDR(
//MSH_FRAG_INPUTS_VPO
layout(location = 0) in vec3 VertexPos;
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
uniform vec4 loc8;//(int, start, end, 0.0)
uniform vec4 loc19;//(r, g, b, a) fog color
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

inline constexpr const char* MSH_FRAG_UNIFORMS_CLIP = R"GLSHDR(
//MSH_FRAG_UNIFORMS_CLIP
uniform vec4 loc10; //plane0
uniform vec4 loc11; //plane1
uniform vec4 loc12; //plane2
uniform vec4 loc13; //plane3
uniform vec4 loc14; //plane4
uniform vec4 loc15; //plane5
)GLSHDR";

inline constexpr const char* MSH_FRAG_CLIP_FUNC = R"GLSHDR(
//MSH_FRAG_CLIP_FUNC
bool mesh_clip_func(vec4 t)
{
	if (dot(t.xyz, loc10.xyz)+loc10.w < 0.0 ||
		dot(t.xyz, loc11.xyz)+loc11.w < 0.0 ||
		dot(t.xyz, loc12.xyz)+loc12.w < 0.0 ||
		dot(t.xyz, loc13.xyz)+loc13.w < 0.0 ||
		dot(t.xyz, loc14.xyz)+loc14.w < 0.0 ||
		dot(t.xyz, loc15.xyz)+loc15.w < 0.0)
		return true;
	else
		return false;
}
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

inline constexpr const char* MSH_FRAG_HEAD_CLIP_FUNC = R"GLSHDR(
	//MSH_FRAG_HEAD_CLIP_FUNC
	if (mesh_clip_func(vec4(VertexPos, 1.0)))
	{
		discard;
		return;
	}
)GLSHDR";

inline constexpr const char* MSH_FRAG_HEAD_FOG = R"GLSHDR(
	//MSH_FRAG_HEAD_FOG
	vec4 fp;
	fp.x = loc8.x;
	fp.y = loc8.y;
	fp.z = loc8.z;
	fp.w = abs(OutFogCoord.z/OutFogCoord.w);

)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_SHADING_COLOR = R"GLSHDR(
	//MSH_FRAG_BODY_SHADING_COLOR
	vec4 c = vec4(1.0);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_PLAIN_COLOR = R"GLSHDR(
	//MSH_FRAG_BODY_PLAIN_COLOR
	vec4 c = vec4(loc0.xyz, 1.0);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_COLOR_OUT = R"GLSHDR(
	// MSH_FRAG_BODY_COLOR_OUT
	FragColor = vec4(c.xyz, c.w*loc0.a);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_DEPTH_OUT = R"GLSHDR(
	// MSH_FRAG_BODY_DEPTH_OUT
	float curz = (fp.y-fp.w)/(fp.y-fp.z);
	FragDepth = curz;
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
	vec3 l_dir = normalize(vec3(loc1.z, loc1.w, 0.2));
	vec3 l_diff = normalize(vec3(-loc1.z, -loc1.w, 0.2));
	if (dot(n, l_dir) < 0.0) n = -n;

	// Lambert diffuse with frosted gradient modulation
	float lambert = max(dot(n, l_dir), 0.0);
	float front = smoothstep(0.3, 1.0, lambert);
	float back  = 1.0 - smoothstep(0.0, 0.5, lambert);
	float shade = 0.5*back + 0.7*front + 0.3;
	float frost = 1.0 + loc1.x * (1.0 - abs(dot(n, eye)));
	vec3 diffuse = loc0.rgb * shade * frost;

	// Reflection vector
	vec3 r = normalize(reflect(-eye, n));

	// Build rectangle basis around light direction
	vec3 refAxis = (abs(l_dir.z) > 0.99) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 0.0, 1.0);
	vec3 uAxis = normalize(cross(l_dir, refAxis));
	vec3 vAxis = normalize(cross(l_dir, uAxis));

	// Rectangle half-sizes
	float uSize = mix(0.6, 0.4, loc1.y);
	float vSize = mix(0.4, 0.3, loc1.y);

	// Project reflection into rectangle space
	float ru = dot(r, uAxis);
	float rv = dot(r, vAxis) - 0.65;

	// Inside check
	float inside = step(abs(ru), uSize) * step(abs(rv), vSize);

	// Edge falloff — scaled by shine factor
	float softScale = mix(0.2, 0.95, loc1.y);
	float edgeU = smoothstep(uSize*softScale, uSize, abs(ru));
	float edgeV = smoothstep(vSize*softScale, vSize, abs(rv));
	float edgeFalloff = max(edgeU, edgeV);

	// Facing term
	float facing = max(dot(r, l_dir), 0.0);

	// Center softness — also modulated by shine
	float viewAlign = abs(dot(n, eye));
	float centerSoft = mix(0.1, 1.0, 1.0 - viewAlign * loc1.y);

	// Highlight intensity
	float rectSpec = inside * (1.0 - edgeFalloff) * facing * centerSoft;
	vec3 keyHighlight = vec3(5.0) * rectSpec;

	// Diffuser highlight (opposite direction, softer)
	vec3 h_diff = normalize(l_diff + eye);
	float diffSpec = pow(abs(dot(h_diff, n)), mix(1.0, 10.0, loc1.y));
	vec3 diffHighlight = vec3(0.8) * diffSpec;

	// Combine
	c.xyz *= diffuse + loc1.x * (keyHighlight + diffHighlight);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_TEXTURE = R"GLSHDR(
	//MSH_FRAG_BODY_TEXTURE
	c *= texture(tex0, OutTexcoord);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_FOG = R"GLSHDR(
	// MSH_FRAG_BODY_FOG
	vec4 v;
	v.x = (fp.y-fp.w)/(fp.y-fp.z);
	v.x = clamp(v.x, 0.0, 1.0);
)GLSHDR";

inline constexpr const char* MSH_FRAG_BODY_FOG_MIX = R"GLSHDR(
	c.xyz = mix(c.xyz, loc19.xyz, v.x*fp.x); 
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

inline constexpr const char* MSH_GEOM_NORMALS_OUTPUTS_VPO = R"GLSHDR(
layout(location = 0) out vec3 OutVertexPos;
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

		OutVertexPos = VertexPos[i];
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