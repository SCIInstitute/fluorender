#define TH 0.3
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
	float4 dvalue = read_imagef(data, samp, coord);
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = dvalue.x>=TH?VSCL:0;
}