const char* str_cl_skeleton_2d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float get_2d_density(image3d_t image, int4 pos, int r)\n" \
"{\n" \
"	float sum = 0.0f;\n" \
"	int d = 2*r+1;\n" \
"	for (int i=-r; i<=r; ++i)\n" \
"	for (int j=-r; j<=r; ++j)\n" \
"		sum += read_imagef(image, samp, pos+(int4)(i, j, 0, 0)).x;\n" \
"	return sum / (float)(d * d);\n" \
"}\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	int dsize,\n" \
"	float th,\n" \
"	float sscale,\n" \
"	unsigned char ini)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (ijk.x == 0 || ijk.x == nx-1 ||\n" \
"		ijk.y == 0 || ijk.y == ny-1)\n" \
"	{\n" \
"		df[index] = 0;\n" \
"		return;\n" \
"	}\n" \
"	//float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
"	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);\n" \
"	dval *= sscale;\n" \
"	if (dval > th)\n" \
"		df[index] = ini;\n" \
"	else\n" \
"		df[index] = 0;\n" \
"}\n" \
"\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned char ini,\n" \
"	unsigned char nn,\n" \
"	unsigned char re)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int nxy = nx*ny;\n" \
"	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (df[index] == ini)\n" \
"	{\n" \
"		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];\n" \
"		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];\n" \
"		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];\n" \
"		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];\n" \
"		if (v1 == nn || v2 == nn ||\n" \
"			v3 == nn || v4 == nn)\n" \
"			df[index] = re;\n" \
"	}\n" \
"}\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned char ini,\n" \
"	unsigned char nn,\n" \
"	unsigned char re)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int nxy = nx*ny;\n" \
"	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (df[index] == ini)\n" \
"	{\n" \
"		short v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];\n" \
"		short v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];\n" \
"		short v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];\n" \
"		short v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];\n" \
"		short rre = (ijk.x % 13 + ijk.y % 17) % 4;\n" \
"		v1 = rre == 0 ? -1 : v1;\n" \
"		v2 = rre == 3 ? -1 : v2;\n" \
"		v3 = rre == 1 ? -1 : v3;\n" \
"		v4 = rre == 2 ? -1 : v4;\n" \
"		if (v1 == nn || v2 == nn ||\n" \
"			v3 == nn || v4 == nn)\n" \
"			df[index] = re;\n" \
"	}\n" \
"}\n" \
"//compute dist field in mask\n" \
"__kernel void kernel_3(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	int dsize,\n" \
"	float th,\n" \
"	float sscale,\n" \
"	unsigned char ini,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;\n" \
"	if (mask_value < 1e-6)\n" \
"	{\n" \
"		df[index] = 0;\n" \
"		return;\n" \
"	}\n" \
"	if (ijk.x == 0 || ijk.x == nx-1 ||\n" \
"		ijk.y == 0 || ijk.y == ny-1)\n" \
"	{\n" \
"		df[index] = 0;\n" \
"		return;\n" \
"	}\n" \
"	//float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
"	float dval = get_2d_density(data, (int4)(ijk, 1), dsize);\n" \
"	dval *= sscale;\n" \
"	if (dval > th)\n" \
"		df[index] = ini;\n" \
"	else\n" \
"		df[index] = 0;\n" \
"}\n" \
"//compute dist field in mask\n" \
"__kernel void kernel_4(\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned char ini,\n" \
"	unsigned char nn,\n" \
"	unsigned char re,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;\n" \
"	if (mask_value < 1e-6)\n" \
"		return;\n" \
"	unsigned int nxy = nx*ny;\n" \
"	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (df[index] == ini)\n" \
"	{\n" \
"		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];\n" \
"		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];\n" \
"		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];\n" \
"		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];\n" \
"		if (v1 == nn || v2 == nn ||\n" \
"			v3 == nn || v4 == nn)\n" \
"			df[index] = re;\n" \
"	}\n" \
"}\n" \
"//compute dist field in mask\n" \
"__kernel void kernel_5(\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned char ini,\n" \
"	unsigned char nn,\n" \
"	unsigned char re,\n" \
"	__read_only image3d_t mask)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	float mask_value = read_imagef(mask, samp, (int4)(ijk, 1)).x;\n" \
"	if (mask_value < 1e-6)\n" \
"		return;\n" \
"	unsigned int nxy = nx*ny;\n" \
"	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (df[index] == ini)\n" \
"	{\n" \
"		short v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];\n" \
"		short v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];\n" \
"		short v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];\n" \
"		short v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];\n" \
"		short rre = (ijk.x % 13 + ijk.y % 17) % 4;\n" \
"		v1 = rre == 0 ? -1 : v1;\n" \
"		v2 = rre == 3 ? -1 : v2;\n" \
"		v3 = rre == 1 ? -1 : v3;\n" \
"		v4 = rre == 2 ? -1 : v4;\n" \
"		if (v1 == nn || v2 == nn ||\n" \
"			v3 == nn || v4 == nn)\n" \
"			df[index] = re;\n" \
"	}\n" \
"}\n" \
;

const char* str_cl_skeleton_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float th,\n" \
"	float sscale)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (ijk.x == 0 || ijk.x == nx-1 ||\n" \
"		ijk.y == 0 || ijk.y == ny-1 ||\n" \
"		ijk.z == 0 || ijk.z == nz-1)\n" \
"	{\n" \
"		df[index] = 0;\n" \
"		return;\n" \
"	}\n" \
"	float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
"	dval *= sscale;\n" \
"	if (dval > th)\n" \
"		df[index] = 1;\n" \
"	else\n" \
"		df[index] = 0;\n" \
"}\n" \
"\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned char nn,\n" \
"	unsigned char re)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int nxy = nx*ny;\n" \
"	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	if (df[index] == 1)\n" \
"	{\n" \
"		unsigned char v1 = df[nxy*ijk.z + nx*ijk.y + ijk.x - 1];\n" \
"		unsigned char v2 = df[nxy*ijk.z + nx*ijk.y + ijk.x + 1];\n" \
"		unsigned char v3 = df[nxy*ijk.z + nx*(ijk.y-1) + ijk.x];\n" \
"		unsigned char v4 = df[nxy*ijk.z + nx*(ijk.y+1) + ijk.x];\n" \
"		unsigned char v5 = df[nxy*(ijk.z-1) + nx*ijk.y + ijk.x];\n" \
"		unsigned char v6 = df[nxy*(ijk.z+1) + nx*ijk.y + ijk.x];\n" \
"		if (v1 == nn || v2 == nn ||\n" \
"			v3 == nn || v4 == nn ||\n" \
"			v5 == nn || v6 == nn)\n" \
"			df[index] = re;\n" \
"	}\n" \
"}\n" \
;

