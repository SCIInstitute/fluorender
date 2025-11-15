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
uniform vec4 loc1;//(ka, kd, ks, ns)
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
	
uniform sampler3D tex0;//data volume
uniform sampler3D tex1;//gm volume
	
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
	vec4 TexCoord = vec4(OutTexture, 1.0);
	vec4 t = TexCoord;
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
	vec4 brickt = matrix2 * t;
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
	if (vol_clip_func(t))
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
	vec4 l = loc0; // {lx, ly, lz, 0}
	vec4 k = vec4(0.3, 0.7, loc1.z, loc1.w); // {ka, kd, ks, ns}
	vec4 n, w;
)GLSHDR";

inline constexpr const char* VOL_TAIL  = R"GLSHDR(
//VOL_TAIL
}
)GLSHDR";

inline constexpr const char* VOL_DATA_VOLUME_LOOKUP  = R"GLSHDR(
	//VOL_DATA_VOLUME_LOOKUP
	vec4 v = texture(tex0, t.stp);
)GLSHDR";

inline constexpr const char* VOL_DATA_VOLUME_LOOKUP_130  = R"GLSHDR(
	//VOL_DATA_VOLUME_LOOKUP_130
	vec4 v = texture(tex0, t.stp);
)GLSHDR";

inline constexpr const char* VOL_GRAD_COMPUTE_LO  = R"GLSHDR(
	//VOL_GRAD_COMPUTE_LO
	vec4 dir = loc4; // 
	vec4 r, p; 
	v = vec4(v.x); 
	n = vec4(0.0); 
	w = vec4(0.0);
	w.x = dir.x; 
	p = clamp(TexCoord + w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.x = v.x - r.x; 
	w = vec4(0.0); 
	w.y = dir.y; 
	p = clamp(TexCoord + w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.y = v.y - r.x; 
	w = vec4(0.0); 
	w.z = dir.x<dir.z?dir.x:dir.z; 
	p = clamp(TexCoord + w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.z = v.z - r.x; 
	p.y = length(n.xyz); 
	p.y = 0.5 * (loc2.x<0.0?(1.0+p.y*loc2.x):p.y*loc2.x); 
)GLSHDR";

inline constexpr const char* VOL_GRAD_COMPUTE  = R"GLSHDR(
	// VOL_GRAD_COMPUTE
	vec4 dir = loc4;//(1/nx, 1/ny, 1/nz, 1/sample_rate)
	vec4 r, p; 
	v = vec4(v.x); 
	n = vec4(0.0); 
	w = vec4(0.0);
	w.x = dir.x; 
	p = clamp(TexCoord + w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.x = r.x + n.x; 
	p = clamp(TexCoord - w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.x = r.x - n.x; 
	w = vec4(0.0); 
	w.y = dir.y; 
	p = clamp(TexCoord + w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.y = r.x + n.y; 
	p = clamp(TexCoord - w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.y = r.x - n.y; 
	w = vec4(0.0); 
	w.z = dir.x<dir.z?dir.x:dir.z; 
	p = clamp(TexCoord + w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.z = r.x + n.z; 
	p = clamp(TexCoord - w, 0.0, 1.0); 
	r = texture(tex0, p.stp); 
	n.z = r.x - n.z; 
	p.y = length(n.xyz); 
	p.y = 0.5 * (loc2.x<0.0?(1.0+p.y*loc2.x):p.y*loc2.x); 
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
	n.xyz *= loc5.xyz;
	n.xyz = normalize(n.xyz);
	float lambert = dot(l.xyz, n.xyz); // calculate angle between light and normal. 
	lambert = clamp(abs(lambert), 0.0, 1.0); // two-sided lighting, n.w = abs(cos(angle))  
	n.w = lambert;
	w = k; // w.x = weight*ka, w.y = weight*kd, w.z = weight*ks 
	w.x = k.x - w.y; // w.x = ka - kd*weight 
	w.x = w.x + k.y; // w.x = ka + kd - kd*weight 
	n.z = pow(n.w, k.w); // n.z = abs(cos(angle))^ns 
	n.z = isnan(n.z)? 1.0:n.z;
	n.w = (n.w * w.y) + w.x; // n.w = abs(cos(angle))*kd+ka
	n.z = w.z * n.z; // n.z = weight*ks*abs(cos(angle))^ns 
)GLSHDR";

inline constexpr const char* VOL_COMPUTED_GM_LOOKUP  = R"GLSHDR(
	//VOL_COMPUTED_GM_LOOKUP
	v.y = p.y;
)GLSHDR";

inline constexpr const char* VOL_COMPUTED_GM_NOUSE  = R"GLSHDR(
	//VOL_COMPUTED_GM_NOUSE
	v.y = 0.0;
)GLSHDR";

inline constexpr const char* VOL_COMPUTED_GM_INVALIDATE  = R"GLSHDR(
	//VOL_COMPUTED_GM_INVALIDATE
	v.y = loc2.y;
)GLSHDR";

inline constexpr const char* VOL_TEXTURE_GM_LOOKUP  = R"GLSHDR(
	//VOL_TEXTURE_GM_LOOKUP
	v.y = texture(tex1, t.stp).x;
)GLSHDR";

inline constexpr const char* VOL_DATA_4D_INTENSITY_DELTA  = R"GLSHDR(
	//VOL_DATA_4D_INTENSITY_DELTA
	float v4d = v.x - texture(tex10, t.stp).x;
)GLSHDR";

inline constexpr const char* VOL_DATA_4D_SPEED  = R"GLSHDR(
	//VOL_DATA_4D_SPEED
	float grad_t = v.x - texture(tex10, t.stp).x;
	vec3 grad_s = n.xyz * loc5.xyz;
	float denom = dot(grad_s, grad_s) + 1e-4;
	grad_s = -grad_s * grad_t / denom;
	float v4d = length(grad_s);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_SIN_COLOR  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_SIN_COLOR
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
		float pa = pow(tf_val, loc18.y);
		alpha = 1.0 - pow(1.0-pa, loc4.w);
		c = vec4(loc9.rgb*loc18.z*alpha*(loc18.y>1.1?1.0:tf_val), alpha);
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID
	vec4 c;
	float tf_val = 0.0;
	float alpha = 0.0;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))
		c = vec4(0.0, 0.0, 0.0, 1.0);
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
		c = vec4(loc9.rgb*loc18.z*tf_val, 1.0);
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
		rb.r = clamp((4.0*valu - 2.0)*loc6.w, 0.0, 1.0);
		rb.g = clamp(valu<0.5 ? 4.0*valu : -4.0*valu+4.0, 0.0, 1.0);
		rb.b = clamp((2.0 - 4.0*valu)*loc6.w, 0.0, 1.0);
)GLSHDR";

inline constexpr const char* VOL_COLORMAP_DIFF_CALC0  = R"GLSHDR(
		//VOL_COLORMAP_DIFF_CALC0
		rb.rgb = clamp(n.xyz*loc6.w, -1.0, 1.0) + vec3(1.0);
)GLSHDR";

//primary-secondary
inline constexpr const char* VOL_COLORMAP_CALC1  = R"GLSHDR(
		//VOL_COLORMAP_CALC1
		rb.rgb = mix(loc6.w>0.0?loc16.rgb:loc9.rgb, loc6.w>0.0?loc9.rgb:loc16.rgb, clamp(valu, 0.0, 1.0));
)GLSHDR";

//hot
inline constexpr const char* VOL_COLORMAP_CALC2  = R"GLSHDR(
		//VOL_COLORMAP_CALC2
		rb.r = clamp(loc6.w*2.0*valu+(loc6.w>0.0?0.0:2.0), 0.0, 1.0);
		rb.g = clamp(loc6.w*(4.0*valu - 2.0), 0.0, 1.0);
		rb.b = clamp(loc6.w*4.0*valu+(loc6.w>0.0?-3.0:1.0), 0.0, 1.0);
)GLSHDR";

//cool
inline constexpr const char* VOL_COLORMAP_CALC3  = R"GLSHDR(
		//VOL_COLORMAP_CALC3
		rb.r = clamp(loc6.w>0.0?valu:(1.0-valu), 0.0, 1.0);
		rb.g = clamp(loc6.w>0.0?(1.0-valu):valu, 0.0, 1.0);
		rb.b = 1.0;
)GLSHDR";

//diverging
inline constexpr const char* VOL_COLORMAP_CALC4  = R"GLSHDR(
		//VOL_COLORMAP_CALC4
		rb.r = clamp(loc6.w>0.0?(valu<0.5?valu*0.9+0.25:0.7):(valu<0.5?0.7:-0.9*valu+1.15), 0.0, 1.0);
		rb.g = clamp(loc6.w>0.0?(valu<0.5?valu*0.8+0.3:1.4-1.4*valu):(valu<0.5?1.4*valu:-0.8*valu+1.1), 0.0, 1.0);
		rb.b = clamp(loc6.w>0.0?(valu<0.5?-0.1*valu+0.75:-1.1*valu+1.25):(valu<0.5?1.1*valu+0.15:0.1*valu+0.65), 0.0, 1.0);
)GLSHDR";

//monochrome
inline constexpr const char* VOL_COLORMAP_CALC5  = R"GLSHDR(
		//VOL_COLORMAP_CALC5
		rb.rgb = vec3((loc6.w>0.0?0.0:1.0) + loc6.w*clamp(valu, 0.0, 1.0));
)GLSHDR";

//high-key
inline constexpr const char* VOL_COLORMAP_CALC6  = R"GLSHDR(
		//VOL_COLORMAP_CALC6
		rb.rgb = mix(loc6.w>0.0?vec3(1.0):loc9.rgb, loc6.w>0.0?loc9.rgb:vec3(1.0), clamp(valu, 0.0, 1.0));
)GLSHDR";

//low-key
inline constexpr const char* VOL_COLORMAP_CALC7  = R"GLSHDR(
		//VOL_COLORMAP_CALC7
		rb.rgb = mix(loc6.w>0.0?loc9.rgb:loc9.rgb*0.1, loc6.w>0.0?loc9.rgb*0.1:loc9.rgb, clamp(valu, 0.0, 1.0));
)GLSHDR";

//increased transp
inline constexpr const char* VOL_COLORMAP_CALC8  = R"GLSHDR(
		//VOL_COLORMAP_CALC8
		rb.rgb = mix(loc6.w>0.0?vec3(0.0):loc9.rgb, loc6.w>0.0?loc9.rgb:vec3(0.0), clamp(valu, 0.0, 1.0));
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_COLORMAP
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
		vec4 rb = vec4(0.0);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU0  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU0
		float valu = (tf_val-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU1  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_Z
		vec4 tt = matrix2 * t;
		float valu = (tt.z-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU2  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_Y
		vec4 tt = matrix2 * t;
		float valu = (tt.y-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU3  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_X
		vec4 tt = matrix2 * t;
		float valu = (tt.x-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU4  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_T
		float valu = (loci1 != 0u) ? float(loci0) / float(loci1) : 0.0;
		valu = (valu-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU5  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_GM
		float valu = (v.y-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU6  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU5
		float valu = dot(clamp(n.xyz, -1.0, 1.0), l.xyz/*vec3(1.0, 1.0, 0.0)*/);
		valu = valu + 1.0;
		valu = (valu-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU7  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU7
		float valu = v4d;
		float exponent = max(loc6.z / 10.0, 0.01);
		valu = valu < 0.0 ? -pow(-valu, exponent) : pow(valu, exponent);
		valu = (valu + 1.0) / 2.0;
		valu = (valu-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_VALU8  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU8
		float valu = v4d;
		float exponent = max(loc6.z, 0.01);
		valu = pow(valu, exponent);
		valu = (valu-loc6.x)/loc6.z;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_RESULT  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_RESULT
		float pa = pow(tf_val, loc18.y);
		float alpha = 1.0 - pow(1.0-pa, loc4.w);
		c = vec4(rb.rgb*loc18.z*alpha*(loc18.y>1.1?1.0:tf_val), alpha);
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_SOLID  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_COLORMAP_SOLID
	vec4 c;
	float tf_val = 0.0;
	float alpha = 0.0;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))
		c = vec4(0.0, 0.0, 0.0, 1.0);
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		vec4 rb = vec4(0.0);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT
		c = vec4(rb.rgb*loc18.z, 1.0);
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_HEAD = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_HEAD
	vec4 c;
	float tf_val = 0.0;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_ZERO = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_ZERO
		c = vec4(0.0, 0.0, 0.0, 0.0);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_ZERO_SOLID = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_ZERO_SOLID
		c = vec4(0.0, 0.0, 0.0, 1.0);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_TF = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_TF
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_ENCODE = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_ENCODE
		float l1 = tf_val * 256.3;
		float l2 = tf_val * 1053.1;
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_TRANSP = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_TRANSP
		c = vec4(l1 + valu, l2 + valu, tf_val, tf_val);
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_SOLID  = R"GLSHDR(
		//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT_SOLID
		c = vec4(l1 + valu, l2 + valu, tf_val, 1.0);
	}
)GLSHDR";

inline constexpr const char* VOL_TRANSFER_FUNCTION_DEPTHMAP  = R"GLSHDR(
	//VOL_TRANSFER_FUNCTION_DEPTHMAP
	vec4 c;
	float tf_val = 0.0;
	float alpha = 0.0;
	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;
	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w) )
		c = vec4(0.0);
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0);
		float gmf = 5.0*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0+gmf*gmf;
		tf_val = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);
		float alpha = tf_val;
		c = vec4(vec3(alpha*tf_val), alpha);
	}
)GLSHDR";

inline constexpr const char* VOL_COLOR_OUTPUT  = R"GLSHDR(
	//VOL_COLOR_OUTPUT
	float shd = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :
		((loc1.x-1.0)*lambert+(2.0-loc1.x)*(n.w+n.z));
	c.xyz *= loc1.y>0.0?shd:1.0;
)GLSHDR";

inline constexpr const char* VOL_COLOR_OUTPUT_LABEL  = R"GLSHDR(
	//VOL_COLOR_OUTPUT_LABEL
	float shad = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :
		(n.w+n.z);
	sel.xyz *= loc1.y>0.0?shad:1.0;
	FragColor = sel*loc18.x;
)GLSHDR";

inline constexpr const char* VOL_COLOR_OUTPUT_LABEL_MASK  = R"GLSHDR(
	//VOL_COLOR_OUTPUT_LABEL_MASK
	float shad = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :
		(n.w+n.z);
	sel.xyz *= loc1.y>0.0?shad:1.0;
	FragColor = sel*alpha*tf_val*loc18.x;
)GLSHDR";

inline constexpr const char* VOL_COLOR_OUTPUT_LABEL_MASK_SOLID  = R"GLSHDR(
	//VOL_COLOR_OUTPUT_LABEL_MASK_SOLID
	float shad = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :
		(n.w+n.z);
	sel.xyz *= loc1.y>0.0?shad:1.0;
	FragColor = vec4(sel.xyz, 1.0);
)GLSHDR";

inline constexpr const char* VOL_FOG_BODY  = R"GLSHDR(
	//VOL_FOG_BODY
	v.x = (fp.y-fp.w)/(fp.y-fp.z);
	v.x = clamp(v.x, 0.0, 1.0);
	v.x = 1.0-exp(-pow(v.x*2.5, 2.0));
	c.xyz = mix(c.xyz, vec3(0.0), v.x*fp.x); 
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
	float w_a = (c * loc18.x).a;
	w_a = w_a > 0.1 ? w_a : 0.1 * smoothstep(0.02, 0.1, w_a);
	float w_d = pow(1.0 - curz, 3.0);
	FragDepth = vec2(curz, 1.0) * w_d * w_a;
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_NOMASK  = R"GLSHDR(
	//VOL_RASTER_BLEND_NOMASK
	vec4 cmask = texture(tex2, t.stp); //get mask value
	FragColor = vec4(1.0-cmask.x)*c*loc18.x;
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_NOMASK_SOLID  = R"GLSHDR(
	//VOL_RASTER_BLEND_NOMASK_SOLID
	vec4 cmask = texture(tex2, t.stp); //get mask value
	FragColor = vec4(1.0-cmask.x)*c;
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_NOMASK_DMAP  = R"GLSHDR(
	//VOL_RASTER_BLEND_NOMASK_DMAP
	float prevz = texture(tex4, fcf).r;
	float currz = gl_FragCoord.z;
	vec4 cmask = texture(tex2, t.stp); //get mask value
	float intpo = (vec4(1.0-cmask.x)*c*loc18.x).r;
	FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_MASK  = R"GLSHDR(
	//VOL_RASTER_BLEND_MASK
	vec4 cmask = texture(tex2, t.stp); //get mask value
	FragColor = tf_val*cmask.x<loc16.w?vec4(0.0):vec4(cmask.x)*c*loc18.x;
	//FragColor = cmask;
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_MASK_SOLID  = R"GLSHDR(
	//VOL_RASTER_BLEND_MASK_SOLID
	vec4 cmask = texture(tex2, t.stp); //get mask value
	FragColor = tf_val*cmask.x<loc16.w?vec4(0.0):vec4(cmask.x)*c;
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_MASK_DMAP  = R"GLSHDR(
	//VOL_RASTER_BLEND_MASK_DMAP
	float prevz = texture(tex4, fcf).r;
	float currz = gl_FragCoord.z;
	vec4 cmask = texture(tex2, t.stp); //get mask value
	float intpo = (vec4(cmask.x)*c*loc18.x).r;
	FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);
)GLSHDR";

#ifdef _WIN32
inline constexpr const char* VOL_RASTER_BLEND_LABEL  = R"GLSHDR(
	//VOL_RASTER_BLEND_LABEL
	uint label = texture(tex3, t.stp).x; //get mask value
	vec4 sel = vec4(0.2,
					0.4,
					0.4, 1.0);
	float hue, p2, p3;
	if (label > uint(0))
	{
		uint cv = label % uint(0xfd);
		uint si = uint(loc5.w);
		cv = (cv << si) & 0xff | (cv >> (8 - si));
		hue = float(cv)/45.0;
		p2 = 1.0 - hue + floor(hue);
		p3 = hue - floor(hue);
		if (hue < 1.0)
			sel = vec4(1.0, p3, 1.0, 1.0);
		else if (hue < 2.0)
			sel = vec4(p2, 1.0, 1.0, 1.0);
		else if (hue < 3.0)
			sel = vec4(1.0, 1.0, p3, 1.0);
		else if (hue < 4.0)
			sel = vec4(1.0, p2, 1.0, 1.0);
		else if (hue < 5.0)
			sel = vec4(p3, 1.0, 1.0, 1.0);
		else
			sel = vec4(1.0, 1.0, p2, 1.0);
	}
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_LABEL_MASK  = R"GLSHDR(
	//VOL_RASTER_BLEND_LABEL_MASK
	vec4 cmask = texture(tex2, t.stp); //get mask value
	if (cmask.x <= loc16.w)
	{
		FragColor = c*loc18.x;
		return;
	}
	uint label = texture(tex3, t.stp).x; //get mask value
	vec4 sel = vec4(0.1,
					0.2,
					0.2, 0.5);
	float hue, p2, p3;
	if (label > uint(0))
	{
		uint cv = label % uint(0xfd);
		uint si = uint(loc5.w);
		cv = (cv << si) & 0xff | (cv >> (8 - si));
		hue = float(cv)/45.0;
		p2 = 1.0 - hue + floor(hue);
		p3 = hue - floor(hue);
		if (hue < 1.0)
			sel = vec4(1.0, p3, 0.0, 1.0);
		else if (hue < 2.0)
			sel = vec4(p2, 1.0, 0.0, 1.0);
		else if (hue < 3.0)
			sel = vec4(0.0, 1.0, p3, 1.0);
		else if (hue < 4.0)
			sel = vec4(0.0, p2, 1.0, 1.0);
		else if (hue < 5.0)
			sel = vec4(p3, 0.0, 1.0, 1.0);
		else
			sel = vec4(1.0, 0.0, p2, 1.0);
	}
)GLSHDR";

#else
inline constexpr const char* VOL_RASTER_BLEND_LABEL  = R"GLSHDR(
	//VOL_RASTER_BLEND_LABEL
	uint label = texture(tex3, t.stp).x; //get mask value
	vec4 sel = vec4(0.2,
					0.4,
					0.4, 1.0);
	float hue, p2, p3;
	if (label > uint(0))
	{
		uint cv = label % uint(0xfd);
		uint si = uint(loc5.w);
		cv = ((cv << si) << 24 >> 24) | (cv >> (8 - si));
		hue = float(cv)/45.0;
		p2 = 1.0 - hue + floor(hue);
		p3 = hue - floor(hue);
		if (hue < 1.0)
			sel = vec4(1.0, p3, 1.0, 1.0);
		else if (hue < 2.0)
			sel = vec4(p2, 1.0, 1.0, 1.0);
		else if (hue < 3.0)
			sel = vec4(1.0, 1.0, p3, 1.0);
		else if (hue < 4.0)
			sel = vec4(1.0, p2, 1.0, 1.0);
		else if (hue < 5.0)
			sel = vec4(p3, 1.0, 1.0, 1.0);
		else
			sel = vec4(1.0, 1.0, p2, 1.0);
	}
)GLSHDR";

inline constexpr const char* VOL_RASTER_BLEND_LABEL_MASK  = R"GLSHDR(
	//VOL_RASTER_BLEND_LABEL_MASK
	vec4 cmask = texture(tex2, t.stp); //get mask value
	if (cmask.x <= loc16.w)
	{
		FragColor = c*loc18.x;
		return;
	}
	uint label = texture(tex3, t.stp).x; //get mask value
	vec4 sel = vec4(0.1,
					0.2,
					0.2, 0.5);
	float hue, p2, p3;
	if (label > uint(0))
	{
		uint cv = label % uint(0xfd);
		uint si = uint(loc5.w);
		cv = ((cv << si) << 24 >> 24) | (cv >> (8 - si));
		hue = float(cv)/45.0;
		p2 = 1.0 - hue + floor(hue);
		p3 = hue - floor(hue);
		if (hue < 1.0)
			sel = vec4(1.0, p3, 0.0, 1.0);
		else if (hue < 2.0)
			sel = vec4(p2, 1.0, 0.0, 1.0);
		else if (hue < 3.0)
			sel = vec4(0.0, 1.0, p3, 1.0);
		else if (hue < 4.0)
			sel = vec4(0.0, p2, 1.0, 1.0);
		else if (hue < 5.0)
			sel = vec4(p3, 0.0, 1.0, 1.0);
		else
			sel = vec4(1.0, 0.0, p2, 1.0);
	}
)GLSHDR";

#endif

#endif//VolShaderCode_h
