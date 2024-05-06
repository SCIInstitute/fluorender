/*
    HoloPlayCore 0.1.1

    include/HoloPlayShaders.h

    author: Evan Kahn
    contact: evan@lookingglassfactory.com

     This file contains static string definitions for the lenticular shader
    that turns a "quilt" texture containing a multistereo image into Looking Glass
    lenticular output. The lenticular shader depends on two sets of uniforms.

     The first set of uniforms (calibration values) varies from device to device,
    and must be queried using functions declared in HoloPlayCore.h. The second set
    (quilt settings) must be provided by the developer and depends on the desired
    resolution of their multiview render.

    By using, copying, or modifying this code in any way, you agree to the terms of
    https://github.com/Looking-Glass/HoloPlayCoreSDK/blob/master/LICENSE.txt

    The contents of this file are not open source and should not be merged into
    codebases with licenses that conflict with the HoloPlay SDK License. In those
    cases, please include HoloPlayShadersOpen.h instead.
*/

static const char* hpc_LightfieldVertShaderGLSL = "\
layout (location = 0)\n\
in vec2 vertPos_data;\n\
\n\
out vec2 texCoords;\n\
\n\
void main()\n\
{\n\
	gl_Position = vec4(vertPos_data.xy, 0.0, 1.0);\n\
	texCoords = (vertPos_data.xy + 1.0) * 0.5;\n\
}";
static const char* hpc_LightfieldFragShaderGLSL = "\
in vec2 texCoords;\n\
out vec4 fragColor;\n\
\n\
// Calibration values\n\
uniform float pitch;\n\
uniform float tilt;\n\
uniform float center;\n\
uniform int invView;\n\
uniform float subp;\n\
uniform float displayAspect;\n\
uniform int ri;\n\
uniform int bi;\n\
\n\
// Quilt settings\n\
uniform vec3 tile;\n\
uniform vec2 viewPortion;\n\
uniform float quiltAspect;\n\
uniform int overscan;\n\
uniform int quiltInvert;\n\
\n\
uniform int debug;\n\
\n\
uniform sampler2D screenTex;\n\
\n\
vec2 texArr(vec3 uvz)\n\
{\n\
	// decide which section to take from based on the z.\n\
	float x = (mod(uvz.z, tile.x) + uvz.x) / tile.x;\n\
	float y = (floor(uvz.z / tile.x) + uvz.y) / tile.y;\n\
	return vec2(x, y) * viewPortion.xy;\n\
}\n\
\n\
// recreate CG clip function (clear pixel if any component is negative)\n\
void clip(vec3 toclip)\n\
{\n\
	if (any(lessThan(toclip, vec3(0,0,0)))) discard;\n\
}\n\
\n\
void main()\n\
{\n\
	if (debug == 1)\n\
	{\n\
		fragColor = texture(screenTex, texCoords.xy);\n\
	}\n\
	else {\n\
		float invert = 1.0;\n\
		if (invView + quiltInvert == 1) invert = -1.0;\n\
		vec3 nuv = vec3(texCoords.xy, 0.0);\n\
		nuv -= 0.5;\n\
		float modx = clamp (step(quiltAspect, displayAspect) * step(float(overscan), 0.5) + step(displayAspect, quiltAspect) * step(0.5, float(overscan)), 0, 1);\n\
		nuv.x = modx * nuv.x * displayAspect / quiltAspect + (1.0-modx) * nuv.x;\n\
		nuv.y = modx * nuv.y + (1.0-modx) * nuv.y * quiltAspect / displayAspect; \n\
		nuv += 0.5;\n\
		clip (nuv);\n\
		clip (1.0-nuv);\n\
		vec4 rgb[3];\n\
		for (int i=0; i < 3; i++)\n\
		{\n\
			nuv.z = (texCoords.x + i * subp + texCoords.y * tilt) * pitch - center;\n\
			nuv.z = mod(nuv.z + ceil(abs(nuv.z)), 1.0);\n\
			nuv.z *= invert;\n\
			nuv.z *= tile.z;\n\
			vec3 coords1 = nuv;\n\
			vec3 coords2 = nuv;\n\
			coords1.y = coords2.y = clamp(nuv.y, 0.005, 0.995);\n\
			coords1.z = floor(nuv.z);\n\
			coords2.z = ceil(nuv.z);\n\
			vec4 col1 = texture(screenTex, texArr(coords1));\n\
			vec4 col2 = texture(screenTex, texArr(coords2));\n\
			rgb[i] = mix(col1, col2, nuv.z - coords1.z);\n\
		}\n\
		fragColor = vec4(rgb[ri].r, rgb[1].g, rgb[bi].b, 1.0);\n\
	}\n\
}\n\
";
