#define TH 0.1
bool check_a(bool nbs[], bool ext[])
{
	if (nbs[14] && !nbs[0] && !nbs[3] && !nbs[6] && !nbs[9] && !nbs[12] && !nbs[15] && !nbs[18] && !nbs[21] && !nbs[24])
		return true;
	if (nbs[12] && ext[0] && !nbs[2] && !nbs[5] && !nbs[8] && !nbs[11] && !nbs[14] && !nbs[17] && !nbs[20] && !nbs[23] && !nbs[26])
		return true;
	if (nbs[4] && !nbs[18] && !nbs[19] && !nbs[20] && !nbs[21] && !nbs[22] && !nbs[23] && !nbs[24] && !nbs[25] && !nbs[26])
		return true;
	if (nbs[22] && ext[5] && !nbs[0] && !nbs[1] && !nbs[2] && !nbs[3] && !nbs[4] && !nbs[5] && !nbs[6] && !nbs[7] && !nbs[8])
		return true;
	if (nbs[16] && !nbs[0] && !nbs[1] && !nbs[2] && !nbs[9] && !nbs[10] && !nbs[11] && !nbs[18] && !nbs[19] && !nbs[20])
		return true;
	if (nbs[10] && ext[2] && !nbs[6] && !nbs[7] && !nbs[8] && !nbs[15] && !nbs[16] && !nbs[17] && !nbs[24] && !nbs[25] && !nbs[26])
		return true;
	return false;
}
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
	bool nbs[27];
	bool ext[6];
	int4 kc;
	float dvalue;
	int i, j, k;
	int count = 0;
	for (k=0; k<3; ++k)
	for (j=0; j<3; ++j)
	for (i=0; i<3; ++i)
	{
		kc = (int4)(coord.x+(i-1),
				coord.y+(j-1),
				coord.z+(k-1), 1);
		dvalue = read_imagef(data, samp, kc).x;
		nbs[count] = dvalue>=TH?true:false;
		count++;
	}
	//extended points
	kc = (int4)(coord.x-2, coord.y, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	ext[0] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+2, coord.y, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	ext[1] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-2, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	ext[2] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y+2, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	ext[3] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y, coord.z-2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	ext[4] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	ext[5] = dvalue>=TH?true:false;
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	//check 4 cases
	if (check_a(nbs, ext))
	{
		result[index] = 0.0;
		return;
	}

	result[index] = value*255.0;
}