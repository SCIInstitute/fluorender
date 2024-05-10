//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#ifndef LightFieldShaderCode_h
#define LightFieldShaderCode_h

#define LIGHT_FIELD_SHADER_VERTEX \
"layout(location = 0)\n\
in vec2 vertPos_data;\n\
\n\
out vec2 texCoords;\n\
\n\
void main()\n\
{\n\
	gl_Position = vec4(vertPos_data.xy, 0.0, 1.0);\n\
	texCoords = (vertPos_data.xy + 1.0) * 0.5;\n\
}\n"

#define LIGHT_FIELD_SHADER_FRAG \
"in vec2 texCoords;\n\
out vec4 fragColor;\n\
\n\
uniform vec4 loc0;//pitch, tilt, center, subp\n\
uniform vec4 loc1;//viewPortion, displayAspect, quiltAspect\n\
uniform vec4 loc2;//tile, overscan\n\
uniform ivec4 lci0;//invView, ri, bi, quiltInvert\n\
uniform uint loci0;//debug\n\
uniform sampler2D tex0;//screenTex\n\
\n\
vec2 texArr(vec3 uvz)\n\
{\n\
	// decide which section to take from based on the z.\n\
	float x = (mod(uvz.z, loc2.x) + uvz.x) / loc2.x;\n\
	float y = (floor(uvz.z / loc2.x) + uvz.y) / loc2.y;\n\
	return vec2(x, y) * loc1.xy;\n\
}\n\
\n\
// recreate CG clip function (clear pixel if any component is negative)\n\
void clip(vec3 toclip)\n\
{\n\
	if (any(lessThan(toclip, vec3(0, 0, 0)))) discard;\n\
}\n\
\n\
void main()\n\
{\n\
	if (loci0 == 1)\n\
	{\n\
		fragColor = texture(tex0, texCoords.xy);\n\
	}\n\
	else\n\
	{\n\
		float invert = 1.0;\n\
		if (lci0.x + lci0.w == 1) invert = -1.0;\n\
		vec3 nuv = vec3(texCoords.xy, 0.0);\n\
		nuv -= 0.5;\n\
		float modx = clamp(step(loc1.w, loc1.z) * step(loc2.w, 0.5) + step(loc1.z, loc1.w) * step(0.5, loc2.w), 0, 1);\n\
		nuv.x = modx * nuv.x * loc1.z / loc1.w + (1.0 - modx) * nuv.x;\n\
		nuv.y = modx * nuv.y + (1.0 - modx) * nuv.y * loc1.w / loc1.z;\n\
		nuv += 0.5;\n\
		clip(nuv);\n\
		clip(1.0 - nuv);\n\
		vec4 rgb[3];\n\
		for (int i = 0; i < 3; i++)\n\
		{\n\
			nuv.z = (texCoords.x + i * loc0.w + texCoords.y * loc0.y) * loc0.x - loc0.z;\n\
			nuv.z = mod(nuv.z + ceil(abs(nuv.z)), 1.0);\n\
			nuv.z *= invert;\n\
			nuv.z *= loc2.z;\n\
			vec3 coords1 = nuv;\n\
			vec3 coords2 = nuv;\n\
			coords1.y = coords2.y = clamp(nuv.y, 0.005, 0.995);\n\
			coords1.z = floor(nuv.z);\n\
			coords2.z = ceil(nuv.z);\n\
			vec4 col1 = texture(tex0, texArr(coords1));\n\
			vec4 col2 = texture(tex0, texArr(coords2));\n\
			rgb[i] = mix(col1, col2, nuv.z - coords1.z);\n\
		}\n\
		fragColor = vec4(rgb[lci0.y].r, rgb[1].g, rgb[lci0.z].b, 1.0);\n\
	}\n\
}\n"

#endif//LightFieldShaderCode_h