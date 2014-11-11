const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_REPEAT|
	CLK_FILTER_NEAREST;
__kernel void main(
	__global read_only image3d_t data,
	__global unsigned char* result,
	unsigned int x,
	unsigned int y,
	unsigned int z)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	float4 dvalue = read_imagef(data, samp, coord);
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = dvalue.x*255.0;
}