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

#ifndef LightFieldShaderCode_h
#define LightFieldShaderCode_h

inline constexpr const char* LIGHT_FIELD_SHADER_VERTEX = R"GLSHDR(
//LIGHT_FIELD_SHADER_VERTEX
layout(location = 0) in vec3 InVertex;
layout(location = 1) in vec3 InTexCoord;
out vec3 OutVertex;
out vec3 OutTexCoord;

void main()
{
	gl_Position = vec4(InVertex, 1.0);
	OutTexCoord = InTexCoord;
	OutVertex = InVertex;
}
)GLSHDR";

inline constexpr const char* LIGHT_FIELD_SHADER_FRAG = R"GLSHDR(
//LIGHT_FIELD_SHADER_FRAG
in vec3 OutVertex;
in vec3 OutTexCoord;
out vec4 fragColor;

uniform vec4 loc0;//pitch, tilt, center, subp {246.87834, -0.18404308, 0.24427177, 0.00021701389}
uniform vec4 loc1;//viewPortion, displayAspect, quiltAspect {0.99975586, 0.99975586, 0.75, 0.75}
uniform vec4 loc2;//tile, overscan {5, 9, 45, 0}
uniform ivec4 lci0;//invView, ri, bi, quiltInvert {1, 0, 2, 0}
uniform uint loci0;//debug {0}
uniform sampler2D tex0;//screenTex

vec2 texArr(vec3 uvz)
{
	// decide which section to take from based on the z.
	float x = (mod(uvz.z, loc2.x) + uvz.x) / loc2.x;
	float y = (floor(uvz.z / loc2.x) + uvz.y) / loc2.y;
	return vec2(x, y) * loc1.xy;
}

// recreate CG clip function (clear pixel if any component is negative)
void clip(vec3 toclip)
{
	if (any(lessThan(toclip, vec3(0, 0, 0)))) discard;
}

void main()
{
	if (loci0 == 1)
	{
		fragColor = texture(tex0, OutTexCoord.xy);
	}
	else
	{
		float invert = 1.0;
		if (lci0.x + lci0.w == 1) invert = -1.0;
		vec3 nuv = vec3(OutTexCoord.xy, 0.0);
		nuv -= 0.5;
		float modx = clamp(step(loc1.w, loc1.z) * step(loc2.w, 0.5) + step(loc1.z, loc1.w) * step(0.5, loc2.w), 0, 1);
		nuv.x = modx * nuv.x * loc1.z / loc1.w + (1.0 - modx) * nuv.x;
		nuv.y = modx * nuv.y + (1.0 - modx) * nuv.y * loc1.w / loc1.z;
		nuv += 0.5;
		clip(nuv);
		clip(1.0 - nuv);
		vec4 rgb[3];
		for (int i = 0; i < 3; i++)
		{
			nuv.z = (OutTexCoord.x + i * loc0.w + OutTexCoord.y * loc0.y) * loc0.x - loc0.z;
			nuv.z = mod(nuv.z + ceil(abs(nuv.z)), 1.0);
			nuv.z *= invert;
			nuv.z *= loc2.z;
			vec3 coords1 = nuv;
			vec3 coords2 = nuv;
			coords1.y = coords2.y = clamp(nuv.y, 0.005, 0.995);
			coords1.z = floor(nuv.z);
			coords2.z = ceil(nuv.z);
			vec4 col1 = texture(tex0, texArr(coords1));
			vec4 col2 = texture(tex0, texArr(coords2));
			rgb[i] = mix(col1, col2, nuv.z - coords1.z);
		}
		fragColor = vec4(rgb[lci0.y].r, rgb[1].g, rgb[lci0.z].b, 1.0);
	}
}
)GLSHDR";

#endif//LightFieldShaderCode_h