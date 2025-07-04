﻿//  
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

#define DEFAULT_FRAGMENT_CODE \
	"void main() {\n" \
	"    FragColor = vec4(1,1,1,1);\n" \
	"}\n" 

#define VOL_INPUTS \
	"in vec3 OutVertex;\n" \
	"in vec3 OutTexture;\n"

#define VOL_INPUTS_FOG \
	"in vec4 OutFogCoord;\n"

#define VOL_OUTPUTS \
	"out vec4 FragColor;\n"

#define VOL_UNIFORMS_COMMON \
	"// VOL_UNIFORMS_COMMON\n" \
	"uniform vec4 loc0;//(lx, ly, lz, alpha)\n" \
	"uniform vec4 loc1;//(ka, kd, ks, ns)\n" \
	"uniform vec4 loc2;//(scalar_scale, gm_thresh, left_thresh, right_thresh)\n" \
	"uniform vec4 loc3;//(gamma, left_offset, right_offset, sw)\n" \
	"uniform vec4 loc4;//(1/nx, 1/ny, 1/nz, 1/sample_rate)\n" \
	"uniform vec4 loc5;//(spcx, spcy, spcz, shuffle)\n" \
	"uniform vec4 loc9;//(primary red, green, blue, alpha_power)\n" \
	"uniform vec4 loc16;//(secondary red, green, blue, mask_threshold)\n" \
	"\n" \
	"uniform sampler3D tex0;//data volume\n" \
	"uniform sampler3D tex1;//gm volume\n" \
	"\n" \
	"uniform mat4 matrix5;//texture\n" \
	"\n"

#define VOL_UNIFORMS_4D_CACHE \
	"// VOL_UNIFORMS_4D_CACHE\n" \
	"uniform sampler3D tex10;//data from t-1\n" \
	"\n"

#define VOL_UNIFORMS_MATRICES \
	"// VOL_UNIFORMS_MATRICES\n" \
	"uniform mat4 matrix2;//tex transform for bricking\n" \
	"\n"

#define VOL_UNIFORMS_SIN_COLOR \
	"//VOL_UNIFORMS_SIN_COLOR\n" \
	"uniform vec4 loc6;//(red, green, blue, 0)\n" \
	"\n"

#define VOL_UNIFORMS_COLORMAP \
	"//VOL_UNIFORMS_COLORMAP\n" \
	"uniform vec4 loc6;//(low, hi, hi-lo, inv)\n" \
	"\n"

#define VOL_UNIFROMS_4D_COLORMAP \
	"//VOL_UNIFROMS_4D_COLORMAP\n" \
	"uniform uint loci0;//time\n" \
	"uniform uint loci1;//time length\n" \
	"\n"

#define VOL_UNIFORMS_2DMAP_LOC \
	"//VOL_UNIFORMS_2DMAP_LOC\n" \
	"uniform vec4 loc7;//(1/vx, 1/vy, 0, 0)\n" \
	"\n"

#define VOL_UNIFORMS_FOG_LOC \
	"//VOL_UNIFORMS_FOG_LOC\n" \
	"uniform vec4 loc8;//(int, start, end, 0.0)\n" \
	"\n"

#define VOL_UNIFORMS_DP \
	"//VOL_UNIFORMS_DP\n" \
	"uniform sampler2D tex14;//depth texture 1\n" \
	"uniform sampler2D tex15;//depth texture 2\n" \
	"\n"

#define VOL_UNIFORMS_CLIP \
	"//VOL_UNIFORMS_CLIP\n" \
	"uniform vec4 loc10; //plane0\n" \
	"uniform vec4 loc11; //plane1\n" \
	"uniform vec4 loc12; //plane2\n" \
	"uniform vec4 loc13; //plane3\n" \
	"uniform vec4 loc14; //plane4\n" \
	"uniform vec4 loc15; //plane5\n" \
	"\n"

#define VOL_UNIFORMS_MASK \
	"//VOL_UNIFORMS_MASK\n" \
	"uniform sampler3D tex2;//3d mask volume\n" \
	"\n"

#define VOL_UNIFORMS_LABEL \
	"//VOL_UNIFORMS_LABEL\n" \
	"uniform usampler3D tex3;//3d label volume\n" \
	"\n"

#define VOL_UNIFORMS_DEPTHMAP \
	"//VOL_UNIFORMS_DEPTHMAP\n" \
	"uniform sampler2D tex4;//2d depth map\n" \
	"\n"

#define VOL_HEAD \
	"//VOL_HEAD\n" \
	"void main()\n" \
	"{\n" \
	"	vec4 TexCoord = vec4(OutTexture, 1.0);\n" \
	"	vec4 t = TexCoord;\n" \
	"\n"

#define VOL_HEAD_2DMAP_LOC \
	"	//VOL_HEAD_2DMAP_LOC\n" \
	"	vec2 fcf = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"\n"

#define VOL_HEAD_DP_1 \
	"	//VOL_HEAD_DP_NEG\n" \
	"	if (texture(tex15, fcf).r < gl_FragCoord.z) discard;\n" \
	"\n"

#define VOL_HEAD_DP_2 \
	"	//VOL_HEAD_DP_POS\n" \
	"	if (texture(tex15, fcf).r > gl_FragCoord.z) discard;\n" \
	"\n"

#define VOL_HEAD_DP_3 \
	"	//VOL_HEAD_DP_BOTH\n" \
	"	if (texture(tex15, fcf).r < gl_FragCoord.z) discard;\n" \
	"	else if (texture(tex14, fcf).r > gl_FragCoord.z) discard;\n" \
	"\n"

#define VOL_HEAD_FOG \
	"	//VOL_HEAD_FOG\n" \
	"	vec4 fp;\n" \
	"	fp.x = loc8.x;\n" \
	"	fp.y = loc8.y;\n" \
	"	fp.z = loc8.z;\n" \
	"	fp.w = abs(OutFogCoord.z/OutFogCoord.w);\n" \
	"\n"

#define VOL_HEAD_CLIP \
	"	//VOL_HEAD_CLIP\n" \
	"	vec4 brickt = matrix2 * t;\n" \
	"	if (dot(brickt.xyz, loc10.xyz)+loc10.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc11.xyz)+loc11.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc12.xyz)+loc12.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc13.xyz)+loc13.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc14.xyz)+loc14.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc15.xyz)+loc15.w < 0.0)\n" \
	"	{\n" \
	"		discard;//FragColor = vec4(0.0);\n" \
	"		return;\n" \
	"	}\n" \
	"\n"

#define VOL_HEAD_CLIP_FUNC \
	"	//VOL_HEAD_CLIP_FUNC\n" \
	"	if (vol_clip_func(t))\n" \
	"	{\n" \
	"		discard;\n" \
	"		return;\n" \
	"	}\n" \
	"\n"

#define VOL_CLIP_FUNC \
	"//VOL_CLIP_FUNC\n" \
	"bool vol_clip_func(vec4 t)\n" \
	"{\n" \
	"	vec4 brickt = matrix2 * t;\n" \
	"	if (dot(brickt.xyz, loc10.xyz)+loc10.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc11.xyz)+loc11.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc12.xyz)+loc12.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc13.xyz)+loc13.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc14.xyz)+loc14.w < 0.0 ||\n" \
	"		dot(brickt.xyz, loc15.xyz)+loc15.w < 0.0)\n" \
	"		return true;\n" \
	"	else\n" \
	"		return false;\n" \
	"}\n" \
	"\n"

#define VOL_HEAD_LIT \
	"	//VOL_HEAD_LIT\n" \
	"	vec4 l = loc0; // {lx, ly, lz, alpha}\n" \
	"	vec4 k = vec4(0.3, 0.7, loc1.z, loc1.w); // {ka, kd, ks, ns}\n" \
	"	vec4 n, w;\n" \
	"\n"

#define VOL_TAIL \
	"//VOL_TAIL\n" \
	"}\n" \
	"\n"

#define VOL_DATA_VOLUME_LOOKUP \
	"	//VOL_DATA_VOLUME_LOOKUP\n" \
	"	vec4 v = texture(tex0, t.stp);\n" \
	"\n"

#define VOL_DATA_VOLUME_LOOKUP_130 \
	"	//VOL_DATA_VOLUME_LOOKUP_130\n" \
	"	vec4 v = texture(tex0, t.stp);\n" \
	"\n"

#define VOL_GRAD_COMPUTE_LO \
	"	//VOL_GRAD_COMPUTE_LO\n" \
	"	vec4 dir = loc4; // \n" \
	"	vec4 r, p; \n" \
	"	v = vec4(v.x); \n" \
	"	n = vec4(0.0); \n" \
	"	w = vec4(0.0);\n" \
	"	w.x = dir.x; \n" \
	"	p = clamp(TexCoord + w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.x = v.x - r.x; \n" \
	"	w = vec4(0.0); \n" \
	"	w.y = dir.y; \n" \
	"	p = clamp(TexCoord + w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.y = v.y - r.x; \n" \
	"	w = vec4(0.0); \n" \
	"	w.z = dir.x<dir.z?dir.x:dir.z; \n" \
	"	p = clamp(TexCoord + w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.z = v.z - r.x; \n" \
	"	p.y = length(n.xyz); \n" \
	"	p.y = 0.5 * (loc2.x<0.0?(1.0+p.y*loc2.x):p.y*loc2.x); \n" \
	"\n"

#define VOL_GRAD_COMPUTE \
	"	// VOL_GRAD_COMPUTE\n" \
	"	vec4 dir = loc4;//(1/nx, 1/ny, 1/nz, 1/sample_rate)\n" \
	"	vec4 r, p; \n" \
	"	v = vec4(v.x); \n" \
	"	n = vec4(0.0); \n" \
	"	w = vec4(0.0);\n" \
	"	w.x = dir.x; \n" \
	"	p = clamp(TexCoord + w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.x = r.x + n.x; \n" \
	"	p = clamp(TexCoord - w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.x = r.x - n.x; \n" \
	"	w = vec4(0.0); \n" \
	"	w.y = dir.y; \n" \
	"	p = clamp(TexCoord + w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.y = r.x + n.y; \n" \
	"	p = clamp(TexCoord - w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.y = r.x - n.y; \n" \
	"	w = vec4(0.0); \n" \
	"	w.z = dir.x<dir.z?dir.x:dir.z; \n" \
	"	p = clamp(TexCoord + w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.z = r.x + n.z; \n" \
	"	p = clamp(TexCoord - w, 0.0, 1.0); \n" \
	"	r = texture(tex0, p.stp); \n" \
	"	n.z = r.x - n.z; \n" \
	"	p.y = length(n.xyz); \n" \
	"	p.y = 0.5 * (loc2.x<0.0?(1.0+p.y*loc2.x):p.y*loc2.x); \n" \
	"\n"

#define VOL_GRAD_COMPUTE_FUNC \
	"// VOL_GRAD_COMPUTE_FUNC\n" \
	"vec4 vol_grad_func(vec4 pos, vec4 dir)\n" \
	"{\n" \
	"	vec4 r, p;\n" \
	"	vec4 n = vec4(0.0);\n" \
	"	vec4 w = vec4(0.0);\n" \
	"	w.x = dir.x;\n" \
	"	p = clamp(pos+w, 0.0, 1.0);\n" \
	"	r = texture(tex0, p.stp);\n" \
	"	n.x = r.x + n.x;\n" \
	"	p = clamp(pos-w, 0.0, 1.0);\n" \
	"	r = texture(tex0, p.stp);\n" \
	"	n.x = r.x - n.x;\n" \
	"	w = vec4(0.0);\n" \
	"	w.y = dir.y;\n" \
	"	p = clamp(pos+w, 0.0, 1.0);\n" \
	"	r = texture(tex0, p.stp);\n" \
	"	n.y = r.x + n.y;\n" \
	"	p = clamp(pos-w, 0.0, 1.0);\n" \
	"	r = texture(tex0, p.stp);\n" \
	"	n.y = r.x - n.y;\n" \
	"	w = vec4(0.0);\n" \
	"	w.z = dir.x<dir.z?dir.x:dir.z;\n" \
	"	p = clamp(pos+w, 0.0, 1.0);\n" \
	"	r = texture(tex0, p.stp);\n" \
	"	n.z = r.x + n.z;\n" \
	"	p = clamp(pos-w, 0.0, 1.0);\n" \
	"	r = texture(tex0, p.stp);\n" \
	"	n.z = 0.3 * (r.x - n.z);\n" \
	"	return n;\n" \
	"}\n"

#define VOL_BODY_SHADING \
	"	//VOL_BODY_SHADING\n" \
	"	n.xyz *= loc5.xyz;\n" \
	"	n.xyz = normalize(n.xyz);\n" \
	"	float lambert = dot(l.xyz, n.xyz); // calculate angle between light and normal. \n" \
	"	lambert = clamp(abs(lambert), 0.0, 1.0); // two-sided lighting, n.w = abs(cos(angle))  \n" \
	"	n.w = lambert;\n" \
	"	w = k; // w.x = weight*ka, w.y = weight*kd, w.z = weight*ks \n" \
	"	w.x = k.x - w.y; // w.x = ka - kd*weight \n" \
	"	w.x = w.x + k.y; // w.x = ka + kd - kd*weight \n" \
	"	n.z = pow(n.w, k.w); // n.z = abs(cos(angle))^ns \n" \
	"	n.z = isnan(n.z)? 1.0:n.z;\n" \
	"	n.w = (n.w * w.y) + w.x; // n.w = abs(cos(angle))*kd+ka\n" \
	"	n.z = w.z * n.z; // n.z = weight*ks*abs(cos(angle))^ns \n" \
	"\n"

#define VOL_COMPUTED_GM_LOOKUP \
	"	//VOL_COMPUTED_GM_LOOKUP\n" \
	"	v.y = p.y;\n" \
	"\n"

#define VOL_COMPUTED_GM_NOUSE \
	"	//VOL_COMPUTED_GM_NOUSE\n" \
	"	v.y = 0.0;\n" \
	"\n"

#define VOL_COMPUTED_GM_INVALIDATE \
	"	//VOL_COMPUTED_GM_INVALIDATE\n" \
	"	v.y = loc2.y;\n" \
	"\n"

#define VOL_TEXTURE_GM_LOOKUP \
	"	//VOL_TEXTURE_GM_LOOKUP\n" \
	"	v.y = texture(tex1, t.stp).x;\n" \
	"\n"

#define VOL_DATA_4D_INTENSITY_DELTA \
	"	//VOL_DATA_4D_INTENSITY_DELTA\n" \
	"	float v4d = v.x - texture(tex10, t.stp).x;\n" \
	"\n"

#define VOL_DATA_4D_SPEED \
	"	//VOL_DATA_4D_SPEED\n" \
	"	float grad_t = v.x - texture(tex10, t.stp).x;\n" \
	"	vec3 grad_s = n.xyz * loc5.xyz;\n" \
	"	float denom = dot(grad_s, grad_s) + 1e-4;\n" \
	"	grad_s = -grad_s * grad_t / denom;\n" \
	"	float v4d = length(grad_s);\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR \
	"	//VOL_TRANSFER_FUNCTION_SIN_COLOR\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \
	"		alpha = 1.0 - pow(1.0-pow(tf_alp, loc9.w), loc4.w);\n" \
	"		c = vec4(loc6.rgb*alpha*(loc9.w>1.1?1.0:tf_alp), alpha);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID \
	"	//VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0, 0.0, 0.0, 1.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \
	"		c = vec4(loc6.rgb*tf_alp, 1.0);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR_L \
	"	//VOL_TRANSFER_FUNCTION_SIN_COLOR_L\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \
	"		c = vec4(tf_alp);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC \
	"//VOL_TRANSFER_FUNCTION_SIN_COLOR_L_FUNC\n" \
	"vec4 vol_trans_sin_color_l(vec4 v)\n" \
	"{\n" \
	"	vec4 c;\n" \
	"	float tf_alp;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \
	"		c = vec4(tf_alp);\n" \
	"	}\n" \
	"	return c;\n" \
	"}\n" \
	"\n"

#define VOL_COMMON_TRANSFER_FUNCTION_CALC \
	"		//VOL_COMMON_TRANSFER_FUNCTION_CALC\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \

//rainbow
#define VOL_COLORMAP_CALC0 \
	"		//VOL_COLORMAP_CALC0\n" \
	"		rb.r = clamp((4.0*valu - 2.0)*loc6.w, 0.0, 1.0);\n" \
	"		rb.g = clamp(valu<0.5 ? 4.0*valu : -4.0*valu+4.0, 0.0, 1.0);\n" \
	"		rb.b = clamp((2.0 - 4.0*valu)*loc6.w, 0.0, 1.0);\n"

#define VOL_COLORMAP_DIFF_CALC0 \
	"		//VOL_COLORMAP_DIFF_CALC0\n" \
	"		rb.rgb = clamp(n.xyz*loc6.w, -1.0, 1.0) + vec3(1.0);\n" \

//primary-secondary
#define VOL_COLORMAP_CALC1 \
	"		//VOL_COLORMAP_CALC1\n" \
	"		rb.rgb = mix(loc6.w>0.0?loc16.rgb:loc9.rgb, loc6.w>0.0?loc9.rgb:loc16.rgb, clamp(valu, 0.0, 1.0));\n"

//hot
#define VOL_COLORMAP_CALC2 \
	"		//VOL_COLORMAP_CALC2\n" \
	"		rb.r = clamp(loc6.w*2.0*valu+(loc6.w>0.0?0.0:2.0), 0.0, 1.0);\n" \
	"		rb.g = clamp(loc6.w*(4.0*valu - 2.0), 0.0, 1.0);\n" \
	"		rb.b = clamp(loc6.w*4.0*valu+(loc6.w>0.0?-3.0:1.0), 0.0, 1.0);\n"

//cool
#define VOL_COLORMAP_CALC3 \
	"		//VOL_COLORMAP_CALC3\n" \
	"		rb.r = clamp(loc6.w>0.0?valu:(1.0-valu), 0.0, 1.0);\n" \
	"		rb.g = clamp(loc6.w>0.0?(1.0-valu):valu, 0.0, 1.0);\n" \
	"		rb.b = 1.0;\n"

//diverging
#define VOL_COLORMAP_CALC4 \
	"		//VOL_COLORMAP_CALC4\n" \
	"		rb.r = clamp(loc6.w>0.0?(valu<0.5?valu*0.9+0.25:0.7):(valu<0.5?0.7:-0.9*valu+1.15), 0.0, 1.0);\n" \
	"		rb.g = clamp(loc6.w>0.0?(valu<0.5?valu*0.8+0.3:1.4-1.4*valu):(valu<0.5?1.4*valu:-0.8*valu+1.1), 0.0, 1.0);\n" \
	"		rb.b = clamp(loc6.w>0.0?(valu<0.5?-0.1*valu+0.75:-1.1*valu+1.25):(valu<0.5?1.1*valu+0.15:0.1*valu+0.65), 0.0, 1.0);\n"

//monochrome
#define VOL_COLORMAP_CALC5 \
	"		//VOL_COLORMAP_CALC5\n" \
	"		rb.rgb = vec3((loc6.w>0.0?0.0:1.0) + loc6.w*clamp(valu, 0.0, 1.0));\n"

//high-key
#define VOL_COLORMAP_CALC6 \
	"		//VOL_COLORMAP_CALC6\n" \
	"		rb.rgb = mix(loc6.w>0.0?vec3(1.0):loc9.rgb, loc6.w>0.0?loc9.rgb:vec3(1.0), clamp(valu, 0.0, 1.0));\n"

//low-key
#define VOL_COLORMAP_CALC7 \
	"		//VOL_COLORMAP_CALC7\n" \
	"		rb.rgb = mix(loc6.w>0.0?loc9.rgb:loc9.rgb*0.1, loc6.w>0.0?loc9.rgb*0.1:loc9.rgb, clamp(valu, 0.0, 1.0));\n"

//increased transp
#define VOL_COLORMAP_CALC8 \
	"		//VOL_COLORMAP_CALC8\n" \
	"		rb.rgb = mix(loc6.w>0.0?vec3(0.0):loc9.rgb, loc6.w>0.0?loc9.rgb:vec3(0.0), clamp(valu, 0.0, 1.0));\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP \
	"	//VOL_TRANSFER_FUNCTION_COLORMAP\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		vec4 rb = vec4(0.0);\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU0 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU\n" \
	"		float valu = (v.x-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU1 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_Z\n" \
	"		vec4 tt = matrix5 * t;\n" \
	"		float valu = (tt.z-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU2 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_Y\n" \
	"		vec4 tt = matrix5 * t;\n" \
	"		float valu = (tt.y-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU3 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_X\n" \
	"		vec4 tt = matrix5 * t;\n" \
	"		float valu = (tt.x-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU4 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_T\n" \
	"		float valu = (loci1 != 0u) ? float(loci0) / float(loci1) : 0.0;\n" \
	"		valu = (valu-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU5 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU_GM\n" \
	"		float valu = (v.y-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU6 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU5\n" \
	"		float valu = dot(clamp(n.xyz, -1.0, 1.0), l.xyz/*vec3(1.0, 1.0, 0.0)*/);\n" \
	"		valu = valu + 1.0;\n" \
	"		valu = (valu-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU7 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU7\n" \
	"		float valu = v4d;\n" \
	"		float exponent = max(loc6.z / 10.0, 0.01);\n" \
	"		valu = valu < 0.0 ? -pow(-valu, exponent) : pow(valu, exponent);\n" \
	"		valu = (valu + 1.0) / 2.0;\n" \
	"		valu = (valu-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_VALU8 \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_VALU8\n" \
	"		float valu = v4d;\n" \
	"		float exponent = max(loc6.z, 0.01);\n" \
	"		valu = pow(valu, exponent);\n" \
	"		valu = (valu-loc6.x)/loc6.z;\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_RESULT \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_RESULT\n" \
	"		float alpha = 1.0 - pow(1.0-pow(tf_alp, loc9.w), loc4.w);\n" \
	"		c = vec4(rb.rgb*alpha*(loc9.w>1.1?1.0:tf_alp), alpha);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_SOLID \
	"	//VOL_TRANSFER_FUNCTION_COLORMAP_SOLID\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0, 0.0, 0.0, 1.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		vec4 rb = vec4(0.0);\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT \
	"		//VOL_TRANSFER_FUNCTION_COLORMAP_SOLID_RESULT\n" \
	"		c = vec4(rb.rgb, 1.0);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ \
	"	//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w))\n" \
	"		c = vec4(0.0, 0.0, 0.0, 0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \

#define VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ_RESULT \
	"		//VOL_TRANSFER_FUNCTION_MIP_COLOR_PROJ\n" \
	"		c = vec4(vec3(tf_alp*260.0+valu), tf_alp);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_DEPTHMAP \
	"	//VOL_TRANSFER_FUNCTION_DEPTHMAP\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || (loc2.w<1.0 && v.x>loc2.w+loc3.w) )\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0 && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (loc2.y>0.0?clamp(v.y/loc2.y, 0.0, 1.0+loc2.y*10.0):1.0)*v.x;\n" \
	"		tf_alp = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0, 1.0), loc3.x);\n" \
	"		float alpha = tf_alp;\n" \
	"		c = vec4(vec3(alpha*tf_alp), alpha);\n" \
	"	}\n" \
	"\n"

#define VOL_COLOR_OUTPUT \
	"	//VOL_COLOR_OUTPUT\n" \
	"	float shd = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :\n" \
	"		((loc1.x-1.0)*lambert+(2.0-loc1.x)*(n.w+n.z));\n" \
	"	c.xyz *= loc1.y>0.0?shd:1.0;\n" \
	"\n"

#define VOL_COLOR_OUTPUT_LABEL \
	"	//VOL_COLOR_OUTPUT_LABEL\n" \
	"	float shad = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :\n" \
	"		(n.w+n.z);\n" \
	"	sel.xyz *= loc1.y>0.0?shad:1.0;\n" \
	"	FragColor = sel*l.w;\n" \
	"\n"

#define VOL_COLOR_OUTPUT_LABEL_MASK \
	"	//VOL_COLOR_OUTPUT_LABEL_MASK\n" \
	"	float shad = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :\n" \
	"		(n.w+n.z);\n" \
	"	sel.xyz *= loc1.y>0.0?shad:1.0;\n" \
	"	FragColor = sel*alpha*tf_alp*l.w;\n" \
	"\n"

#define VOL_COLOR_OUTPUT_LABEL_MASK_SOLID \
	"	//VOL_COLOR_OUTPUT_LABEL_MASK_SOLID\n" \
	"	float shad = loc1.x < 1.0 ? (loc1.x*(n.w+n.z)+1.0-loc1.x) :\n" \
	"		(n.w+n.z);\n" \
	"	sel.xyz *= loc1.y>0.0?shad:1.0;\n" \
	"	FragColor = vec4(sel.xyz, 1.0);\n" \
	"\n"

#define VOL_FOG_BODY \
	"	//VOL_FOG_BODY\n" \
	"	v.x = (fp.z-fp.w)/(fp.z-fp.y);\n" \
	"	v.x = 1.0-clamp(v.x, 0.0, 1.0);\n" \
	"	v.x = 1.0-exp(-pow(v.x*2.5, 2.0));\n" \
	"	c.xyz = mix(c.xyz, vec3(0.0), v.x*fp.x); \n" \
	"\n"

#define VOL_RASTER_BLEND \
	"	//VOL_RASTER_BLEND\n" \
	"	FragColor = c*l.w; // VOL_RASTER_BLEND\n" \
	"\n"

#define VOL_RASTER_BLEND_SOLID \
	"	//VOL_RASTER_BLEND_SOLID\n" \
	"	FragColor = c;\n" \
	"\n"

#define VOL_RASTER_BLEND_DMAP \
	"	//VOL_RASTER_BLEND_DMAP\n" \
	"	//float prevz = texture(tex4, fcf).r;\n" \
	"	float currz = gl_FragCoord.z;\n" \
	"	float intpo = (c*l.w).r;\n" \
	"	//FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);\n" \
	"	if (intpo < 0.05) discard;\n" \
	"	FragColor = vec4(vec3(currz), 1.0);\n" \
	"\n"

#define VOL_RASTER_BLEND_NOMASK \
	"	//VOL_RASTER_BLEND_NOMASK\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	FragColor = vec4(1.0-cmask.x)*c*l.w;\n" \
	"\n"

#define VOL_RASTER_BLEND_NOMASK_SOLID \
	"	//VOL_RASTER_BLEND_NOMASK_SOLID\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	FragColor = vec4(1.0-cmask.x)*c;\n" \
	"\n"

#define VOL_RASTER_BLEND_NOMASK_DMAP \
	"	//VOL_RASTER_BLEND_NOMASK_DMAP\n" \
	"	float prevz = texture(tex4, fcf).r;\n" \
	"	float currz = gl_FragCoord.z;\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	float intpo = (vec4(1.0-cmask.x)*c*l.w).r;\n" \
	"	FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);\n" \
	"\n"

#define VOL_RASTER_BLEND_MASK \
	"	//VOL_RASTER_BLEND_MASK\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	FragColor = tf_alp*cmask.x<loc16.w?vec4(0.0):vec4(cmask.x)*c*l.w;\n" \
	"	//FragColor = cmask;\n" \
	"\n"

#define VOL_RASTER_BLEND_MASK_SOLID \
	"	//VOL_RASTER_BLEND_MASK_SOLID\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	FragColor = tf_alp*cmask.x<loc16.w?vec4(0.0):vec4(cmask.x)*c;\n" \
	"\n"

#define VOL_RASTER_BLEND_MASK_DMAP \
	"	//VOL_RASTER_BLEND_MASK_DMAP\n" \
	"	float prevz = texture(tex4, fcf).r;\n" \
	"	float currz = gl_FragCoord.z;\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	float intpo = (vec4(cmask.x)*c*l.w).r;\n" \
	"	FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);\n" \
	"\n"

#ifdef _WIN32
#define VOL_RASTER_BLEND_LABEL \
	"	//VOL_RASTER_BLEND_LABEL\n" \
	"	uint label = texture(tex3, t.stp).x; //get mask value\n" \
	"	vec4 sel = vec4(0.2,\n" \
	"					0.4,\n" \
	"					0.4, 1.0);\n" \
	"	float hue, p2, p3;\n" \
	"	if (label > uint(0))\n" \
	"	{\n" \
	"		uint cv = label % uint(0xfd);\n" \
	"		uint si = uint(loc5.w);\n" \
	"		cv = (cv << si) & 0xff | (cv >> (8 - si));\n" \
	"		hue = float(cv)/45.0;\n" \
	"		p2 = 1.0 - hue + floor(hue);\n" \
	"		p3 = hue - floor(hue);\n" \
	"		if (hue < 1.0)\n" \
	"			sel = vec4(1.0, p3, 1.0, 1.0);\n" \
	"		else if (hue < 2.0)\n" \
	"			sel = vec4(p2, 1.0, 1.0, 1.0);\n" \
	"		else if (hue < 3.0)\n" \
	"			sel = vec4(1.0, 1.0, p3, 1.0);\n" \
	"		else if (hue < 4.0)\n" \
	"			sel = vec4(1.0, p2, 1.0, 1.0);\n" \
	"		else if (hue < 5.0)\n" \
	"			sel = vec4(p3, 1.0, 1.0, 1.0);\n" \
	"		else\n" \
	"			sel = vec4(1.0, 1.0, p2, 1.0);\n" \
	"	}\n" \
	"\n"

#define VOL_RASTER_BLEND_LABEL_MASK \
	"	//VOL_RASTER_BLEND_LABEL_MASK\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	if (cmask.x <= loc16.w)\n" \
	"	{\n" \
	"		FragColor = c*l.w;\n" \
	"		return;\n" \
	"	}\n" \
	"	uint label = texture(tex3, t.stp).x; //get mask value\n" \
	"	vec4 sel = vec4(0.1,\n" \
	"					0.2,\n" \
	"					0.2, 0.5);\n" \
	"	float hue, p2, p3;\n" \
	"	if (label > uint(0))\n" \
	"	{\n" \
	"		uint cv = label % uint(0xfd);\n" \
	"		uint si = uint(loc5.w);\n" \
	"		cv = (cv << si) & 0xff | (cv >> (8 - si));\n" \
	"		hue = float(cv)/45.0;\n" \
	"		p2 = 1.0 - hue + floor(hue);\n" \
	"		p3 = hue - floor(hue);\n" \
	"		if (hue < 1.0)\n" \
	"			sel = vec4(1.0, p3, 0.0, 1.0);\n" \
	"		else if (hue < 2.0)\n" \
	"			sel = vec4(p2, 1.0, 0.0, 1.0);\n" \
	"		else if (hue < 3.0)\n" \
	"			sel = vec4(0.0, 1.0, p3, 1.0);\n" \
	"		else if (hue < 4.0)\n" \
	"			sel = vec4(0.0, p2, 1.0, 1.0);\n" \
	"		else if (hue < 5.0)\n" \
	"			sel = vec4(p3, 0.0, 1.0, 1.0);\n" \
	"		else\n" \
	"			sel = vec4(1.0, 0.0, p2, 1.0);\n" \
	"	}\n" \
	"\n"
#else
#define VOL_RASTER_BLEND_LABEL \
	"	//VOL_RASTER_BLEND_LABEL\n" \
	"	uint label = texture(tex3, t.stp).x; //get mask value\n" \
	"	vec4 sel = vec4(0.2,\n" \
	"					0.4,\n" \
	"					0.4, 1.0);\n" \
	"	float hue, p2, p3;\n" \
	"	if (label > uint(0))\n" \
	"	{\n" \
	"		uint cv = label % uint(0xfd);\n" \
	"		uint si = uint(loc5.w);\n" \
	"		cv = ((cv << si) << 24 >> 24) | (cv >> (8 - si));\n" \
	"		hue = float(cv)/45.0;\n" \
	"		p2 = 1.0 - hue + floor(hue);\n" \
	"		p3 = hue - floor(hue);\n" \
	"		if (hue < 1.0)\n" \
	"			sel = vec4(1.0, p3, 1.0, 1.0);\n" \
	"		else if (hue < 2.0)\n" \
	"			sel = vec4(p2, 1.0, 1.0, 1.0);\n" \
	"		else if (hue < 3.0)\n" \
	"			sel = vec4(1.0, 1.0, p3, 1.0);\n" \
	"		else if (hue < 4.0)\n" \
	"			sel = vec4(1.0, p2, 1.0, 1.0);\n" \
	"		else if (hue < 5.0)\n" \
	"			sel = vec4(p3, 1.0, 1.0, 1.0);\n" \
	"		else\n" \
	"			sel = vec4(1.0, 1.0, p2, 1.0);\n" \
	"	}\n" \
	"\n"

#define VOL_RASTER_BLEND_LABEL_MASK \
	"	//VOL_RASTER_BLEND_LABEL_MASK\n" \
	"	vec4 cmask = texture(tex2, t.stp); //get mask value\n" \
	"	if (cmask.x <= loc16.w)\n" \
	"	{\n" \
	"		FragColor = c*l.w;\n" \
	"		return;\n" \
	"	}\n" \
	"	uint label = texture(tex3, t.stp).x; //get mask value\n" \
	"	vec4 sel = vec4(0.1,\n" \
	"					0.2,\n" \
	"					0.2, 0.5);\n" \
	"	float hue, p2, p3;\n" \
	"	if (label > uint(0))\n" \
	"	{\n" \
	"		uint cv = label % uint(0xfd);\n" \
	"		uint si = uint(loc5.w);\n" \
	"		cv = ((cv << si) << 24 >> 24) | (cv >> (8 - si));\n" \
	"		hue = float(cv)/45.0;\n" \
	"		p2 = 1.0 - hue + floor(hue);\n" \
	"		p3 = hue - floor(hue);\n" \
	"		if (hue < 1.0)\n" \
	"			sel = vec4(1.0, p3, 0.0, 1.0);\n" \
	"		else if (hue < 2.0)\n" \
	"			sel = vec4(p2, 1.0, 0.0, 1.0);\n" \
	"		else if (hue < 3.0)\n" \
	"			sel = vec4(0.0, 1.0, p3, 1.0);\n" \
	"		else if (hue < 4.0)\n" \
	"			sel = vec4(0.0, p2, 1.0, 1.0);\n" \
	"		else if (hue < 5.0)\n" \
	"			sel = vec4(p3, 0.0, 1.0, 1.0);\n" \
	"		else\n" \
	"			sel = vec4(1.0, 0.0, p2, 1.0);\n" \
	"	}\n" \
	"\n"
#endif

#endif//VolShaderCode_h
