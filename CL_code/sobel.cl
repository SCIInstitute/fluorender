#define KX 3
#define KY 3
#define KZ 3
__constant float krnx[KX*KY*KZ] =
{1, 0, -1,
 2, 0, -2,
 1, 0, -1,
 2, 0, -2,
 4, 0, -4,
 2, 0, -2,
 1, 0, -1,
 2, 0, -2,
 1, 0, -1};
__constant float krny[KX*KY*KZ] =
{1, 2, 1,
 0, 0, 0,
 -1, -2, -1,
 2, 4, 2,
 0, 0, 0,
 -2, -4, -2,
 1, 2, 1,
 0, 0, 0,
 -1, -2, -1};
__constant float krnz[KX*KY*KZ] =
{1, 2, 1,
 2, 4, 2,
 1, 2, 1,
 0, 0, 0,
 0, 0, 0,
 0, 0, 0,
 -1, -2, -1,
 -2, -4, -2,
 -1, -2, -1};
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
	float rx = 0.0;
	float ry = 0.0;
	float rz = 0.0;
	int i, j, k;
	for (i=0; i<KX; ++i)
	for (j=0; j<KY; ++j)
	for (k=0; k<KZ; ++k)
	{
		kc = (int4)(coord.x+(i-KX/2),
				coord.y+(j-KY/2),
				coord.z+(k-KZ/2), 1);
		dvalue = read_imagef(data, samp, kc);
		rx += krnx[KX*KY*k+KX*j+i] * dvalue.x;
		ry += krny[KX*KY*k+KX*j+i] * dvalue.x;
		rz += krnz[KX*KY*k+KX*j+i] * dvalue.x;
	}
	float rvalue = sqrt(rx*rx + ry*ry + rz*rz);
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = clamp(rvalue, 0.0f, 1.0f)*255.0;
}