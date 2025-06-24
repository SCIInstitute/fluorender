#define KX 3
#define KY 3
#define KZ 3
#define DWL unsigned char
#define VSCL 255
#define MAX_INT 255.0f
#define EPSILON (1.0f / VSCL * 5.0f)

// Forward PSF (Gaussian)
float psf(int i, int j, int k) {
	const float g[3] = { 0.25f, 0.5f, 0.25f };
	return g[i] * g[j] * g[k];
}

// Flipped PSF (same as PSF if symmetric)
float psf_flip(int i, int j, int k) {
	return psf(KX - 1 - i, KY - 1 - j, KZ - 1 - k);
}

const sampler_t samp =
CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP_TO_EDGE |
CLK_FILTER_NEAREST;

__kernel void kernel_main(
	read_only image3d_t data,   // original blurred image
	//read_only image3d_t estimate,   // current estimate (input)
	__global DWL* result,           // updated estimate (output)
	unsigned int x,
	unsigned int y,
	unsigned int z)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1),
		get_global_id(2), 1);

	// Step 1: Convolve estimate with PSF
	float blurred = 0.0f;
	for (int i = 0; i < KX; ++i)
		for (int j = 0; j < KY; ++j)
			for (int k = 0; k < KZ; ++k) {
				int4 kc = (int4)(coord.x + (i - KX / 2),
					coord.y + (j - KY / 2),
					coord.z + (k - KZ / 2), 1);
				float4 val = read_imagef(data, samp, kc);
				blurred += psf(i, j, k) * val.x;
			}

	// Step 2: Compute ratio = observed / blurred
	float4 obs_val = read_imagef(data, samp, coord);
	//float gain = blurred / (blurred * blurred + EPSILON);
	//float ratio = obs_val.x * gain;
	float ratio = obs_val.x / (blurred + EPSILON);

	// Step 3: Convolve ratio with flipped PSF
	float correction = 0.0f;
	for (int i = 0; i < KX; ++i)
		for (int j = 0; j < KY; ++j)
			for (int k = 0; k < KZ; ++k) {
				int4 kc = (int4)(coord.x + (i - KX / 2),
					coord.y + (j - KY / 2),
					coord.z + (k - KZ / 2), 1);
				float4 val = read_imagef(data, samp, kc);
				correction += psf_flip(i, j, k) * ratio;
			}

	// Step 4: Multiply estimate by correction
	float4 est_val = read_imagef(data, samp, coord);
	float updated = est_val.x * correction;

	// Write back updated estimate
	unsigned int index = x * y * coord.z + x * coord.y + coord.x;
	result[index] = clamp(updated, 0.0f, 1.0f) * VSCL;
}
