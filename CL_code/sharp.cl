#define KX 3
#define KY 3
#define KZ 3
#define MAX 0.03
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
	int i, j, k;
	dvalue = read_imagef(data, samp, coord);
	float cvalue = dvalue.x;
	for (i=0; i<KX; ++i)
	for (j=0; j<KY; ++j)
	for (k=0; k<KZ; ++k)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z+(k-KZ/2), 1);
		dvalue = read_imagef(data, samp, kc);
		rvalue += fabs(dvalue.x-cvalue);
	}
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	rvalue /= KX*KY*KZ;
	if (rvalue > MAX)
		rvalue = 1.0;
	else
	{
		rvalue -= MAX;
		rvalue *= rvalue;
		rvalue = -rvalue/0.01;
		rvalue = exp(rvalue);
	}
	result[index] = clamp(cvalue*rvalue, 0.0f, 1.0f)*255.0;
}