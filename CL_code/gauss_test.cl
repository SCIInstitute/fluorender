#define KX 3
#define KY 3
#define KZ 3
float gauss(int i, int j, int k)
{
	//2*sigma*sigma
	float s = 10;
	float r = i*i+j*j+k*k;
	return exp(-r/s)/pow(3.1415*s, 1.5);
}
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
	for (int i=0; i<KX; ++i)
	for (int j=0; j<KY; ++j)
	for (int k=0; k<KZ; ++k)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z+(k-KZ/2), 1);
		dvalue = read_imagef(data, samp, kc);
		rvalue += gauss(i-KX/2, j-KY/2, k-KZ/2) * dvalue.x;
	}
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = rvalue*255.0;
}