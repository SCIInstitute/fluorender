#define KX 3
#define KY 3
#define KZ 3
#define TH 0.1
#define DWL unsigned char
#define VSCL 255
bool check_loop(bool loop[])
{
	int count = loop[0]?1:0;
	int count_chg = 0;
	for (int i=1; i<8; ++i)
	{
		count += loop[i]?1:0;
		if (!loop[i-1] && loop[i])
			count_chg++;
	}
	if (!loop[7] && loop[0])
		count_chg++;
	return count>2 && count<8 && count_chg==1;
}
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
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	//check 5 loops
	bool loop[8];
	//first
	loop[0] = nbs[1]; loop[1] = nbs[4]; loop[2] = nbs[7]; loop[3] = nbs[16];
	loop[4] = nbs[25]; loop[5] = nbs[22]; loop[6] = nbs[19]; loop[7] = nbs[10];
	if (!check_loop(loop))
	{
		result[index] = value*VSCL;
		return;
	}
	//second
	loop[0] = nbs[0]; loop[1] = nbs[3]; loop[2] = nbs[6]; loop[3] = nbs[16];
	loop[4] = nbs[26]; loop[5] = nbs[23]; loop[6] = nbs[20]; loop[7] = nbs[10];
	if (!check_loop(loop))
	{
		result[index] = value*VSCL.0;
		return;
	}
	//third
	loop[0] = nbs[9]; loop[1] = nbs[12]; loop[2] = nbs[15]; loop[3] = nbs[16];
	loop[4] = nbs[17]; loop[5] = nbs[14]; loop[6] = nbs[11]; loop[7] = nbs[10];
	if (!check_loop(loop))
	{
		result[index] = value*VSCL.0;
		return;
	}
	//fourth
	loop[0] = nbs[18]; loop[1] = nbs[21]; loop[2] = nbs[24]; loop[3] = nbs[16];
	loop[4] = nbs[8]; loop[5] = nbs[5]; loop[6] = nbs[2]; loop[7] = nbs[10];
	if (!check_loop(loop))
	{
		result[index] = value*VSCL.0;
		return;
	}
	//fifth
	loop[0] = nbs[3]; loop[1] = nbs[4]; loop[2] = nbs[5]; loop[3] = nbs[14];
	loop[4] = nbs[23]; loop[5] = nbs[22]; loop[6] = nbs[21]; loop[7] = nbs[12];
	if (!check_loop(loop))
	{
		result[index] = value*VSCL.0;
		return;
	}
	result[index] = 0.0;
}