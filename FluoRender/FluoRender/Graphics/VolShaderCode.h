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

#ifndef VolShaderCode_h
#define VolShaderCode_h

#define CORE_PROFILE_VTX_SHADER 1

inline constexpr const char* VTX_SHADER_CODE_CORE_PROFILE = R"GLSHDR(
//VTX_SHADER_CODE_CORE_PROFILE
uniform mat4 matrix0; //projection matrix
uniform mat4 matrix1; //modelview matrix
layout(location = 0) in vec3 InVertex;  //w will be set to 1.0 automatically
layout(location = 1) in vec3 InTexture;
layout(location = 0) out vec3 OutVertex;
layout(location = 1) out vec3 OutTexture;
//-------------------
void main()
{
	gl_Position = matrix0 * matrix1 * vec4(InVertex,1.);
	OutTexture = InTexture;
	OutVertex  = InVertex;
}
)GLSHDR";

inline constexpr const char* VTX_SHADER_CODE_FOG  = R"GLSHDR(
//VTX_SHADER_CODE_FOG
uniform mat4 matrix0; //projection matrix
uniform mat4 matrix1; //modelview matrix
layout(location = 0) in vec3 InVertex;  //w will be set to 1.0 automatically
layout(location = 1) in vec3 InTexture;
layout(location = 0) out vec3 OutVertex;
layout(location = 1) out vec3 OutTexture;
layout(location = 2) out vec4 OutFogCoord;
//-------------------
void main()
{
	OutFogCoord = matrix1 * vec4(InVertex,1.);
	gl_Position = matrix0 * OutFogCoord;
	OutTexture = InTexture;
	OutVertex  = InVertex;
}
)GLSHDR";

inline constexpr const char* FRG_SHADER_CODE_CORE_PROFILE  = R"GLSHDR(
//FRG_SHADER_CODE_CORE_PROFILE
layout(location = 0) in vec3 OutVertex;
layout(location = 1) in vec3 OutTexCoord;
layout(location = 0) out vec4 FragColor;
	
uniform vec4 loc0;//color
void main()
{
	FragColor = loc0;
}
)GLSHDR";

inline constexpr const char* DEFAULT_FRAGMENT_CODE  = R"GLSHDR(
void main() {
	FragColor = vec4(1,1,1,1);
}
)GLSHDR";

inline constexpr const char* VOL_INPUTS  = R"GLSHDR(
layout(location = 0) in vec3 OutVertex;
layout(location = 1) in vec3 OutTexture;
)GLSHDR";

inline constexpr const char* VOL_INPUTS_FOG  = R"GLSHDR(
layout(location = 2) in vec4 OutFogCoord;
)GLSHDR";

inline constexpr const char* VOL_OUTPUTS_COLOR  = R"GLSHDR(
layout(location = 0) out vec4 FragColor;
)GLSHDR";

inline constexpr const char* VOL_OUTPUTS_DEPTH = R"GLSHDR(
layout(location = 1) out vec2 FragDepth;
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_COMMON  = R"GLSHDR(
// VOL_UNIFORMS_COMMON
uniform vec4 loc0;//(lx, ly, lz, 0)
uniform vec4 loc1;//(strength, shine, dir_y, dir_x)
uniform vec4 loc2;//(scalar_scale, gm_scale, left_thresh, right_thresh)
uniform vec4 loc3;//(gamma, left_offset, right_offset, sw)
uniform vec4 loc4;//(1/nx, 1/ny, 1/nz, 1/sample_rate)
uniform vec4 loc5;//(spcx, spcy, spcz, shuffle)
uniform vec4 loc6;//(low, hi, hi-lo, inv)
uniform vec4 loc7;//(1/vx, 1/vy, 0, 0) 2dmap loc
uniform vec4 loc8;//(int, start, end, 0.0) fog loc
uniform vec4 loc9;//(primary red, green, blue, 0)
uniform vec4 loc16;//(secondary red, green, blue, mask_threshold)
uniform vec4 loc17;//(gm_low, gm_high, gm_max, 0)
uniform vec4 loc18;//(alpha, alpha_power, luminance, 0)
uniform vec4 loc19;//(fog/background color)

uniform sampler3D tex0;//data volume

uniform mat4 matrix2;//texture
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_CLIP  = R"GLSHDR(
//VOL_UNIFORMS_CLIP
uniform vec4 loc10; //plane0
uniform vec4 loc11; //plane1
uniform vec4 loc12; //plane2
uniform vec4 loc13; //plane3
uniform vec4 loc14; //plane4
uniform vec4 loc15; //plane5
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_4D_CACHE  = R"GLSHDR(
// VOL_UNIFORMS_4D_CACHE
uniform sampler3D tex10;//data from t-1
)GLSHDR";

inline constexpr const char* VOL_UNIFROMS_4D_COLORMAP  = R"GLSHDR(
//VOL_UNIFROMS_4D_COLORMAP
uniform uint loci0;//time
uniform uint loci1;//time length
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_RADIAL_GRADIENT = R"GLSHDR(
//VOL_UNIFORMS_RADIAL_GRADIENT
uniform vec4 loc22; //(center x, y, z, radius)
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_LINEAR_GRADIENT = R"GLSHDR(
//VOL_UNIFORMS_LINEAR_GRADIENT
uniform vec4 loc22; //(start plane x, y, z, d) normalized to texture coords
uniform vec4 loc23; //(end plane x, y, z, d) normalized to texture coords
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_DP  = R"GLSHDR(
//VOL_UNIFORMS_DP
uniform sampler2D tex14;//depth texture 1
uniform sampler2D tex15;//depth texture 2
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_MASK  = R"GLSHDR(
//VOL_UNIFORMS_MASK
uniform sampler3D tex2;//3d mask volume
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_LABEL  = R"GLSHDR(
//VOL_UNIFORMS_LABEL
uniform usampler3D tex3;//3d label volume
)GLSHDR";

inline constexpr const char* VOL_UNIFORMS_DEPTHMAP  = R"GLSHDR(
//VOL_UNIFORMS_DEPTHMAP
uniform sampler2D tex4;//2d depth map
)GLSHDR";

inline constexpr const char* VOL_HEAD  = R"GLSHDR(
//VOL_HEAD
void main()
{
	vec3 texCoord = OutTexture;
)GLSHDR";

inline constexpr const char* VOL_HEAD_2DMAP_LOC  = R"GLSHDR(
	//VOL_HEAD_2DMAP_LOC
	vec2 fcf = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);
)GLSHDR";

inline constexpr const char* VOL_HEAD_DP_1  = R"GLSHDR(
	//VOL_HEAD_DP_NEG
	if (texture(tex15, fcf).r < gl_FragCoord.z) discard;
)GLSHDR";

inline constexpr const char* VOL_HEAD_DP_2  = R"GLSHDR(
	//VOL_HEAD_DP_POS
	if (texture(tex15, fcf).r > gl_FragCoord.z) discard;
)GLSHDR";

inline constexpr const char* VOL_HEAD_DP_3  = R"GLSHDR(
	//VOL_HEAD_DP_BOTH
	if (texture(tex15, fcf).r < gl_FragCoord.z) discard;
	else if (texture(tex14, fcf).r > gl_FragCoord.z) discard;
)GLSHDR";

inline constexpr const char* VOL_HEAD_FOG  = R"GLSHDR(
	//VOL_HEAD_FOG
	vec4 fp;
	fp.x = loc8.x;
	fp.y = loc8.y;
	fp.z = loc8.z;
	fp.w = abs(OutFogCoord.z/OutFogCoord.w);
)GLSHDR";

inline constexpr const char* VOL_HEAD_CLIP  = R"GLSHDR(
	//VOL_HEAD_CLIP
	vec4 brickt = matrix2 * vec4(texCoord, 1.0);
	if (dot(brickt.xyz, loc10.xyz)+loc10.w < 0.0 ||
		dot(brickt.xyz, loc11.xyz)+loc11.w < 0.0 ||
		dot(brickt.xyz, loc12.xyz)+loc12.w < 0.0 ||
		dot(brickt.xyz, loc13.xyz)+loc13.w < 0.0 ||
		dot(brickt.xyz, loc14.xyz)+loc14.w < 0.0 ||
		dot(brickt.xyz, loc15.xyz)+loc15.w < 0.0)
	{
		discard;//FragColor = vec4(0.0);
		return;
	}
)GLSHDR";

inline constexpr const char* VOL_HEAD_CLIP_FUNC  = R"GLSHDR(
	//VOL_HEAD_CLIP_FUNC
	if (vol_clip_func(vec4(texCoord, 1.0)))
	{
		discard;
		return;
	}
)GLSHDR";

inline constexpr const char* VOL_CLIP_FUNC  = R"GLSHDR(
//VOL_CLIP_FUNC
bool vol_clip_func(vec4 t)
{
	vec4 brickt = matrix2 * t;
	if (dot(brickt.xyz, loc10.xyz)+loc10.w < 0.0 ||
		dot(brickt.xyz, loc11.xyz)+loc11.w < 0.0 ||
		dot(brickt.xyz, loc12.xyz)+loc12.w < 0.0 ||
		dot(brickt.xyz, loc13.xyz)+loc13.w < 0.0 ||
		dot(brickt.xyz, loc14.xyz)+loc14.w < 0.0 ||
		dot(brickt.xyz, loc15.xyz)+loc15.w < 0.0)
		return true;
	else
		return false;
}
)GLSHDR";

inline constexpr const char* VOL_HEAD_LIT  = R"GLSHDR(
	//VOL_HEAD_LIT
	vec3 eye = loc0.xyz; // {lx, ly, lz, 0}
	vec3 grad;
)GLSHDR";

inline constexpr const char* VOL_TAIL  = R"GLSHDR(
//VOL_TAIL
}
)GLSHDR";

inline constexpr const char* VOL_DATA_VOLUME_LOOKUP  = R"GLSHDR(
	//VOL_DATA_VOLUME_LOOKUP
	vec4 v = texture(tex0, texCoord);
)GLSHDR";

inline constexpr const char* VOL_NO_MASK = R"GLSHDR(
	//VOL_NO_MASK
	vec4 m = vec4(0.0);
)GLSHDR";

inline constexpr const char* VOL_MASK_LOOKUP = R"GLSHDR(
	//VOL_MASK_LOOKUP
	vec4 m = texture(tex2, texCoord);
)GLSHDR";

inline constexpr const char* VOL_GRAD_COMPUTE  = R"GLSHDR(
	// VOL_GRAD_COMPUTE
	vec3 dir = loc4.xyz; // (1/nx, 1/ny, 1/nz)

	// Sample neighbors in +x and -x
	float xp = texture(tex0, clamp(texCoord + vec3(dir.x, 0.0, 0.0), 0.0, 1.0)).r;
	float xm = texture(tex0, clamp(texCoord - vec3(dir.x, 0.0, 0.0), 0.0, 1.0)).r;
	grad.x = (xp - xm) * 0.5;

	// Sample neighbors in +y and -y
	float yp = texture(tex0, clamp(texCoord + vec3(0.0, dir.y, 0.0), 0.0, 1.0)).r;
	float ym = texture(tex0, clamp(texCoord - vec3(0.0, dir.y, 0.0), 0.0, 1.0)).r;
	grad.y = (yp - ym) * 0.5;

	// Sample neighbors in +z and -z
	float zp = texture(tex0, clamp(texCoord + vec3(0.0, 0.0, dir.z), 0.0, 1.0)).r;
	float zm = texture(tex0, clamp(texCoord - vec3(0.0, 0.0, dir.z), 0.0, 1.0)).r;
	grad.z = (zp - zm) * 0.5;

	// Gradient magnitude
	grad /= loc5.xyz;
	float gradMag = length(grad);

	// Apply scaling
	gradMag = 0.5 * (loc2.x < 0.0 ? (1.0 + gradMag * loc2.x) : gradMag * loc2.x);
)GLSHDR";

inline constexpr const char* VOL_GRAD_COMPUTE_FUNC  = R"GLSHDR(
// VOL_GRAD_COMPUTE_FUNC
vec4 vol_grad_func(vec4 pos, vec4 dir)
{
	vec4 r, p;
	vec4 n = vec4(0.0);
	vec4 w = vec4(0.0);
	w.x = dir.x;
	p = clamp(pos+w, 0.0, 1.0);
	r = texture(tex0, p.stp);
	n.x = r.x + n.x;
	p = clamp(pos-w, 0.0, 1.0);
	r = texture(tex0, p.stp);
	n.x = r.x - n.x;
	w = vec4(0.0);
	w.y = dir.y;
	p = clamp(pos+w, 0.0, 1.0);
	r = texture(tex0, p.stp);
	n.y = r.x + n.y;
	p = clamp(pos-w, 0.0, 1.0);
	r = texture(tex0, p.stp);
	n.y = r.x - n.y;
	w = vec4(0.0);
	w.z = dir.x<dir.z?dir.x:dir.z;
	p = clamp(pos+w, 0.0, 1.0);
	r = texture(tex0, p.stp);
	n.z = r.x + n.z;
	p = clamp(pos-w, 0.0, 1.0);
	r = texture(tex0, p.stp);
	n.z = 0.3 * (r.x - n.z);
	return n;
}
)GLSHDR";

inline constexpr const char* VOL_BODY_SHADING  = R"GLSHDR(
	//VOL_BODY_SHADING
	float gradLen = length(grad);
	grad = (gradLen > 1e-6) ? grad / gradLen : vec3(0.0);

	// Build perturbation basis
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 side = normalize(cross(eye, up));
	vec3 up2  = normalize(cross(side, eye));

	// Key light direction
	vec3 l_dir = normalize(eye + side * loc1.z + up2 * loc1.w + eye * 0.2);
	float lambert = max(dot(grad, l_dir), 0.0);

	// Lambert diffuse with frosted gradient modulation
	float front = smoothstep(0.3, 1.0, lambert);
	float back  = 1.0 - smoothstep(0.0, 0.5, lambert);
	float shade = 0.5*back + 0.7*front + 0.3;
	float frost = smoothstep(0.0, 0.1, gradMag);
	float diffuse = shade + frost;

	// Key light highlight (sharp, white)
	vec3 l_high = normalize(eye - side * loc1.z - up2 * loc1.w + eye * 5.0);
	vec3 h = normalize(l_high + eye);
	float keyHighlight = 6.0 * pow(smoothstep(0.9, 1.0, abs(dot(h, grad))), mix(10.0, 100.0, loc1.y));

	// Diffuser highlight (opposite direction, softer)
	vec3 l_diff = normalize(eye + side * loc1.z + up2 * loc1.w + eye * 3.0);
	vec3 h_diff = normalize(l_diff + eye);
	float diffHighlight = 3.0 * pow(abs(dot(h_diff, grad)), mix(1.0, 10.0, loc1.y));

	//composition
	float all_light = mix(1.0, (diffuse + keyHighlight + diffHighlight) * 0.5, loc1.x);
)GLSHDR";

inline constexpr const char* VOL_COMPUTED_GM_LOOKUP  = R"GLSHDR(
	//VOL_COMPUTED_GM_LOOKUP
	v.y = gradMag;
)GLSHDR";

inline constexpr const char* VOL_COMPUTED_GM_NOUSE  = R"GLSHDR(
	//VOL_COMPUTED_GM_NOUSE
	v.y = 0.0;
)GLSHDR";

inline constexpr const char* VOL_COMPUTED_GM_INVALIDATE  = R"GLSHDR(
	//VOL_COMPUTED_GM_INVALIDATE
	v.y = loc2.y;
)GLSHDR";

inline constexpr const char* VOL_DATA_4D_INTENSITY_DELTA  = R"GLSHDR(
	//VOL_DATA_4D_INTENSITY_DELTA
	float v4d = v.x - texture(tex10, texCoord).x;
)GLSHDR";

inline constexpr const char* VOL_DATA_4D_SPEED  = R"GLSHDR(
	//VOL_DATA_4D_SPEED
	float grad_t = v.x - texture(tex10, texCoord).x;
	vec3 grad_s = grad * loc5.xyz;
	float denom = dot(grad_s, grad_s) + 1e-4;
	grad_s = -grad_s * grad_t / denom;
	float v4d = length(grad_s);
)GLSHDR";

//output raw c, tf_val, and alpha
inline constexpr const char* VOL_TRANSFER_FUNCTION  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION
	vec4 c = vec4(0.0);
	float tf_val = 0.0;
	float alpha = 0.0;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x >= loc2.z - loc3.w && (loc2.w >= 1.0 || v.x <= loc2.w + loc3.w))
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
		alpha = pow(tf_val, loc18.y);
	}
)GLSHDR";

inline constexpr const char* VOL_OUT_COLOR_DECLARE = R"GLSHDR(
	//VOL_OUT_COLOR_DECLARE
	vec3 out_color = vec3(0.0);
)GLSHDR";

inline constexpr const char* VOL_OUT_COLORMAP_VALUE_DECLARE = R"GLSHDR(
	//VOL_OUT_COLORMAP_VALUE_DECLARE
	float cm_value = 0.0;
)GLSHDR";

inline constexpr const char* VOL_COLORMAP_COLOR_DECLARE = R"GLSHDR(
	//VOL_COLORMAP_COLOR_DECLARE
	vec3 rb = vec3(0.0);
)GLSHDR";

inline constexpr const char* VOL_OUT_COLOR_SINGLE_COLOR_MAIN = R"GLSHDR(
	//VOL_OUT_COLOR_SINGLE_COLOR_MAIN
	out_color = loc9.rgb;
)GLSHDR";

inline constexpr const char* VOL_OUT_COLOR_SINGLE_COLOR_ALT = R"GLSHDR(
	//VOL_OUT_COLOR_SINGLE_COLOR_ALT
	out_color = loc16.rgb;
)GLSHDR";

inline constexpr const char* VOL_OUT_COLOR_COLORMAP = R"GLSHDR(
	//VOL_OUT_COLOR_COLORMAP
	out_color = rb;
)GLSHDR";

inline constexpr const char* VOL_OUT_COLOR_NO_COMP = R"GLSHDR(
	//VOL_OUT_COLOR_NO_COMP
	out_color = vec3(0.2, 0.4, 0.4);
)GLSHDR";

inline constexpr const char* VOL_OUT_COLOR_COMPONENT = R"GLSHDR(
	//VOL_OUT_COLOR_COMPONENT
	uint comp = texture(tex3, texCoord).x;
	out_color = vec3(0.2, 0.4, 0.4);
	float hue, p2, p3;
	if (comp > uint(0))
	{
		uint cv = comp % uint(0xfd);
		uint si = uint(loc5.w);
		cv = (cv << si) & 0xff | (cv >> (8 - si));
		hue = float(cv)/45.0;
		p2 = 1.0 - hue + floor(hue);
		p3 = hue - floor(hue);
		if (hue < 1.0)
			out_color = vec3(1.0, p3, 0.0);
		else if (hue < 2.0)
			out_color = vec3(p2, 1.0, 0.0);
		else if (hue < 3.0)
			out_color = vec3(0.0, 1.0, p3);
		else if (hue < 4.0)
			out_color = vec3(0.0, p2, 1.0);
		else if (hue < 5.0)
			out_color = vec3(p3, 0.0, 1.0);
		else
			out_color = vec3(1.0, 0.0, p2);
	}
)GLSHDR";

inline constexpr const char* VOL_COLOR_NONE = R"GLSHDR(
	//VOL_COLOR_NONE
	c = vec4(0.0);
	tf_val = 0.0;
	alpha = 0.0;
)GLSHDR";

inline constexpr const char* VOL_SINGLE_COLOR_ALPHA = R"GLSHDR(
	//VOL_SINGLE_COLOR_ALPHA
	if (alpha > 0.0)
	{
		float pa = 1.0 - pow(1.0-alpha, loc4.w);
		tf_val = loc18.z*pa*(loc18.y>1.1?1.0:tf_val);
		c = vec4(out_color*tf_val, pa);
	}
)GLSHDR";

inline constexpr const char* VOL_SINGLE_COLOR_SOLID  = R"GLSHDR(
	//VOL_SINGLE_COLOR_SOLID
	if (alpha > 0.0)
	{
		tf_val = loc18.z*tf_val;
		c = vec4(out_color*tf_val, 1.0);
	}
	else
		c.a = 1.0;
)GLSHDR";

inline constexpr const char* VOL_COLOR_MODE_MASK_IF = R"GLSHDR(
	//VOL_COLOR_MODE_MASK_IF
	if (m.x > 0.0)
	{
)GLSHDR";

inline constexpr const char* VOL_COLOR_MODE_MASK_ELSE = R"GLSHDR(
	//VOL_COLOR_MODE_MASK_ELSE
	}
	else
	{
)GLSHDR";

inline constexpr const char* VOL_SHADER_MODE_MASK_END = R"GLSHDR(
	//VOL_SHADER_MODE_MASK_END
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_SIN_COLOR_L  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_SIN_COLOR_L
	vec4 c;
	float tf_val = 0.0;
	float alpha = 0.0;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))
		c = vec4(0.0);
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
		alpha = pow(tf_val, loc18.y);
		c = vec4(tf_val);
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC  = R"GLSHDR(
//VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC
vec4 vol_trans_sin_color_l(vec4 v)
{
	vec4 c;
	float tf_val;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))
		c = vec4(0.0);
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
		c = vec4(tf_val);
	}
	return c;
}
)GLSHDR";

inline constexpr const char* VOL_COMMON_TRANSFER_FUNCTION_CALC  = R"GLSHDR(
		//VOL_COMMON_TRANSFER_FUNCTION_CALC
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
)GLSHDR";

//rainbow
inline constexpr const char* VOL_COLORMAP_CALC0  = R"GLSHDR(
		//VOL_COLORMAP_CALC0
		rb.r = clamp((4.0*cm_value - 2.0)*loc6.w, 0.0, 1.0);
		rb.g = clamp(cm_value<0.5 ? 4.0*cm_value : -4.0*cm_value+4.0, 0.0, 1.0);
		rb.b = clamp((2.0 - 4.0*cm_value)*loc6.w, 0.0, 1.0);
)GLSHDR";

inline constexpr const char* VOL_COLORMAP_DIFF_CALC0  = R"GLSHDR(
		//VOL_COLORMAP_DIFF_CALC0
		rb.rgb = clamp(grad*loc6.w, -1.0, 1.0) + vec3(1.0);
)GLSHDR";

//primary-secondary
inline constexpr const char* VOL_COLORMAP_CALC1  = R"GLSHDR(
		//VOL_COLORMAP_CALC1
		rb.rgb = mix(loc6.w>0.0?loc16.rgb:loc9.rgb, loc6.w>0.0?loc9.rgb:loc16.rgb, clamp(cm_value, 0.0, 1.0));
)GLSHDR";

//hot
inline constexpr const char* VOL_COLORMAP_CALC2  = R"GLSHDR(
		//VOL_COLORMAP_CALC2
		rb.r = clamp(loc6.w*2.0*cm_value+(loc6.w>0.0?0.0:2.0), 0.0, 1.0);
		rb.g = clamp(loc6.w*(4.0*cm_value - 2.0), 0.0, 1.0);
		rb.b = clamp(loc6.w*4.0*cm_value+(loc6.w>0.0?-3.0:1.0), 0.0, 1.0);
)GLSHDR";

//cool
inline constexpr const char* VOL_COLORMAP_CALC3  = R"GLSHDR(
		//VOL_COLORMAP_CALC3
		rb.r = clamp(loc6.w>0.0?cm_value:(1.0-cm_value), 0.0, 1.0);
		rb.g = clamp(loc6.w>0.0?(1.0-cm_value):cm_value, 0.0, 1.0);
		rb.b = 1.0;
)GLSHDR";

//diverging
inline constexpr const char* VOL_COLORMAP_CALC4  = R"GLSHDR(
		//VOL_COLORMAP_CALC4
		rb.r = clamp(loc6.w>0.0?(cm_value<0.5?cm_value*0.9+0.25:0.7):(cm_value<0.5?0.7:-0.9*cm_value+1.15), 0.0, 1.0);
		rb.g = clamp(loc6.w>0.0?(cm_value<0.5?cm_value*0.8+0.3:1.4-1.4*cm_value):(cm_value<0.5?1.4*cm_value:-0.8*cm_value+1.1), 0.0, 1.0);
		rb.b = clamp(loc6.w>0.0?(cm_value<0.5?-0.1*cm_value+0.75:-1.1*cm_value+1.25):(cm_value<0.5?1.1*cm_value+0.15:0.1*cm_value+0.65), 0.0, 1.0);
)GLSHDR";

//monochrome
inline constexpr const char* VOL_COLORMAP_CALC5  = R"GLSHDR(
		//VOL_COLORMAP_CALC5
		rb.rgb = vec3((loc6.w>0.0?0.0:1.0) + loc6.w*clamp(cm_value, 0.0, 1.0));
)GLSHDR";

//high-key
inline constexpr const char* VOL_COLORMAP_CALC6  = R"GLSHDR(
		//VOL_COLORMAP_CALC6
		rb.rgb = mix(loc6.w>0.0?vec3(1.0):loc9.rgb, loc6.w>0.0?loc9.rgb:vec3(1.0), clamp(cm_value, 0.0, 1.0));
)GLSHDR";

//low-key
inline constexpr const char* VOL_COLORMAP_CALC7  = R"GLSHDR(
		//VOL_COLORMAP_CALC7
		rb.rgb = mix(loc6.w>0.0?loc9.rgb:loc9.rgb*0.1, loc6.w>0.0?loc9.rgb*0.1:loc9.rgb, clamp(cm_value, 0.0, 1.0));
)GLSHDR";

//increased transp
inline constexpr const char* VOL_COLORMAP_CALC8  = R"GLSHDR(
		//VOL_COLORMAP_CALC8
		rb.rgb = mix(loc6.w>0.0?vec3(0.0):loc9.rgb, loc6.w>0.0?loc9.rgb:vec3(0.0), clamp(cm_value, 0.0, 1.0));
)GLSHDR";

//intensity
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP0  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP0_INTENSITY
		cm_value = (tf_val-loc6.x)/loc6.z;
)GLSHDR";

//value z
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP1  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP1_VALUE_Z
		vec4 tt = matrix2 * vec4(texCoord, 1.0);
		cm_value = (tt.z-loc6.x)/loc6.z;
)GLSHDR";

//value y
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP2  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP2_VALUE_Y
		vec4 tt = matrix2 * vec4(texCoord, 1.0);
		cm_value = (tt.y-loc6.x)/loc6.z;
)GLSHDR";

//value x
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP3  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP3_VALUE_X
		vec4 tt = matrix2 * vec4(texCoord, 1.0);
		cm_value = (tt.x-loc6.x)/loc6.z;
)GLSHDR";

//t value
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP4  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP4_VALUE_T
		cm_value = (loci1 != 0u) ? float(loci0) / float(loci1) : 0.0;
		cm_value = (cm_value-loc6.x)/loc6.z;
)GLSHDR";

//radial gradient
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP5 = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP5_RADIAL_GRADIENT
		vec4 tt = matrix2 * vec4(texCoord, 1.0);
		tt.xyz *= loc5.xyz / loc4.xyz;
		cm_value = 1.0 - length(tt.xyz - loc22.xyz) / loc22.w;
		cm_value = (cm_value-loc6.x)/loc6.z;
)GLSHDR";

//linear gradient
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP6 = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP6_LINEAR_GRADIENT
		vec4 tt = matrix2 * vec4(texCoord, 1.0);
		tt.xyz *= loc5.xyz / loc4.xyz;
		float d0 = dot(loc22.xyz, tt.xyz) + loc22.w;
		float d1 = dot(loc23.xyz, tt.xyz) + loc23.w;
		cm_value = d0 / (d0 - d1);
		cm_value = (cm_value-loc6.x)/loc6.z;
)GLSHDR";

//gradient magnitude
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP7  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP7_VALUE_GRADIENT_MAGNITUDE
		cm_value = (v.y-loc6.x)/loc6.z;
)GLSHDR";

//normal
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP8  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP8_VALUE_NORMAL
		cm_value = dot(clamp(grad, -1.0, 1.0), eye.xyz/*vec3(1.0, 1.0, 0.0)*/);
		cm_value = cm_value + 1.0;
		cm_value = (cm_value-loc6.x)/loc6.z;
)GLSHDR";

//delta intensity
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP9  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP9_VALUE_DELTA_INTENSITY
		cm_value = v4d;
		float exponent = max(loc6.z / 10.0, 0.01);
		cm_value = cm_value < 0.0 ? -pow(-cm_value, exponent) : pow(cm_value, exponent);
		cm_value = (cm_value + 1.0) / 2.0;
		cm_value = (cm_value-loc6.x)/loc6.z;
)GLSHDR";

//speed
inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP10  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP10_VALUE_SPEED
		cm_value = v4d;
		float exponent = max(loc6.z, 0.01);
		cm_value = pow(cm_value, exponent);
		cm_value = (cm_value-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_ENCODE = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_ENCODE
	float l1 = tf_val * 256.3;
	float l2 = tf_val * 1053.1;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_TRANSP = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_TRANSP
	c = vec4(l1 + cm_value, l2 + cm_value, tf_val, tf_val);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_SOLID  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_SOLID
	c = vec4(l1 + cm_value, l2 + cm_value, tf_val, 1.0);
)GLSHDR";

inline constexpr const char* VOL_SHADING_OUTPUT  = R"GLSHDR(
	//VOL_SHADING_OUTPUT
	c.xyz *= all_light;
)GLSHDR";

inline constexpr const char* VOL_FOG_BODY  = R"GLSHDR(
	//VOL_FOG_BODY
	v.x = (fp.y-fp.w)/(fp.y-fp.z);
	v.x = clamp(v.x, 0.0, 1.0);
	c.xyz = mix(c.xyz, loc19.xyz*tf_val, v.x*fp.x);
	if (any(greaterThan(loc19.xyz, vec3(0.5))))
		c.a = mix(c.a, 0.0, v.x*fp.x);
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND  = R"GLSHDR(
	//VOL_RASTER_BLEND
	FragColor = c*loc18.x; // VOL_RASTER_BLEND
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_SOLID  = R"GLSHDR(
	//VOL_RASTER_BLEND_SOLID
	FragColor = c;
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_DMAP  = R"GLSHDR(
	//VOL_RASTER_BLEND_DMAP
	float curz = (fp.y-fp.w)/(fp.y-fp.z);
	float w_a = alpha * loc18.x;
	float w_t = 0.3 * loc3.z;
	w_a = w_a > w_t ? w_a : w_t * smoothstep(0.1 * w_t, w_t, w_a);
	float w_d = pow(1.0 - curz, 3.0);
	FragDepth = vec2(curz, 1.0) * w_d * w_a;
)GLSHDR";

#endif//VolShaderCode_h
