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

namespace FLIVR
{
#define VOL_VERSION_130 \
	"#version 130\n"\
	"\n"

#define VOL_UNIFORMS_COMMON \
	"// VOL_UNIFORMS_COMMON\n" \
	"uniform vec4 loc0;//(lx, ly, lz, alpha)\n" \
	"uniform vec4 loc1;//(ka, kd, ks, ns)\n" \
	"uniform vec4 loc2;//(scalar_scale, gm_scale, left_thresh, right_thresh)\n" \
	"uniform vec4 loc3;//(gamma, gm_thresh, offset, sw)\n" \
	"uniform vec4 loc4;//(1/nx, 1/ny, 1/nz, 1/sample_rate)\n" \
	"uniform vec4 loc5;//(spcx, spcy, spcz, 1.0)\n" \
	"\n" \
	"uniform sampler3D tex0;//data volume\n" \
	"uniform sampler3D tex1;//gm volume\n" \
	"\n"

#define VOL_UNIFORMS_MATRICES \
	"// VOL_UNIFORMS_MATRICES\n" \
	"uniform mat4 matrix2;//tex transform for bricking\n" \
	"\n"

#define VOL_UNIFORMS_SIN_COLOR \
	"//VOL_UNIFORMS_SIN_COLOR\n" \
	"uniform vec4 loc6;//(red, green, blue, mask_threshold)\n" \
	"\n"

#define VOL_UNIFORMS_COLORMAP \
	"//VOL_UNIFORMS_COLORMAP\n" \
	"uniform vec4 loc6;//(low, hi, hi-lo, 0)\n" \
	"\n"

#define VOL_UNIFORMS_2DMAP_LOC \
	"//VOL_UNIFORMS_2DMAP_LOC\n" \
	"uniform vec4 loc7;//(1/vx, 1/vy, 0, 0)\n" \
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
	"	vec4 t = gl_TexCoord[0];\n" \
	"\n"

#define VOL_HEAD_2DMAP_LOC \
	"	//VOL_HEAD_2DMAP_LOC\n" \
	"	vec2 fcf = vec2(gl_FragCoord.x*loc7.x, gl_FragCoord.y*loc7.y);\n" \
	"\n"

#define VOL_HEAD_DP_1 \
	"	//VOL_HEAD_DP_NEG\n" \
	"	if (texture2D(tex15, fcf).r < gl_FragCoord.z) discard;\n" \
	"\n"

#define VOL_HEAD_DP_2 \
	"	//VOL_HEAD_DP_POS\n" \
	"	if (texture2D(tex15, fcf).r > gl_FragCoord.z) discard;\n" \
	"\n"

#define VOL_HEAD_DP_3 \
	"	//VOL_HEAD_DP_BOTH\n" \
	"	if (texture2D(tex15, fcf).r < gl_FragCoord.z) discard;\n" \
	"	else if (texture2D(tex14, fcf).r > gl_FragCoord.z) discard;\n" \
	"\n"

#define VOL_HEAD_FOG \
	"	//VOL_HEAD_FOG\n" \
	"	vec4 fc = gl_Fog.color;\n" \
	"	vec4 fp;\n" \
	"	fp.x = gl_Fog.density;\n" \
	"	fp.y = gl_Fog.start;\n" \
	"	fp.z = gl_Fog.end;\n" \
	"	fp.w = gl_Fog.scale;\n" \
	"	vec4 tf = gl_TexCoord[1];\n" \
	"	vec4 fctmp;\n" \
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
	"		discard;//gl_FragColor = vec4(0.0);\n" \
	"		return;\n" \
	"	}\n" \
	"\n"

#define VOL_HEAD_CLIP_FUNC \
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
	"	vec4 k = loc1; // {ka, kd, ks, ns}\n" \
	"	k.x = k.x>1.0?log2(3.0-k.x):k.x;\n" \
	"	vec4 n, w;\n" \
	"\n"

#define VOL_TAIL \
	"//VOL_TAIL\n" \
	"}\n" \
	"\n"

#define VOL_DATA_VOLUME_LOOKUP \
	"	//VOL_DATA_VOLUME_LOOKUP\n" \
	"	vec4 v = texture3D(tex0, t.stp);\n" \
	"\n"

#define VOL_DATA_VOLUME_LOOKUP_130 \
	"	//VOL_DATA_VOLUME_LOOKUP_130\n" \
	"	vec4 v = texture(tex0, t.stp);\n" \
	"\n"

#define VOL_GRAD_COMPUTE_LO \
	"	//VOL_GRAD_COMPUTE_LO\n" \
	"	vec4 dir = loc4; // \n" \
	"	vec4 r, p; \n" \
	"	mat4 tmat = gl_TextureMatrixInverseTranspose[0]; \n" \
	"	v = vec4(v.x); \n" \
	"	n = vec4(0.0); \n" \
	"	w = vec4(0.0);\n" \
	"	w.x = dir.x; \n" \
	"	p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.x = v.x - r.x; \n" \
	"	w = vec4(0.0); \n" \
	"	w.y = dir.y; \n" \
	"	p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.y = v.y - r.x; \n" \
	"	w = vec4(0.0); \n" \
	"	w.z = dir.x<dir.z?dir.x:dir.z; \n" \
	"	p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.z = v.z - r.x; \n" \
	"	w.x = dot(n.xxx, vec3(tmat[0].x, tmat[1].x, tmat[2].x)); \n" \
	"	w.y = dot(n.yyy, vec3(tmat[0].y, tmat[1].y, tmat[2].y)); \n" \
	"	w.z = dot(n.zzz, vec3(tmat[0].z, tmat[1].z, tmat[2].z)); \n" \
	"	p.y = length(w.xyz); \n" \
	"	p.y = 0.87 * (loc2.x<0.0?(1.0+p.y*loc2.x):p.y*loc2.x); \n" \
	"	n.xyz = w.xyz; \n" \
	"\n"

#define VOL_GRAD_COMPUTE_HI \
	"	// VOL_GRAD_COMPUTE_HI\n" \
	"	vec4 dir = loc4;//(1/nx, 1/ny, 1/nz, 1/sample_rate)\n" \
	"	vec4 r, p; \n" \
	"	mat4 tmat = gl_TextureMatrixInverseTranspose[0]; \n" \
	"	v = vec4(v.x); \n" \
	"	n = vec4(0.0); \n" \
	"	w = vec4(0.0);\n" \
	"	w.x = dir.x; \n" \
	"	p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.x = r.x + n.x; \n" \
	"	p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.x = r.x - n.x; \n" \
	"	w = vec4(0.0); \n" \
	"	w.y = dir.y; \n" \
	"	p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.y = r.x + n.y; \n" \
	"	p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.y = r.x - n.y; \n" \
	"	w = vec4(0.0); \n" \
	"	w.z = dir.x<dir.z?dir.x:dir.z; \n" \
	"	p = clamp(gl_TexCoord[0] + w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.z = r.x + n.z; \n" \
	"	p = clamp(gl_TexCoord[0] - w, 0.0, 1.0); \n" \
	"	r = texture3D(tex0, p.stp); \n" \
	"	n.z = r.x - n.z; \n" \
	"	w.x = dot(n.xxx, vec3(tmat[0].x, tmat[1].x, tmat[2].x)); \n" \
	"	w.y = dot(n.yyy, vec3(tmat[0].y, tmat[1].y, tmat[2].y)); \n" \
	"	w.z = dot(n.zzz, vec3(tmat[0].z, tmat[1].z, tmat[2].z)); \n" \
	"	p.y = length(w.xyz); \n" \
	"	p.y = 0.5 * (loc2.x<0.0?(1.0+p.y*loc2.x):p.y*loc2.x); \n" \
	"	n.xyz = w.xyz; \n" \
	"\n"

#define VOL_GRAD_COMPUTE_FUNC \
	"// VOL_GRAD_COMPUTE_FUNC\n" \
	"vec4 vol_grad_func(vec4 pos, vec4 dir)\n" \
	"{\n" \
	"	vec4 r, p;\n" \
	"	mat4 tmat = gl_TextureMatrixInverseTranspose[0];\n" \
	"	vec4 n = vec4(0.0);\n" \
	"	vec4 w = vec4(0.0);\n" \
	"	w.x = dir.x;\n" \
	"	p = clamp(pos+w, 0.0, 1.0);\n" \
	"	r = texture3D(tex0, p.stp);\n" \
	"	n.x = r.x + n.x;\n" \
	"	p = clamp(pos-w, 0.0, 1.0);\n" \
	"	r = texture3D(tex0, p.stp);\n" \
	"	n.x = r.x - n.x;\n" \
	"	w = vec4(0.0);\n" \
	"	w.y = dir.y;\n" \
	"	p = clamp(pos+w, 0.0, 1.0);\n" \
	"	r = texture3D(tex0, p.stp);\n" \
	"	n.y = r.x + n.y;\n" \
	"	p = clamp(pos-w, 0.0, 1.0);\n" \
	"	r = texture3D(tex0, p.stp);\n" \
	"	n.y = r.x - n.y;\n" \
	"	w = vec4(0.0);\n" \
	"	w.z = dir.x<dir.z?dir.x:dir.z;\n" \
	"	p = clamp(pos+w, 0.0, 1.0);\n" \
	"	r = texture3D(tex0, p.stp);\n" \
	"	n.z = r.x + n.z;\n" \
	"	p = clamp(pos-w, 0.0, 1.0);\n" \
	"	r = texture3D(tex0, p.stp);\n" \
	"	n.z = r.x - n.z;\n" \
	"	w.x = dot(n.xxx, vec3(tmat[0].x, tmat[1].x, tmat[2].x)); \n" \
	"	w.y = dot(n.yyy, vec3(tmat[0].y, tmat[1].y, tmat[2].y)); \n" \
	"	w.z = 0.3*dot(n.zzz, vec3(tmat[0].z, tmat[1].z, tmat[2].z)); \n" \
	"	return w;\n" \
	"}\n"

#define VOL_BODY_SHADING \
	"	//VOL_BODY_SHADING\n" \
	"	n.xyz = normalize(n.xyz);\n" \
	"	n.w = dot(l.xyz, n.xyz); // calculate angle between light and normal. \n" \
	"	n.w = clamp(abs(n.w), 0.0, 1.0); // two-sided lighting, n.w = abs(cos(angle))  \n" \
	"	w = k; // w.x = weight*ka, w.y = weight*kd, w.z = weight*ks \n" \
	"	w.x = k.x - w.y; // w.x = ka - kd*weight \n" \
	"	w.x = w.x + k.y; // w.x = ka + kd - kd*weight \n" \
	"	n.z = pow(n.w, k.w); // n.z = abs(cos(angle))^ns \n" \
	"	n.w = (n.w * w.y) + w.x; // n.w = abs(cos(angle))*kd+ka\n" \
	"	n.z = w.z * n.z; // n.z = weight*ks*abs(cos(angle))^ns \n" \
	"\n"

#define VOL_COMPUTED_GM_LOOKUP \
	"	//VOL_COMPUTED_GM_LOOKUP\n" \
	"	v.y = p.y;\n" \
	"\n"

#define VOL_COMPUTED_GM_INVALIDATE \
	"	//VOL_COMPUTED_GM_INVALIDATE\n" \
	"	v.y = loc3.y;\n" \
	"\n"

#define VOL_TEXTURE_GM_LOOKUP \
	"	//VOL_TEXTURE_GM_LOOKUP\n" \
	"	v.y = texture3D(tex1, t.stp).x;\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR \
	"	//VOL_TRANSFER_FUNCTION_SIN_COLOR\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	float alpha = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y-loc3.w)\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		v.x = (v.y<loc3.y?(loc3.w-loc3.y+v.y)/loc3.w:1.0)*v.x;\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
	"		alpha = 1.0 - pow(clamp(1.0-tf_alp, 0.0, 1.0), loc4.w);\n" \
	"		c = vec4(loc6.rgb*alpha*tf_alp, alpha);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID \
	"	//VOL_TRANSFER_FUNCTION_SIN_COLOR_SOLID\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y)\n" \
	"		c = vec4(0.0, 0.0, 0.0, 1.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
	"		c = vec4(loc6.rgb*tf_alp, 1.0);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_SIN_COLOR_L \
	"	//VOL_TRANSFER_FUNCTION_SIN_COLOR_L\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y)\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
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
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y)\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
	"		c = vec4(tf_alp);\n" \
	"	}\n" \
	"	return c;\n" \
	"}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP \
	"	//VOL_TRANSFER_FUNCTION_COLORMAP\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y)\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		vec4 rb = vec4(0.0);\n" \
	"		float valu = (v.x-loc6.x)/loc6.z;\n" \
	"		rb.r = clamp(4.0*valu - 2.0, 0.0, 1.0);\n" \
	"		rb.g = clamp(valu<0.5 ? 4.0*valu : -4.0*valu+4.0, 0.0, 1.0);\n" \
	"		rb.b = clamp(-4.0*valu+2.0, 0.0, 1.0);\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
	"		float alpha = 1.0 - pow(1.0-tf_alp, loc4.w);\n" \
	"		c = vec4(rb.rgb*alpha*tf_alp, alpha);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_COLORMAP_SOLID \
	"	//VOL_TRANSFER_FUNCTION_COLORMAP_SOLID\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y)\n" \
	"		c = vec4(0.0, 0.0, 0.0, 1.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		vec4 rb = vec4(0.0);\n" \
	"		float valu = (v.x-loc6.x)/loc6.z;\n" \
	"		rb.r = clamp(4.0*valu - 2.0, 0.0, 1.0);\n" \
	"		rb.g = clamp(valu<0.5 ? 4.0*valu : -4.0*valu+4.0, 0.0, 1.0);\n" \
	"		rb.b = clamp(-4.0*valu+2.0, 0.0, 1.0);\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
	"		c = vec4(rb.rgb*tf_alp, 1.0);\n" \
	"	}\n" \
	"\n"

#define VOL_TRANSFER_FUNCTION_DEPTHMAP \
	"	//VOL_TRANSFER_FUNCTION_DEPTHMAP\n" \
	"	vec4 c;\n" \
	"	float tf_alp = 0.0;\n" \
	"	v.x = loc2.x<0.0?(1.0+v.x*loc2.x):v.x*loc2.x;\n" \
	"	if (v.x<loc2.z-loc3.w || v.x>loc2.w+loc3.w || v.y<loc3.y)\n" \
	"		c = vec4(0.0);\n" \
	"	else\n" \
	"	{\n" \
	"		v.x = (v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0))*v.x;\n" \
	"		tf_alp = pow(clamp(v.x/loc3.z,\n" \
	"			loc3.x<1.0?-(loc3.x-1.0)*0.00001:0.0,\n" \
	"			loc3.x>1.0?0.9999:1.0), loc3.x);\n" \
	"		float alpha = tf_alp;\n" \
	"		c = vec4(vec3(alpha*tf_alp), alpha);\n" \
	"	}\n" \
	"\n"

#define VOL_COLOR_OUTPUT \
	"	//VOL_COLOR_OUTPUT\n" \
	"	c.xyz = c.xyz*clamp(1.0-loc1.x, 0.0, 1.0) + loc1.x*c.xyz*(loc1.y > 0.0?(n.w + n.z):1.0);\n" \
	"	c.xyz *= pow(1.0 - loc1.x/2.0, 2.0) + 1.0;\n" \
	"\n"

#define VOL_FOG_BODY \
	"	//VOL_FOG_BODY\n" \
	"	v.x = fp.z - tf.x;\n" \
	"	v.x = (1.0 - clamp(v.x * fp.w, 0.0, 1.0))*fp.x;\n" \
	"	fctmp = c.w * fc;\n" \
	"	c.xyz = mix(fctmp.xyzz, c.xyzz, (1.0-v.x)).xyz; \n" \
	"\n"

#define VOL_RASTER_BLEND \
	"	//VOL_RASTER_BLEND\n" \
	"	gl_FragColor = c*l.w; // VOL_RASTER_BLEND\n" \
	"\n"

#define VOL_RASTER_BLEND_SOLID \
	"	//VOL_RASTER_BLEND_SOLID\n" \
	"	gl_FragColor = c;\n" \
	"\n"

#define VOL_RASTER_BLEND_DMAP \
	"	//VOL_RASTER_BLEND_DMAP\n" \
	"	//float prevz = texture2D(tex4, fcf).r;\n" \
	"	float currz = gl_FragCoord.z;\n" \
	"	float intpo = (c*l.w).r;\n" \
	"	//gl_FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);\n" \
	"	if (intpo < 0.05) discard;\n" \
	"	gl_FragColor = vec4(vec3(currz), 1.0);\n" \
	"\n"

#define VOL_RASTER_BLEND_NOMASK \
	"	//VOL_RASTER_BLEND_NOMASK\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	gl_FragColor = vec4(1.0-cmask.x)*c*l.w;\n" \
	"\n"

#define VOL_RASTER_BLEND_NOMASK_SOLID \
	"	//VOL_RASTER_BLEND_NOMASK_SOLID\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	gl_FragColor = vec4(1.0-cmask.x)*c;\n" \
	"\n"

#define VOL_RASTER_BLEND_NOMASK_DMAP \
	"	//VOL_RASTER_BLEND_NOMASK_DMAP\n" \
	"	float prevz = texture2D(tex4, fcf).r;\n" \
	"	float currz = gl_FragCoord.z;\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	float intpo = (vec4(1.0-cmask.x)*c*l.w).r;\n" \
	"	gl_FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);\n" \
	"\n"

#define VOL_RASTER_BLEND_MASK \
	"	//VOL_RASTER_BLEND_MASK\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	gl_FragColor = tf_alp*cmask.x<loc6.w?vec4(0.0):vec4(cmask.x)*c*l.w;\n" \
	"\n"

#define VOL_RASTER_BLEND_MASK_SOLID \
	"	//VOL_RASTER_BLEND_MASK_SOLID\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	gl_FragColor = tf_alp*cmask.x<loc6.w?vec4(0.0):vec4(cmask.x)*c;\n" \
	"\n"

#define VOL_RASTER_BLEND_MASK_DMAP \
	"	//VOL_RASTER_BLEND_MASK_DMAP\n" \
	"	float prevz = texture2D(tex4, fcf).r;\n" \
	"	float currz = gl_FragCoord.z;\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	float intpo = (vec4(cmask.x)*c*l.w).r;\n" \
	"	gl_FragColor = vec4(vec3(intpo>0.05?currz:prevz), 1.0);\n" \
	"\n"

#define VOL_RASTER_BLEND_LABEL \
	"	//VOL_RASTER_BLEND_LABEL\n" \
	"	uint label = texture(tex3, t.stp).x; //get mask value\n" \
	"	vec4 sel = vec4(0.0,\n" \
	"					0.0,\n" \
	"					0.0, 0.0);\n" \
	"	float hue, p2, p3;\n" \
	"	if (label > uint(0))\n" \
	"	{\n" \
	"		hue = float(label%uint(360))/60.0;\n" \
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
	"	gl_FragColor = sel;\n" \
	"\n"

#define VOL_RASTER_BLEND_LABEL_MASK \
	"	//VOL_RASTER_BLEND_LABEL_MASK\n" \
	"	vec4 cmask = texture3D(tex2, t.stp); //get mask value\n" \
	"	if (cmask.x <= loc6.w)\n" \
	"	{\n" \
	"		gl_FragColor = c*l.w;\n" \
	"		return;\n" \
	"	}\n" \
	"	uint label = texture(tex3, t.stp).x; //get mask value\n" \
	"	vec4 sel = vec4(1.0-loc6.x,\n" \
	"					1.0-loc6.y,\n" \
	"					1.0-loc6.z, 1.0);\n" \
	"	float hue, p2, p3;\n" \
	"	if (label > uint(0))\n" \
	"	{\n" \
	"		hue = float(label%uint(360))/60.0;\n" \
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
	"	gl_FragColor = sel*alpha*tf_alp;\n" \
	"\n"

}//namespace FLIVR
