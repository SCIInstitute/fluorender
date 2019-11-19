const char* str_cl_diffusion = \
"const sampler_t samp =\n" \
"	CLK_NORMALIZED_COORDS_FALSE|\n" \
"	CLK_ADDRESS_CLAMP_TO_EDGE|\n" \
"	CLK_FILTER_LINEAR;\n" \
"\n" \
"__kernel void kernel_0(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* mask,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl,\n" \
"	float3 p,\n" \
"	unsigned char val)\n" \
"{\n" \
"	unsigned int i = (int)(p.x);\n" \
"	unsigned int j = (int)(p.y);\n" \
"	unsigned int k = (int)(p.z);\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 pt = (float3)((float)(i) / (float)(nx), (float)(j) / (float)(ny), (float)(k) / (float)(nz));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"		return;\n" \
"	atomic_xchg(mask+index, val);\n" \
"}\n" \
"__kernel void kernel_1(\n" \
"	__read_only image3d_t data,\n" \
"	__global unsigned char* mask,\n" \
"	unsigned int nx,\n" \
"	unsigned int ny,\n" \
"	unsigned int nz,\n" \
"	float4 p0,\n" \
"	float4 p1,\n" \
"	float4 p2,\n" \
"	float4 p3,\n" \
"	float4 p4,\n" \
"	float4 p5,\n" \
"	float3 scl,\n" \
"	float3 trl,\n" \
"	float4 loc2,\n" \
"	float4 loc3,\n" \
"	float4 loc7)\n" \
"{\n" \
"	unsigned int i = (unsigned int)(get_global_id(0));\n" \
"	unsigned int j = (unsigned int)(get_global_id(1));\n" \
"	unsigned int k = (unsigned int)(get_global_id(2));\n" \
"	unsigned int index = nx*ny*k + nx*j + i;\n" \
"	float3 dir = (float3)(1.0/(float)(nx), 1.0/(float)(ny), 1.0/(float)(nz));\n" \
"	float3 pt = dir * (float3)((float)(i), (float)(j), (float)(k));\n" \
"	pt = pt * scl + trl;\n" \
"	if (dot(pt, p0.xyz)+p0.w < 0.0 ||\n" \
"		dot(pt, p1.xyz)+p1.w < 0.0 ||\n" \
"		dot(pt, p2.xyz)+p2.w < 0.0 ||\n" \
"		dot(pt, p3.xyz)+p3.w < 0.0 ||\n" \
"		dot(pt, p4.xyz)+p4.w < 0.0 ||\n" \
"		dot(pt, p5.xyz)+p5.w < 0.0)\n" \
"		return;\n" \
"	//grad compute\n" \
"	float3 v;\n" \
"	v.x = read_imagef(data, samp, (int4)(i, j, k, 1)).x;\n" \
"	float3 n = (float3)(0.0);\n" \
"	n.x += read_imagef(data, samp, (int4)(i+1, j, k, 1)).x;\n" \
"	n.x -= read_imagef(data, samp, (int4)(i-1, j, k, 1)).x;\n" \
"	n.y += read_imagef(data, samp, (int4)(i, j+1, k, 1)).x;\n" \
"	n.y -= read_imagef(data, samp, (int4)(i, j-1, k, 1)).x;\n" \
"	n.z += read_imagef(data, samp, (float4)((float)(i), (float)(j), (float)(k)+min((float)(nz)/(float)(nx), 1.0), 1.0)).x;\n" \
"	n.z -= read_imagef(data, samp, (float4)((float)(i), (float)(j), (float)(k)-min((float)(nz)/(float)(nx), 1.0), 1.0)).x;\n" \
"	v.y = length(n);\n" \
"	v.y = 0.5 * (loc2.x<0.0?(1.0+v.y*loc2.x):v.y*loc2.x);\n" \
"	//VOL_TRANSFER_FUNCTION_SIN_COLOR_L\n" \
"	float c;\n" \
"	v.x = loc2.x < 0.0 ? (1.0 + v.x*loc2.x) : v.x*loc2.x;\n" \
"	if (v.x < loc2.z - loc3.w || (loc2.w<1.0 && v.x>loc2.w + loc3.w))\n" \
"		c = 0.0;\n" \
"	else\n" \
"	{\n" \
"		v.x = (v.x < loc2.z ? (loc3.w - loc2.z + v.x) / loc3.w : (loc2.w<1.0 && v.x>loc2.w ? (loc3.w - v.x + loc2.w) / loc3.w : 1.0))*v.x;\n" \
"		v.x = (loc3.y > 0.0 ? clamp(v.y / loc3.y, 0.0, 1.0 + loc3.y*10.0) : 1.0)*v.x;\n" \
"		c = pow(clamp(v.x / loc3.z, loc3.x<1.0 ? -(loc3.x - 1.0)*0.00001 : 0.0, loc3.x>1.0 ? 0.9999 : 1.0), loc3.x);\n" \
"	}\n" \
"	//SEG_BODY_DB_GROW_STOP_FUNC\n" \
"	if (c <= 0.0001)\n" \
"		return;\n" \
"	v.x = c > 1.0 ? 1.0 : c.x;\n" \
"	float stop =\n" \
"		(loc7.y >= 1.0 ? 1.0 : (v.y > sqrt(loc7.y)*2.12 ? 0.0 : exp(-v.y*v.y / loc7.y)))*\n" \
"		(v.x > loc7.w ? 1.0 : (loc7.z > 0.0 ? (v.x < loc7.w - sqrt(loc7.z)*2.12 ? 0.0 : exp(-(v.x - loc7.w)*(v.x - loc7.w) / loc7.z)) : 0.0));\n" \
"	if (stop <= 0.0001)\n" \
"		return;\n" \
"	//SEG_BODY_DB_GROW_BLEND_APPEND\n" \
"	unsigned char cc = mask[index];\n" \
"	float val = (1.0 - stop) * (float)(cc);\n" \
"	int3 nb_coord;\n" \
"	int3 max_nb;\n" \
"	unsigned int nb_index;\n" \
"	unsigned char m;\n" \
"	unsigned char mx;\n" \
"	for (int ii = -1; ii < 2; ii++)\n" \
"	for (int jj = -1; jj < 2; jj++)\n" \
"	for (int kk = -1; kk < 2; kk++)\n" \
"	{\n" \
"		nb_coord = (int3)(ii+i, jj+j, kk+k);\n" \
"		if (nb_coord.x < 0 || nb_coord.x > nx-1 ||\n" \
"			nb_coord.y < 0 || nb_coord.y > ny-1 ||\n" \
"			nb_coord.z < 0 || nb_coord.z > nz-1)\n" \
"			continue;\n" \
"		nb_index = nx*ny*nb_coord.z + nx*nb_coord.y + nb_coord.x;\n" \
"		m = mask[nb_index];\n" \
"		if (m > cc)\n" \
"		{\n" \
"			cc = m;\n" \
"			max_nb = nb_coord;\n" \
"		}\n" \
"	}\n" \
"	if (loc7.y > 0.0)\n" \
"	{\n" \
"		m = (unsigned char)((read_imagef(data, samp, (int4)(max_nb, 1)).x + loc7.y) * 255.0);\n" \
"		mx = (unsigned char)(read_imagef(data, samp, (int4)(i, j, k, 1)).x * 255.0);\n" \
"		if (m < mx || m - mx > (unsigned char)(510.0*loc7.y))\n" \
"			return;\n" \
"	}\n" \
"	cc = clamp(cc * (unsigned char)(stop * 255.0), 0, 255);\n" \
"	atomic_xchg(mask+index, cc);\n" \
;

