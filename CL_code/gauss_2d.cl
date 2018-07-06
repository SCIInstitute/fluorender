#define KX 3
#define KY 3
__constant float krn[KX*KY] =
{0.108796548095085, 0.112250121255763, 0.108796548095085,
 0.112250121255763, 0.115813322596608, 0.112250121255763,
 0.108796548095085, 0.112250121255763, 0.108796548095085};
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;
__kernel void kernel_main(
	read_only image3d_t data,
	__global unsigned char* result,
	unsigned int x,
	unsigned int y,
	unsigned int z)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	int4 kc;
	float4 dvalue;
	float rvalue = 0.0;
	int i, j;
	for (i=0; i<KX; ++i)
	for (j=0; j<KY; ++j)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z, 1);
		dvalue = read_imagef(data, samp, kc);
		rvalue += krn[KX*j+i] * dvalue.x;
	}
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = clamp(rvalue, 0.0f, 1.0f)*255.0;
}