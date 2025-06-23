#define KX 3
#define KY 3
#define KZ 3
#define DWL unsigned char
#define VSCL 255
#define EPSILON 0.01f  // Regularization term to avoid division by zero

// Example 3x3x3 Gaussian PSF (normalized)
float psf(int i, int j, int k) {
	const float g[3] = { 0.25f, 0.5f, 0.25f };
	return g[i] * g[j] * g[k];
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

	float observed = read_imagef(data, samp, coord).x;
	float blurred = 0.0f;
	for (int i = 0; i < KX; ++i)
		for (int j = 0; j < KY; ++j)
			for (int k = 0; k < KZ; ++k) {
				int4 kc = (int4)(coord.x + (i - KX / 2),
					coord.y + (j - KY / 2),
					coord.z + (k - KZ / 2), 1);
				float4 dvalue = read_imagef(data, samp, kc);
				blurred += psf(i, j, k) * dvalue.x;
			}

	float gain = blurred / (blurred * blurred + EPSILON);
	gain = clamp(gain, 0.0f, 2.0f);  // limit amplification
	float restored = observed * gain;
	restored = clamp(restored, 0.0f, 1.0f);

	unsigned int index = x * y * coord.z + x * coord.y + coord.x;
	result[index] = restored * VSCL;
}
