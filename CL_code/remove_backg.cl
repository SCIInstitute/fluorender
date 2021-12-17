#define KX 40
#define KY 40
#define KN 1600
#define VTH 0.0001
#define GTH 2
#define GTH2 4
#define DWL unsigned char
#define VSCL 255
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;
__kernel void kernel_main(
	read_only image3d_t data,
	__global DWL* result,
	unsigned int x,
	unsigned int y,
	unsigned int z)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	int4 kc;
	float4 dvalue;
	dvalue = read_imagef(data, samp, coord);
	float cvalue = dvalue.x;
	float sumi = 0.0;
	float sumi2 = 0.0;
	int i, j, k;
	for (i=0; i<KX; ++i)
	for (j=0; j<KY; ++j)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z, 1);
		dvalue = read_imagef(data, samp, kc);
		sumi += dvalue.x;
		sumi2 += dvalue.x * dvalue.x;
	}
	float mean = sumi / KN;
	float var = sqrt((sumi2 + KN * mean * mean - 2.0 * mean * sumi) / KN);
	cvalue = (var < VTH) || (cvalue - mean < var * GTH) ? 0.0 : cvalue;
	cvalue = cvalue - mean > var * GTH2 ? cvalue - mean : cvalue;
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = cvalue * VSCL;
}