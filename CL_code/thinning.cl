#define KX 3
#define KY 3
#define KZ 3
#define TH 0.5
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_LINEAR;
__kernel void kernel_main(
	read_only image3d_t data,
	__global unsigned char* result,
	unsigned int x,
	unsigned int y,
	unsigned int z)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	float value = read_imagef(data, samp, coord).x;
	if (value < TH)
		return;
	bool nbs[KX*KY*KZ];
	int4 kc;
	float dvalue;
	int i, j, k;
	int count = 0;
	for (i=0; i<KX; ++i)
	for (j=0; j<KY; ++j)
	for (k=0; k<KZ; ++k)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z+(k-KZ/2), 1);
		dvalue = read_imagef(data, samp, kc).x;
		nbs[count] = dvalue>=TH?true:false;
		count++;
	}
	int pos1 = -1;
	int pos2 = -1;
	for (i=0; i<KX*KY*KZ; ++i)
	{
		if (!nbs[i] && pos1==-1)
			pos1 = i;
	}
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = 0.0;
}