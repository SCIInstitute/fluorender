#define KX 11
#define KY 11
#define DWL unsigned char
#define VSCL 255
#define SIGMA 1.5f
#define PI 3.14159265f

float gauss(int i, int j)
{
	float s2 = 2.0f * SIGMA * SIGMA;
	float r2 = (float)(i * i + j * j);
	return exp(-r2 / s2) / (PI * s2);
}

const sampler_t samp =
CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP_TO_EDGE |
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
	int4 kc;
	float4 dvalue;
	float rvalue = 0.0f;
	float total_weight = 0.0f;

	for (int i = 0; i < KX; ++i)
	for (int j = 0; j < KY; ++j)
	{
		int dx = i - KX / 2;
		int dy = j - KY / 2;
		float w = gauss(dx, dy);

		kc = (int4)(coord.x + dx, coord.y + dy, coord.z, 1);
		dvalue = read_imagef(data, samp, kc);
		rvalue += w * dvalue.x;
		total_weight += w;
	}

	rvalue = rvalue / total_weight;
	unsigned int index = x*y*coord.z + x*coord.y + coord.x;
	result[index] = clamp(rvalue, 0.0f, 1.0f) * VSCL;
}
