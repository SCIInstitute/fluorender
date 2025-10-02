//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#ifndef VOL_KERNEL_CODE_H
#define VOL_KERNEL_CODE_H

inline constexpr const char* KERNEL_TEST_CODE = R"CLKER(
__kernel void add_numbers(__global float4* data, 
	__local float* local_result, __global float* group_result) {
	float sum;
	float4 input1, input2, sum_vector;
	uint global_addr, local_addr;
	global_addr = get_global_id(0) * 2;
	input1 = data[global_addr];
	input2 = data[global_addr+1];
	sum_vector = input1 + input2;
	local_addr = get_local_id(0);
	local_result[local_addr] = sum_vector.s0 + sum_vector.s1 + 
		sum_vector.s2 + sum_vector.s3;
	barrier(CLK_LOCAL_MEM_FENCE);
	if(get_local_id(0) == 0) {
		sum = 0.0f;
		for(int i=0; i<get_local_size(0); i++) {
		sum += local_result[i];
		}
		group_result[get_group_id(0)] = sum;
	}
}
)CLKER";

inline constexpr const char* KERNEL_HIST_3D_CODE = R"CLKER(
//KERNEL_HIST_3D_CODE
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;
__kernel void hist_3d(image3d_t data,
	image3d_t mask, __global float* hist,
	__const int hist_size)
{
	int4 coord = (int4)(get_global_id(0),
		get_global_id(1), get_global_id(2), 1);
	float4 mask_value = read_imagef(mask, samp, coord);
	if (mask_value.x > 0.0)
	{
		float4 data_value = read_imagef(data, samp, coord);
		int index = (int)(data_value.x*(hist_size-1));
		hist[index] += 1.0;
	}
}
)CLKER";

#endif//VOL_KERNEL_CODE_H