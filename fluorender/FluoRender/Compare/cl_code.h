/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
const char* str_cl_chann_compare_ = \
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

