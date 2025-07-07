#pragma once
enum class PixelFormats
{
	NoFormat     = 0x0,
	RGB          = 0x1907,
	RGBA         = 0x1908,
	BGRA         = 0x80E1,
	Red          = 0x1903,
	RGB_DXT1     = 0x83F0,
	RGBA_DXT5    = 0x83F3,
	YCoCg_DXT5   = 0x01,
	A_RGTC1      = 0x8DBB,
    SRGB         = 0x8C41,
    SRGB_A       = 0x8C43,
	R32F         = 0x822E,
	RGBA32F      = 0x8814,
	RGBA_INTEGER = 0x8D99,
	RGB10_A2	 = 0x8059
};
