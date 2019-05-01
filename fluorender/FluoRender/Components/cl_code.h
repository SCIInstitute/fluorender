const char* str_cl_slice_brainbow = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float2 vol_grad_func(image2d_t image, int2 pos)\n" \
"{\n" \
"	float2 grad1;\n" \
"	float2 grad2;\n" \
"	grad1.x = read_imagef(image, samp, pos+(int2)(1, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(-1, 0)).x;\n" \
"	grad1.y = read_imagef(image, samp, pos+(int2)(0, 1)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(0, -1)).x;\n" \
"	grad2.x = read_imagef(image, samp, pos+(int2)(1, 1)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(-1, -1)).x;\n" \
"	grad2.y = read_imagef(image, samp, pos+(int2)(1, -1)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(-1, 1)).x;\n" \
"	//rotate\n" \
"	float2 grad2r;\n" \
"	grad2r.x = dot(grad2, (float2)(-0.707, 0.707));\n" \
"	grad2r.y = dot(grad2, (float2)(-0.707, -0.707));\n" \
"	return 0.586f*grad1 + 0.414f*grad2r;\n" \
"}\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image2d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	float vv_f,\n" \
"	float av_f)\n" \
"{\n" \
"	atomic_inc(rcnt);\n" \
"	int2 coord = (int2)(get_global_id(0),\n" \
"		get_global_id(1));\n" \
"	unsigned int index = nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	float value = read_imagef(data, samp, coord).x;\n" \
"	float grad = length(vol_grad_func(data, coord));\n" \
"	//measures\n" \
"	int2 nb_coord;\n" \
"	float nb_value;\n" \
"	float avg_value = 0.0f;\n" \
"	float2 nb_grad;\n" \
"	float2 avg_grad;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	{\n" \
"		nb_coord = (int2)(coord.x+i, coord.y+j);\n" \
"		nb_value = read_imagef(data, samp, nb_coord).x;\n" \
"		avg_value += nb_value;\n" \
"		nb_grad = vol_grad_func(data, nb_coord);\n" \
"		avg_grad += nb_grad;\n" \
"	}\n" \
"	avg_value /= 9.0f;\n" \
"	avg_grad = normalize(avg_grad);\n" \
"	float value_var = 0.0f;\n" \
"	float angle_var = 0.0f;\n" \
"	for (int i=-2; i<3; ++i)\n" \
"	for (int j=-2; j<3; ++j)\n" \
"	{\n" \
"		nb_coord = (int2)(coord.x+i, coord.y+j);\n" \
"		nb_value = read_imagef(data, samp, nb_coord).x;\n" \
"		nb_grad = vol_grad_func(data, nb_coord);\n" \
"		value_var += fabs(nb_value - avg_value);\n" \
"		angle_var += length(nb_grad)*(1.0f-dot(avg_grad,\n" \
"			normalize(nb_grad))/2.0f);\n" \
"	}\n" \
"	value_var /= 25.0f;\n" \
"	angle_var /= 25.0f;\n" \
"	\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f))*\n" \
"		(vv_f>0.0f?(value_var>sqrt(vv_f)*2.12f?0.0f:exp(-value_var*value_var/vv_f)):1.0f)*\n" \
"		(vv_f>0.0f?(avg_value>sqrt(vv_f)*2.12f?0.0f:exp(-avg_value*avg_value/vv_f)):1.0f)*\n" \
"		(av_f>0.0f?(angle_var>sqrt(av_f)*2.12f?0.0f:exp(-angle_var*angle_var/av_f)):1.0f);\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	unsigned int label_value = label[index];\n" \
"	int2 max_nb_coord = coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	{\n" \
"		nb_coord = (int2)(coord.x+i, coord.y+j);\n" \
"		nb_index = nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_value)\n" \
"		{\n" \
"			label_value = m;\n" \
"			max_nb_coord = nb_coord;\n" \
"		}\n" \
"	}\n" \
"	if (grad_f > 0.0f)\n" \
"	{\n" \
"		float xc = value;\n" \
"		float xn = read_imagef(data, samp, max_nb_coord).x + grad_f;\n" \
"		if (xn < xc || xn - xc > 2.0f*grad_f)\n" \
"			return;\n" \
"	}\n" \
"\n" \
"	label[index] = label_value;\n" \
"}\n";

const char* str_cl_cleanup_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int lenx,\n" \
"	unsigned int lenz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int value_l = label[index];\n" \
"	if (value_l == 0)\n" \
"		return;\n" \
"	unsigned int res = nx*ny*nz - value_l;\n" \
"	unsigned int x = 0;\n" \
"	unsigned int y = 0;\n" \
"	unsigned int z = 0;\n" \
"	unsigned int ii;\n" \
"	for (ii=0; ii<lenx; ++ii)\n" \
"	{\n" \
"		x |= (1<<(2*ii) & res)>>(ii);\n" \
"		y |= (1<<(2*ii+1) & res)>>(ii+1);\n" \
"	}\n" \
"	z = res<<(32-lenx*2-lenz)>>(32-lenz);\n" \
"	x = reverse_bit(x, lenx);\n" \
"	y = reverse_bit(y, lenx);\n" \
"	z = reverse_bit(z, lenz);\n" \
"	index = nx*ny*z + nx*y + x;\n" \
"	atomic_inc(&(mask[index]));\n" \
"}\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int lenx,\n" \
"	unsigned int lenz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int value_l = label[index];\n" \
"	if (value_l == 0)\n" \
"		return;\n" \
"	unsigned int res = nx*ny*nz - value_l;\n" \
"	unsigned int x = 0;\n" \
"	unsigned int y = 0;\n" \
"	unsigned int z = 0;\n" \
"	unsigned int ii;\n" \
"	for (ii=0; ii<lenx; ++ii)\n" \
"	{\n" \
"		x |= (1<<(2*ii) & res)>>(ii);\n" \
"		y |= (1<<(2*ii+1) & res)>>(ii+1);\n" \
"	}\n" \
"	z = res<<(32-lenx*2-lenz)>>(32-lenz);\n" \
"	x = reverse_bit(x, lenx);\n" \
"	y = reverse_bit(y, lenx);\n" \
"	z = reverse_bit(z, lenz);\n" \
"	unsigned int index2 = nx*ny*z + nx*y + x;\n" \
"	if (index != index2)\n" \
"		mask[index] = mask[index2];\n" \
"}\n" \
"__kernel void kernel_2(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int thresh)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	//break if large enough\n" \
"	if (label[index]==0 ||\n" \
"		mask[index] > thresh)\n" \
"		return;\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int min_dist = 10;\n" \
"	unsigned int dist;\n" \
"	unsigned int max_nb_index;\n" \
"	float nb_value;\n" \
"	for (int ni=-1; ni<2; ++ni)\n" \
"	for (int nj=-1; nj<2; ++nj)\n" \
"	for (int nk=-1; nk<2; ++nk)\n" \
"	{\n" \
"		if ((k==0 && nk==-1) ||\n" \
"			(k==nz-1 && nk==1))\n" \
"			continue;\n" \
"		nb_index = nx*ny*(k+nk) + nx*(j+nj) + i+ni;\n" \
"		dist = abs(nk) + abs(nj) + abs(ni);\n" \
"		if (mask[nb_index]>thresh &&\n" \
"			dist < min_dist)\n" \
"		{\n" \
"			nb_value = read_imagef(data, samp, (int4)(i+ni, j+nj, k+nk, 1)).x;\n" \
"			if (nb_value < value)\n" \
"				continue;\n" \
"			min_dist = dist;\n" \
"			max_nb_index = nb_index;\n" \
"		}\n" \
"	}\n" \
"	if (min_dist < 10)\n" \
"		label[index] = label[max_nb_index];\n" \
"}\n" \
;

const char* str_cl_brainbow_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float3 vol_grad_func(image3d_t image, int4 pos)\n" \
"{\n" \
"	float3 grad;\n" \
"	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;\n" \
"	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;\n" \
"	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;\n" \
"	return grad;\n" \
"}\n" \
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
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	float density,\n" \
"	int dsize)\n" \
"{\n" \
"	atomic_inc(rcnt);\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	//break if low density\n" \
"	if (density > 0.0f && dsize > 0 &&\n" \
"		get_2d_density(data, (int4)(coord, 1), dsize) < density)\n" \
"		return;\n" \
"	float value = read_imagef(data, samp, (int4)(coord, 1)).x;\n" \
"	float grad = length(vol_grad_func(data, (int4)(coord, 1)));\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	unsigned int label_value = label[index];\n" \
"	int3 nb_coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	for (int k=-1; k<2; ++k)\n" \
"	{\n" \
"		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_value)\n" \
"			label_value = m;\n" \
"	}\n" \
"	label[index] = label_value;\n" \
"}\n" \
;

const char* str_cl_brainbow_3d_sized = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float3 vol_grad_func(image3d_t image, int4 pos)\n" \
"{\n" \
"	float3 grad;\n" \
"	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;\n" \
"	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;\n" \
"	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;\n" \
"	return grad;\n" \
"}\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
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
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int lenx,\n" \
"	unsigned int lenz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int value_l = label[index];\n" \
"	if (value_l == 0)\n" \
"		return;\n" \
"	unsigned int res = nx*ny*nz - value_l;\n" \
"	unsigned int x = 0;\n" \
"	unsigned int y = 0;\n" \
"	unsigned int z = 0;\n" \
"	unsigned int ii;\n" \
"	for (ii=0; ii<lenx; ++ii)\n" \
"	{\n" \
"		x |= (1<<(2*ii) & res)>>(ii);\n" \
"		y |= (1<<(2*ii+1) & res)>>(ii+1);\n" \
"	}\n" \
"	z = res<<(32-lenx*2-lenz)>>(32-lenz);\n" \
"	x = reverse_bit(x, lenx);\n" \
"	y = reverse_bit(y, lenx);\n" \
"	z = reverse_bit(z, lenz);\n" \
"	index = nx*ny*z + nx*y + x;\n" \
"	atomic_inc(&(mask[index]));\n" \
"}\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int lenx,\n" \
"	unsigned int lenz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	unsigned int value_l = label[index];\n" \
"	if (value_l == 0)\n" \
"		return;\n" \
"	unsigned int res = nx*ny*nz - value_l;\n" \
"	unsigned int x = 0;\n" \
"	unsigned int y = 0;\n" \
"	unsigned int z = 0;\n" \
"	unsigned int ii;\n" \
"	for (ii=0; ii<lenx; ++ii)\n" \
"	{\n" \
"		x |= (1<<(2*ii) & res)>>(ii);\n" \
"		y |= (1<<(2*ii+1) & res)>>(ii+1);\n" \
"	}\n" \
"	z = res<<(32-lenx*2-lenz)>>(32-lenz);\n" \
"	x = reverse_bit(x, lenx);\n" \
"	y = reverse_bit(y, lenx);\n" \
"	z = reverse_bit(z, lenz);\n" \
"	unsigned int index2 = nx*ny*z + nx*y + x;\n" \
"	if (index != index2)\n" \
"		mask[index] = mask[index2];\n" \
"}\n" \
"__kernel void kernel_2(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	unsigned int thresh,\n" \
"	float density,\n" \
"	int dsize)\n" \
"{\n" \
"	atomic_inc(rcnt);\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	//break if large enough\n" \
"	if (mask[index] > thresh)\n" \
"		return;\n" \
"	//break if low density\n" \
"	if (density > 0.0f && dsize > 0 &&\n" \
"		get_2d_density(data, (int4)(coord, 1), dsize) < density)\n" \
"		return;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	float value = read_imagef(data, samp, (int4)(coord, 1)).x;\n" \
"	float grad = length(vol_grad_func(data, (int4)(coord, 1)));\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	unsigned int label_value = label[index];\n" \
"	int3 nb_coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	for (int k=-1; k<2; ++k)\n" \
"	{\n" \
"		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_value)\n" \
"			label_value = m;\n" \
"	}\n" \
"	label[index] = label_value;\n" \
"}\n" \
;

const char* str_cl_clear_borders_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	unsigned int x, y, z;\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	if (i == 0 || i == nx-1 ||\n" \
"		j == 0 || j == ny-1 ||\n" \
"		k == 0 || k == nz-1)\n" \
"		label[index] = 0;\n" \
"}\n";

const char* str_cl_clear_borders_2d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny)\n" \
"{\n" \
"	unsigned int x, y;\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int index = nx*j + i;\n" \
"	if (i == 0 || i == nx-1 ||\n" \
"		j == 0 || j == ny-1)\n" \
"		label[index] = 0;\n" \
"}\n";

const char* str_cl_fill_borders_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float tol)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	if (i == 0)\n" \
"	{\n" \
"		float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"		float nb_value = read_imagef(data, samp, (int4)(i+1, j, k, 1)).x;\n" \
"		if (fabs(value - nb_value) < tol)\n" \
"		{\n" \
"			unsigned int index = nx*ny*k + nx*j + i;\n" \
"			unsigned int nb_index = index + 1;\n" \
"			label[index] = label[nb_index];\n" \
"		}\n" \
"	}\n" \
"	if (j == 0)\n" \
"	{\n" \
"		float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"		float nb_value = read_imagef(data, samp, (int4)(i, j+1, k, 1)).x;\n" \
"		if (fabs(value - nb_value) < tol)\n" \
"		{\n" \
"			unsigned int index = nx*ny*k + nx*j + i;\n" \
"			unsigned int nb_index = index + nx;\n" \
"			label[index] = label[nb_index];\n" \
"		}\n" \
"	}\n" \
"	if (k == 0)\n" \
"	{\n" \
"		float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"		float nb_value = read_imagef(data, samp, (int4)(i, j, k+1, 1)).x;\n" \
"		if (fabs(value - nb_value) < tol)\n" \
"		{\n" \
"			unsigned int index = nx*ny*k + nx*j + i;\n" \
"			unsigned int nb_index = index + nx*ny;\n" \
"			label[index] = label[nb_index];\n" \
"		}\n" \
"	}\n" \
"}\n";

const char* str_cl_fill_borders_2d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image2d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	float tol)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	if (i == 0)\n" \
"	{\n" \
"		float value = read_imagef(data, samp, (int2)(i, j)).x;\n" \
"		float nb_value = read_imagef(data, samp, (int2)(i+1, j)).x;\n" \
"		if (fabs(value - nb_value) < tol)\n" \
"		{\n" \
"			unsigned int index = nx*j + i;\n" \
"			unsigned int nb_index = index + 1;\n" \
"			label[index] = label[nb_index];\n" \
"		}\n" \
"	}\n" \
"	if (j == 0)\n" \
"	{\n" \
"		float value = read_imagef(data, samp, (int2)(i, j)).x;\n" \
"		float nb_value = read_imagef(data, samp, (int2)(i, j+1)).x;\n" \
"		if (fabs(value - nb_value) < tol)\n" \
"		{\n" \
"			unsigned int index = nx*j + i;\n" \
"			unsigned int nb_index = index + nx;\n" \
"			label[index] = label[nb_index];\n" \
"		}\n" \
"	}\n" \
"}\n";

const char* str_cl_shuffle_id_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int lenx,\n" \
"	unsigned int lenz)\n" \
"{\n" \
"	unsigned int res;\n" \
"	unsigned int x, y, z, ii;\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		label[index] = 0;\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		label[index] = 0;\n" \
"	else\n" \
"	{\n" \
"		x = reverse_bit(i, lenx);\n" \
"		y = reverse_bit(j, lenx);\n" \
"		z = reverse_bit(k, lenz);\n" \
"		res = 0;\n" \
"		for (ii=0; ii<lenx; ++ii)\n" \
"		{\n" \
"			res |= (1<<ii & x)<<(ii);\n" \
"			res |= (1<<ii & y)<<(ii+1);\n" \
"		}\n" \
"		res |= z<<lenx*2;\n" \
"		label[index] = nx*ny*nz - res;\n" \
"	}\n" \
"}\n";

const char* str_cl_order_id_2d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		label[index] = 0;\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		label[index] = 0;\n" \
"	else\n" \
"		label[index] = index + 1;\n" \
"}\n";

const char* str_cl_shuffle_id_2d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int len)\n" \
"{\n" \
"	unsigned int x, y, res, ii;\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float value = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	if (value < 0.001)\n" \
"		label[index] = 0;\n" \
"	else if (i<1 || i>nx-2 ||\n" \
"			j<1 || j>ny-2)\n" \
"		label[index] = 0;\n" \
"	else\n" \
"	{\n" \
"		x = reverse_bit(i, len);\n" \
"		y = reverse_bit(j, len);\n" \
"		res = 0;\n" \
"		res |= k<<(2*len);\n" \
"		for (ii=0; ii<len; ++ii)\n" \
"		{\n" \
"			res |= (1<<ii & x)<<(ii);\n" \
"			res |= (1<<ii & y)<<(ii+1);\n" \
"		}\n" \
"		label[index] = nx*ny - res;\n" \
"	}\n" \
"}\n";

const char* str_cl_grow_size = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float2 vol_grad_func(image2d_t image, int2 pos)\n" \
"{\n" \
"	float2 grad1;\n" \
"	float2 grad2;\n" \
"	grad1.x = read_imagef(image, samp, pos+(int2)(1, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(-1, 0)).x;\n" \
"	grad1.y = read_imagef(image, samp, pos+(int2)(0, 1)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(0, -1)).x;\n" \
"	grad2.x = read_imagef(image, samp, pos+(int2)(1, 1)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(-1, -1)).x;\n" \
"	grad2.y = read_imagef(image, samp, pos+(int2)(1, -1)).x-\n" \
"		read_imagef(image, samp, pos+(int2)(-1, 1)).x;\n" \
"	//rotate\n" \
"	float2 grad2r;\n" \
"	grad2r.x = dot(grad2, (float2)(-0.707, 0.707));\n" \
"	grad2r.y = dot(grad2, (float2)(-0.707, -0.707));\n" \
"	return 0.586f*grad1 + 0.414f*grad2r;\n" \
"}\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
"__kernel void kernel_0(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int len)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int index = nx*j + i;\n" \
"	unsigned int value_l = label[index];\n" \
"	if (value_l == 0)\n" \
"		return;\n" \
"	unsigned int res = nx*ny - value_l;\n" \
"	unsigned int x = 0;\n" \
"	unsigned int y = 0;\n" \
"	unsigned int ii;\n" \
"	for (ii=0; ii<len; ++ii)\n" \
"	{\n" \
"		x |= (1<<(2*ii) & res)>>(ii);\n" \
"		y |= (1<<(2*ii+1) & res)>>(ii+1);\n" \
"	}\n" \
"	x = reverse_bit(x, len);\n" \
"	y = reverse_bit(y, len);\n" \
"	index = nx*y + x;\n" \
"	atomic_inc(&(mask[index]));\n" \
"}\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int len)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int index = nx*j + i;\n" \
"	unsigned int value_l = label[index];\n" \
"	if (value_l == 0)\n" \
"		return;\n" \
"	unsigned int res = nx*ny - value_l;\n" \
"	unsigned int x = 0;\n" \
"	unsigned int y = 0;\n" \
"	unsigned int ii;\n" \
"	for (ii=0; ii<len; ++ii)\n" \
"	{\n" \
"		x |= (1<<(2*ii) & res)>>(ii);\n" \
"		y |= (1<<(2*ii+1) & res)>>(ii+1);\n" \
"	}\n" \
"	x = reverse_bit(x, len);\n" \
"	y = reverse_bit(y, len);\n" \
"	unsigned int index2 = nx*y + x;\n" \
"	if (index != index2)\n" \
"		mask[index] = mask[index2];\n" \
"}\n" \
"__kernel void kernel_2(\n" \
"	__read_only image2d_t data,\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	float vv_f,\n" \
"	float av_f,\n" \
"	unsigned int thresh)\n" \
"{\n" \
"	atomic_inc(rcnt);\n" \
"	int2 coord = (int2)(get_global_id(0),\n" \
"		get_global_id(1));\n" \
"	unsigned int index = nx*coord.y + coord.x;\n" \
"	//break if large enough\n" \
"	if (mask[index] > thresh)\n" \
"		return;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	float value = read_imagef(data, samp, coord).x;\n" \
"	float grad = length(vol_grad_func(data, coord));\n" \
"	//measures\n" \
"	int2 nb_coord;\n" \
"	float nb_value;\n" \
"	float avg_value = 0.0f;\n" \
"	float2 nb_grad;\n" \
"	float2 avg_grad;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	{\n" \
"		nb_coord = (int2)(coord.x+i, coord.y+j);\n" \
"		nb_value = read_imagef(data, samp, nb_coord).x;\n" \
"		avg_value += nb_value;\n" \
"		nb_grad = vol_grad_func(data, nb_coord);\n" \
"		avg_grad += nb_grad;\n" \
"	}\n" \
"	avg_value /= 9.0f;\n" \
"	avg_grad = normalize(avg_grad);\n" \
"	float value_var = 0.0f;\n" \
"	float angle_var = 0.0f;\n" \
"	for (int i=-2; i<3; ++i)\n" \
"	for (int j=-2; j<3; ++j)\n" \
"	{\n" \
"		nb_coord = (int2)(coord.x+i, coord.y+j);\n" \
"		nb_value = read_imagef(data, samp, nb_coord).x;\n" \
"		nb_grad = vol_grad_func(data, nb_coord);\n" \
"		value_var += fabs(nb_value - avg_value);\n" \
"		angle_var += length(nb_grad)*(1.0f-dot(avg_grad,\n" \
"			normalize(nb_grad))/2.0f);\n" \
"	}\n" \
"	value_var /= 25.0f;\n" \
"	angle_var /= 25.0f;\n" \
"	\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f))*\n" \
"		(vv_f>0.0f?(value_var>sqrt(vv_f)*2.12f?0.0f:exp(-value_var*value_var/vv_f)):1.0f)*\n" \
"		(vv_f>0.0f?(avg_value>sqrt(vv_f)*2.12f?0.0f:exp(-avg_value*avg_value/vv_f)):1.0f)*\n" \
"		(av_f>0.0f?(angle_var>sqrt(av_f)*2.12f?0.0f:exp(-angle_var*angle_var/av_f)):1.0f);\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	unsigned int label_value = label[index];\n" \
"	int2 max_nb_coord = coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	{\n" \
"		nb_coord = (int2)(coord.x+i, coord.y+j);\n" \
"		nb_index = nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_value)\n" \
"		{\n" \
"			label_value = m;\n" \
"			max_nb_coord = nb_coord;\n" \
"		}\n" \
"	}\n" \
"	if (grad_f > 0.0f)\n" \
"	{\n" \
"		float xc = value;\n" \
"		float xn = read_imagef(data, samp, max_nb_coord).x + grad_f;\n" \
"		if (xn < xc || xn - xc > 2.0f*grad_f)\n" \
"			return;\n" \
"	}\n" \
"\n" \
"	label[index] = label_value;\n" \
"}\n";

const char*str_cl_clean_up = \
"__kernel void kernel_0(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int thresh)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int index = nx*j + i;\n" \
"	//break is large enough\n" \
"	if (label[index]==0 ||\n" \
"		mask[index] > thresh)\n" \
"		return;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int max_size = 0;\n" \
"	unsigned int max_nb_index;\n" \
"	for (int ni=-1; ni<2; ++ni)\n" \
"	for (int nj=-1; nj<2; ++nj)\n" \
"	{\n" \
"		nb_index = nx*(j+nj) + i+ni;\n" \
"		if (mask[nb_index]>thresh &&\n" \
"			mask[nb_index]>max_size)\n" \
"		{\n" \
"			max_size = mask[nb_index];\n" \
"			max_nb_index = nb_index;\n" \
"		}\n" \
"	}\n" \
"	if (max_size > 0)\n" \
"		label[index] = label[max_nb_index];\n" \
"}\n";

const char* str_cl_match_slices = \
"__kernel void kernel_0(\n" \
"	__global unsigned int* mask,\n" \
"	__global unsigned int* label1,\n" \
"	__global unsigned int* label2,\n" \
"	__global bool* flag,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int index = nx*j + i;\n" \
"	if (flag[index])\n" \
"		return;\n" \
"	unsigned int value_l1 = label1[index];\n" \
"	unsigned int value_l2 = label2[index];\n" \
"	if (!value_l1 || !value_l2)\n" \
"		return;\n" \
"	unsigned int svl1, svl2, sidx;\n" \
"	unsigned int size = 0;\n" \
"	for (unsigned int ii=0; ii<nx; ++ii)\n" \
"	for (unsigned int jj=0; jj<ny; ++jj)\n" \
"	{\n" \
"		sidx = nx*jj + ii;\n" \
"		svl1 = label1[sidx];\n" \
"		svl2 = label2[sidx];\n" \
"		if (svl1!=value_l1 || svl2!=value_l2)\n" \
"			continue;\n" \
"		if (flag[sidx])\n" \
"			return;\n" \
"		flag[sidx] = true;\n" \
"		size++;\n" \
"	}\n" \
"	for (unsigned int ii=0; ii<nx; ++ii)\n" \
"	for (unsigned int jj=0; jj<ny; ++jj)\n" \
"	{\n" \
"		sidx = nx*jj + ii;\n" \
"		svl1 = label1[sidx];\n" \
"		svl2 = label2[sidx];\n" \
"		if (svl1!=value_l1 || svl2!=value_l2)\n" \
"			continue;\n" \
"		mask[sidx] = size;\n" \
"	}\n" \
"}\n" \
"\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned int* mask1,\n" \
"	__global unsigned int* mask2,\n" \
"	__global unsigned int* mask_and,\n" \
"	__global unsigned int* label1,\n" \
"	__global unsigned int* label2,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int thresh)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int index = nx*j +i;\n" \
"	unsigned int value_l1 = label1[index];\n" \
"	unsigned int value_l2 = label2[index];\n" \
"	if (!value_l1 || !value_l2 || value_l1==value_l2)\n" \
"		return;\n" \
"	unsigned int size1 = mask1[index];\n" \
"	unsigned int size2 = mask2[index];\n" \
"	if (size1<=thresh || size2<=thresh)\n" \
"		return;\n" \
"	unsigned int size_and = mask_and[index];\n" \
"	if ((float)size_and/(float)size1 +\n" \
"		(float)size_and/(float)size2 <= 1.0)\n" \
"		return;\n" \
"	unsigned int sidx;\n" \
"	for (unsigned int ii=0; ii<nx; ++ii)\n" \
"	for (unsigned int jj=0; jj<ny; ++jj)\n" \
"	{\n" \
"		sidx = nx*jj + ii;\n" \
"		if (label2[sidx] == value_l2)\n" \
"			label2[sidx] = value_l1;\n" \
"	}\n" \
"}\n";

const char* str_cl_dist_field_3d = \
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
"	float th)\n" \
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

const char* str_cl_dist_field_2d = \
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
"	float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
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
"	__read_only image3d_t data,\n" \
"	__global unsigned char* df,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float maxd)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int nxy = nx*ny;\n" \
"	unsigned int index = nxy*ijk.z + nx*ijk.y + ijk.x;\n" \
"	float dval = read_imagef(data, samp, (int4)(ijk, 1)).x;\n" \
"	df[index] = (unsigned char)(dval * df[index] * 255.0 / maxd);\n" \
"}\n" \
;

const char* str_cl_density_field_3d = \
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
"\n" \
"//generate density field\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* df,\n" \
"	unsigned int dnxy,\n" \
"	unsigned int dnx,\n" \
"	int dsize)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = dnxy*ijk.z + dnx*ijk.y + ijk.x;\n" \
"	float density = get_2d_density(data, (int4)(ijk, 1), dsize);\n" \
"	df[index] = (unsigned char)(density * 255.0);\n" \
"}\n" \
"\n" \
"//compute statistics on density field\n" \
"__kernel void kernel_1(\n" \
"	__global unsigned char* df,\n" \
"	__global unsigned char* gavg,\n" \
"	__global unsigned char* gvar,\n" \
"	unsigned int gsx,\n" \
"	unsigned int gsy,\n" \
"	unsigned int gsz,\n" \
"	unsigned int ngxy,\n" \
"	unsigned int ngx,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx)\n" \
"{\n" \
"	int3 gid = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 lb = (int3)(gid.x*gsx, gid.y*gsy, gid.z*gsz);\n" \
"	int3 ub = (int3)(lb.x + gsx, lb.y + gsy, lb.z + gsz);\n" \
"	int3 ijk = (int3)(0);\n" \
"	float sum = 0.0;\n" \
"	unsigned int index1;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		index1 = dnxy*ijk.z + dnx*ijk.y + ijk.x;\n" \
"		sum += df[index1];\n" \
"	}\n" \
"	unsigned int index2 = ngxy * gid.z + ngx * gid.y + gid.x;\n" \
"	float avg = sum / (gsx*gsy*gsz);\n" \
"	gavg[index2] = avg;\n" \
"	sum = 0.0;\n" \
"	for (ijk.x = lb.x; ijk.x < ub.x; ++ijk.x)\n" \
"	for (ijk.y = lb.y; ijk.y < ub.y; ++ijk.y)\n" \
"	for (ijk.z = lb.z; ijk.z < ub.z; ++ijk.z)\n" \
"	{\n" \
"		index1 = dnxy*ijk.z + dnx*ijk.y + ijk.x;\n" \
"		sum += (avg - df[index1])*(avg - df[index1]);\n" \
"	}\n" \
"	float var = sqrt(sum / (gsx*gsy*gsz));\n" \
"	gvar[index2] = var;\n" \
"}\n" \
"\n" \
"//interpolate statistics on density field\n" \
"__kernel void kernel_2(\n" \
"	__global unsigned char* idf,\n" \
"	__global unsigned char* gd,\n" \
"	unsigned int gsx,\n" \
"	unsigned int gsy,\n" \
"	unsigned int gsz,\n" \
"	unsigned int ngx,\n" \
"	unsigned int ngy,\n" \
"	unsigned int ngz,\n" \
"	unsigned int dnxy, \n" \
"	unsigned int dnx)\n" \
"{\n" \
"	int3 ijk = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	int3 gid;\n" \
"	int3 gijk;\n" \
"	gijk = ijk % (int3)(gsx, gsy, gsz);\n" \
"	gid = ijk / (int3)(gsx, gsy, gsz);\n" \
"	gid += isless((float3)(gijk.x, gijk.y, gijk.z), (float3)(gsx/2.0, gsy/2.0, gsz/2.0));\n" \
"	int3 gcrd = clamp(gid + (int3)(0, 0, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c000 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(1, 0, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c100 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(0, 1, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c010 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(1, 1, 0), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c110 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(0, 0, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c001 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(1, 0, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c101 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(0, 1, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c011 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	gcrd = clamp(gid + (int3)(1, 1, 1), (int3)(0), (int3)(ngx-1, ngy-1, ngz-1));\n" \
"	uchar c111 = gd[ngx*ngy*gcrd.z + ngx*gcrd.y + gcrd.x];\n" \
"	float3 d = ((float3)(gijk.x, gijk.y, gijk.z) - (float3)(gsx/2.0, gsy/2.0, gsz/2.0)) / (float3)(gsx, gsy, gsz);\n" \
"	int3 delta = isless(d, (float3)(0.0));\n" \
"	d -= (float3)(delta.x, delta.y, delta.z);\n" \
"	float c00 = (float)(c000)*(1.0-d.x) + (float)(c100)*d.x;\n" \
"	float c01 = (float)(c001)*(1.0-d.x) + (float)(c101)*d.x;\n" \
"	float c10 = (float)(c010)*(1.0-d.x) + (float)(c110)*d.x;\n" \
"	float c11 = (float)(c011)*(1.0-d.x) + (float)(c111)*d.x;\n" \
"	float c0 = c00*(1.0-d.y) + c10*d.y;\n" \
"	float c1 = c01*(1.0-d.y) + c11*d.y;\n" \
"	idf[dnxy* ijk.z + dnx*ijk.y + ijk.x] = c0*(1.0-d.z) + c1*d.z;\n" \
"}\n" \
;

const char* str_cl_density_grow_3d = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_NEAREST;\n" \
"\n" \
"float3 vol_grad_func(image3d_t image, int4 pos)\n" \
"{\n" \
"	float3 grad;\n" \
"	grad.x = read_imagef(image, samp, pos+(int4)(1, 0, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(-1, 0, 0, 0)).x;\n" \
"	grad.y = read_imagef(image, samp, pos+(int4)(0, 1, 0, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, -1, 0, 0)).x;\n" \
"	grad.z = read_imagef(image, samp, pos+(int4)(0, 0, 1, 0)).x-\n" \
"		read_imagef(image, samp, pos+(int4)(0, 0, -1, 0)).x;\n" \
"	return grad;\n" \
"}\n" \
"\n" \
"unsigned int __attribute((always_inline)) reverse_bit(unsigned int val, unsigned int len)\n" \
"{\n" \
"	unsigned int res = val;\n" \
"	int s = len - 1;\n" \
"	for (val >>= 1; val; val >>= 1)\n" \
"	{\n" \
"		res <<= 1;\n" \
"		res |= val & 1;\n" \
"		s--;\n" \
"	}\n" \
"	res <<= s;\n" \
"	res <<= 32-len;\n" \
"	res >>= 32-len;\n" \
"	return res;\n" \
"}\n" \
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
"	__global unsigned int* label,\n" \
"	__global unsigned char* df,\n" \
"	__global unsigned char* avg,\n" \
"	__global unsigned char* var,\n" \
"	__global unsigned int* rcnt,\n" \
"	unsigned int seed,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	unsigned int dnxy,\n" \
"	unsigned int dnx,\n" \
"	float value_t,\n" \
"	float value_f,\n" \
"	float grad_f,\n" \
"	float density)\n" \
"{\n" \
"	atomic_inc(rcnt);\n" \
"	int3 coord = (int3)(get_global_id(0),\n" \
"		get_global_id(1), get_global_id(2));\n" \
"	unsigned int index = nx*ny*coord.z + nx*coord.y + coord.x;\n" \
"	unsigned int label_v = label[index];\n" \
"	if (label_v == 0)\n" \
"		return;\n" \
"	//break if low density\n" \
"	if (density > 0.0f)\n" \
"	{\n" \
"		unsigned int index2 = dnxy*coord.z + dnx*coord.y + coord.x;\n" \
"		unsigned char vdf = df[index2];\n" \
"		unsigned char vavg = avg[index2];\n" \
"		unsigned char vvar = var[index2];\n" \
"		if (vdf < vavg - (1.0-density)*vvar)\n" \
"			return;\n" \
"	}\n" \
"	float value = read_imagef(data, samp, (int4)(coord, 1)).x;\n" \
"	float grad = length(vol_grad_func(data, (int4)(coord, 1)));\n" \
"	//stop function\n" \
"	float stop =\n" \
"		(grad_f>0.0f?(grad>sqrt(grad_f)*2.12f?0.0f:exp(-grad*grad/grad_f)):1.0f)*\n" \
"		(value>value_t?1.0f:(value_f>0.0f?(value<value_t-sqrt(value_f)*2.12f?0.0f:exp(-(value-value_t)*(value-value_t)/value_f)):0.0f));\n" \
"	\n" \
"	//max filter\n" \
"	float random = (float)((*rcnt) % seed)/(float)(seed)+0.001f;\n" \
"	if (stop < random)\n" \
"		return;\n" \
"	unsigned int label_value = label[index];\n" \
"	int3 nb_coord;\n" \
"	unsigned int nb_index;\n" \
"	unsigned int m;\n" \
"	for (int i=-1; i<2; ++i)\n" \
"	for (int j=-1; j<2; ++j)\n" \
"	for (int k=-1; k<2; ++k)\n" \
"	{\n" \
"		nb_coord = (int3)(coord.x+i, coord.y+j, coord.z+k);\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = label[nb_index];\n" \
"		if (m > label_value)\n" \
"			label_value = m;\n" \
"	}\n" \
"	label[index] = label_value;\n" \
"}\n" \
;

