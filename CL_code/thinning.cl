#define TH 0.1
#define DWL unsigned char
#define VSCL 255
bool check_tail(bool n[])
{
	int count = 0;
	for (int i = 0; i < 27; ++i)
		count += n[i]?1:0;
	if (count == 2)
		return true;
	if (count == 3 && ((n[10] && n[14]) || (n[10] && n[22]) || (n[16] && n[12]) || (n[22] && n[12]) || (n[16] && n[4]) || (n[14] && n[4])))
		return true;
	return false;
}
bool check_a(bool n[], bool e[])
{
	if (n[14] && !n[0] && !n[3] && !n[6] && !n[9] && !n[12] && !n[15] && !n[18] && !n[21] && !n[24])
		return true;
	if (n[12] && e[0] && !n[2] && !n[5] && !n[8] && !n[11] && !n[14] && !n[17] && !n[20] && !n[23] && !n[26])
		return true;
	if (n[4] && !n[18] && !n[19] && !n[20] && !n[21] && !n[22] && !n[23] && !n[24] && !n[25] && !n[26])
		return true;
	if (n[22] && e[5] && !n[0] && !n[1] && !n[2] && !n[3] && !n[4] && !n[5] && !n[6] && !n[7] && !n[8])
		return true;
	if (n[16] && !n[0] && !n[1] && !n[2] && !n[9] && !n[10] && !n[11] && !n[18] && !n[19] && !n[20])
		return true;
	if (n[10] && e[2] && !n[6] && !n[7] && !n[8] && !n[15] && !n[16] && !n[17] && !n[24] && !n[25] && !n[26])
		return true;
	return false;
}
bool check_b(bool n[], bool e[])
{
	if (n[14] && n[22] && e[5] && !n[0] && !n[1] && !n[3] && !n[4] && !n[6] && !n[7] && !n[9] && !n[12] && !n[15])
		return true;
	if (n[12] && n[22] && e[0] && e[5] && !n[1] && !n[2] && !n[4] && !n[5] && !n[7] && !n[8] && !n[11] && !n[14] && !n[17])
		return true;
	if (n[14] && n[10] && e[2] && !n[3] && !n[6] && !n[7] && !n[12] && !n[15] && !n[16] && !n[21] && !n[24] && !n[25])
		return true;
	if (n[10] && n[12] && e[0] && e[2] && !n[5] && !n[7] && !n[8] && !n[14] && !n[16] && !n[17] && !n[23] && !n[25] && !n[26])
		return true;
	if (n[14] && n[16] && e[3] && !n[0] && !n[1] && !n[3] && !n[9] && !n[10] && !n[12] && !n[18] && !n[19] && !n[21])
		return true;
	if (n[12] && n[16] && e[0] && e[3] && !n[1] && !n[2] && !n[5] && !n[10] && !n[11] && !n[14] && !n[19] && !n[20] && !n[23])
		return true;
	if (n[4] && n[14] && e[4] && !n[9] && !n[12] && !n[15] && !n[18] && !n[19] && !n[21] && !n[22] && !n[24] && !n[25])
		return true;
	if (n[4] && n[12] && n[0] && n[4] && !n[11] && !n[14] && !n[17] && !n[19] && !n[20] && !n[22] && !n[23] && !n[25] && !n[26])
		return true;
	if (n[4] && n[10] && e[2] && !n[15] && !n[16] && !n[17] && !n[21] && !n[22] && !n[23] && !n[24] && !n[25] && !n[26])
		return true;
	if (n[10] && n[22] && e[2] && e[5] && !n[3] && !n[4] && !n[5] && !n[6] && !n[7] && !n[8] && !n[15] && !n[16] && !n[17])
		return true;
	if (n[4] && n[16] && e[3] && !n[9] && !n[10] && !n[11] && !n[18] && !n[19] && !n[20] && !n[21] && !n[22] && !n[23])
		return true;
	if (n[16] && n[22] && e[3] && e[5] && !n[0] && !n[1] && !n[2] && !n[3] && !n[4] && !n[5] && !n[9] && !n[10] && !n[11])
		return true;
	return false;
}
bool check_c(bool n[], bool e[])
{
	if (n[4] && n[12] && n[16] && e[0] && !n[10] && !n[11] && !n[14] && !n[19] && !n[20] && !n[22] && !n[23])
		return true;
	if (n[12] && n[16] && n[22] && e[0] && e[5] && !n[1] && !n[2] && !n[4] && !n[5] && !n[10] && !n[11] && !n[14])
		return true;
	if (n[4] && n[10] && n[12] && e[2] && !n[14] && !n[16] && !n[17] && !n[22] && !n[23] && !n[25] && !n[26])
		return true;
	if (n[10] && n[12] && n[22] && e[2] && e[5] && !n[4] && !n[5] && !n[7] && !n[8] && !n[14] && !n[16] && !n[17])
		return true;
	if (n[4] && n[10] && n[14] && e[1] && !n[12] && !n[15] && !n[16] && !n[21] && !n[22] && !n[24] && !n[25])
		return true;
	if (n[10] && n[14] && n[22] && e[1] && e[5] && !n[3] && !n[4] && !n[6] && !n[7] && !n[12] && !n[15] && !n[16])
		return true;
	if (n[4] && n[14] && n[16] && e[3] && !n[9] && !n[10] && !n[12] && !n[18] && !n[19] && !n[21] && !n[22])
		return true;
	if (n[14] && n[16] && n[22] && e[3] && e[5] && !n[0] && !n[1] && !n[3] && !n[4] && !n[9] && !n[10] && !n[12])
		return true;
	return false;
}
bool check_d(bool n[], bool e[], bool p[])
{
	if (n[5] && n[10] && n[16] && !n[3] && !n[4] && !n[12] && !n[14] && !n[21] && !n[22] && !n[23])
		return true;
	if (n[21] && (e[0] || e[2] || p[6] || p[7] || p[8]) &&
		!n[0] && !n[1] && !n[2] && !n[3] && !n[4] && !n[5] && !n[6] && !n[7] &&
		!n[8] && !n[11] && !n[12] && !n[14] && !n[17] && !n[20] && !n[22] && !n[23] && !n[26])
		return true;
	if (n[3] && n[10] && n[16] && !n[4] && !n[5] && !n[12] && !n[14] && !n[21] && !n[22] && !n[23])
		return true;
	if (n[23] && (e[1] || e[5] || p[9] || p[10] || p[11]) &&
		!n[0] && !n[1] && !n[2] && !n[3] && !n[4] && !n[5] && !n[6] && !n[7] &&
		!n[8] && !n[9] && !n[12] && !n[14] && !n[15] && !n[18] && !n[21] && !n[22] && !n[24])
		return true;
	if (n[4] && n[15] && n[22] && !n[9] && !n[10] && !n[11] && !n[12] && !n[14] && !n[16] && !n[17])
		return true;
	if (n[11] && (e[1] || e[2] || p[3] || p[4] || p[5]) &&
		!n[0] && !n[3] && !n[6] && !n[7] && !n[8] && !n[9] && !n[10] && !n[12] &&
		!n[14] && !n[15] && !n[16] && !n[17] && !n[18] && !n[21] && !n[24] && !n[25] && !n[26])
		return true;
	if (n[4] && n[9] && n[22] && !n[10] && !n[11] && !n[12] && !n[14] && !n[15] && !n[16] && !n[17])
		return true;
	if (n[17] && (e[1] || e[3] || p[0] || p[1] || p[2]) &&
		!n[0] && !n[1] && !n[2] && !n[3] && !n[6] && !n[9] && !n[10] && !n[11] &&
		!n[12] && !n[14] && !n[15] && !n[16] && !n[18] && !n[19] && !n[20] && !n[21] && !n[24])
		return true;
	if (n[7] && n[12] && n[14] && !n[1] && !n[4] && !n[10] && !n[16] && !n[19] && !n[22] && !n[25])
		return true;
	if (n[19] && (e[3] || e[4] || p[12] || p[13] || p[14]) &&
		!n[0] && !n[1] && !n[2] && !n[3] && !n[4] && !n[5] && !n[6] && !n[7] &&
		!n[8] && !n[10] && !n[15] && !n[16] && !n[17] && !n[22] && !n[24] && !n[25] && !n[26])
		return true;
	if (n[12] && n[14] && n[25] && !n[1] && !n[4] && !n[7] && !n[10] && !n[16] && !n[19] && !n[22])
		return true;
	if (n[1] && (e[3] || e[5] || p[15] || p[16] || p[17]) &&
		!n[4] && !n[6] && !n[7] && !n[8] && !n[10] && !n[15] && !n[16] && !n[17] &&
		!n[18] && !n[19] && !n[20] && !n[21] && !n[22] && !n[23] && !n[24] && !n[25] && !n[26])
		return true;
	return false;
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
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	float value = read_imagef(data, samp, coord).x;
	if (value < TH)
	{
		result[index] = 0.0;
		return;
	}
	bool nbs[27];
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
	//check tail
	if (check_tail(nbs))
	{
		result[index] = value*VSCL;
		return;
	}
	//extended points
	bool ext[6];
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
	//check 4 cases
	if (check_a(nbs, ext))
	{
		result[index] = 0.0;
		return;
	}
	if (check_b(nbs, ext))
	{
		result[index] = 0.0;
		return;
	}
	if (check_c(nbs, ext))
	{
		result[index] = 0.0;
		return;
	}
	//extened planes
	bool pxt[18];
	kc = (int4)(coord.x+2, coord.y+1, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[0] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+2, coord.y+2, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[1] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+1, coord.y+2, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[2] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+1, coord.y-2, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[3] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+2, coord.y-2, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[4] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+2, coord.y-1, coord.z, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[5] = dvalue>=TH?true:false;
	kc = (int4)(coord.x-2, coord.y, coord.z+1, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[6] = dvalue>=TH?true:false;
	kc = (int4)(coord.x-2, coord.y, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[7] = dvalue>=TH?true:false;
	kc = (int4)(coord.x-1, coord.y, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[8] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+1, coord.y, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[9] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+2, coord.y, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[10] = dvalue>=TH?true:false;
	kc = (int4)(coord.x+2, coord.y, coord.z+1, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[11] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-1, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[12] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-2, coord.z+2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[13] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-2, coord.z+1, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[14] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-2, coord.z-1, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[15] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-2, coord.z-2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[16] = dvalue>=TH?true:false;
	kc = (int4)(coord.x, coord.y-1, coord.z-2, 1);
	dvalue = read_imagef(data, samp, kc).x;
	pxt[17] = dvalue>=TH?true:false;
	if (check_d(nbs, ext, pxt))
	{
		result[index] = 0.0;
		return;
	}
	result[index] = value*VSCL;
}