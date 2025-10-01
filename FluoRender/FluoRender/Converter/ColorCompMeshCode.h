/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#ifndef _COLOR_COMP_MESH_CODE_H_
#define _COLOR_COMP_MESH_CODE_H_

inline constexpr const char* str_cl_color_comp_mesh = R"CLKER(
__kernel void kernel_0(
	image3d_t volume,                      // intensity volume (brick)
	__global const unsigned int* label,   // label volume (flat array, brick-aligned)
	__global const float* vertex_vbo,     // float3 per vertex
	__global float* color_vbo,            // float4 per vertex
	const int3 voxel_cnt,                 // brick dimensions
	const int3 vol_org,                   // brick origin in voxel space
	const int si,                        // shuffle index
	const int num_vertices)
{
	int gid = get_global_id(0);
	if (gid >= num_vertices) return;

	// Load vertex position in world space
	float3 pos = (float3)(
		vertex_vbo[gid * 3 + 0],
		vertex_vbo[gid * 3 + 1],
		vertex_vbo[gid * 3 + 2]);

	// Convert to voxel-space index relative to brick origin
	int3 voxel_idx = convert_int3(pos) - vol_org;

	// Bounds check
	if (voxel_idx.x < 0 || voxel_idx.x >= voxel_cnt.x ||
		voxel_idx.y < 0 || voxel_idx.y >= voxel_cnt.y ||
		voxel_idx.z < 0 || voxel_idx.z >= voxel_cnt.z)
	{
		// Default color for out-of-bounds
		color_vbo[gid * 4 + 0] = 0.2f;
		color_vbo[gid * 4 + 1] = 0.4f;
		color_vbo[gid * 4 + 2] = 0.4f;
		color_vbo[gid * 4 + 3] = 0.0f;
		return;
	}

	// Convert to normalized texture coordinates within brick
	float3 tex_coord = (convert_float3(voxel_idx) + 0.5f) / convert_float3(voxel_cnt);

	// Sample intensity from volume texture
	float intensity = read_imagef(
		volume,
		CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP | CLK_FILTER_LINEAR,
		(int4)(voxel_idx.x, voxel_idx.y, voxel_idx.z, 1)).x;

	// Flat index into label array
	int flat_idx = voxel_idx.z * voxel_cnt.y * voxel_cnt.x +
				   voxel_idx.y * voxel_cnt.x +
				   voxel_idx.x;

	uint lbl = label[flat_idx];
	float4 sel = (float4)(0.2f, 0.4f, 0.4f, 1.0f);

	if (lbl > 0)
	{
		uint cv = lbl % 0xfd;
		cv = ((cv << si) & 0xff) | (cv >> (8 - si));
		float hue = (float)cv / 45.0f;
		float p2 = 1.0f - hue + floor(hue);
		float p3 = hue - floor(hue);

		if (hue < 1.0f)
			sel = (float4)(1.0f, p3, 1.0f, 1.0f);
		else if (hue < 2.0f)
			sel = (float4)(p2, 1.0f, 1.0f, 1.0f);
		else if (hue < 3.0f)
			sel = (float4)(1.0f, 1.0f, p3, 1.0f);
		else if (hue < 4.0f)
			sel = (float4)(1.0f, p2, 1.0f, 1.0f);
		else if (hue < 5.0f)
			sel = (float4)(p3, 1.0f, 1.0f, 1.0f);
		else
			sel = (float4)(1.0f, 1.0f, p2, 1.0f);
	}

	// Final RGBA: RGB from label, alpha from intensity
	color_vbo[gid * 4 + 0] = sel.x;
	color_vbo[gid * 4 + 1] = sel.y;
	color_vbo[gid * 4 + 2] = sel.z;
	color_vbo[gid * 4 + 3] = intensity;
}
)CLKER";

#endif//_COLOR_COMP_MESH_CODE_H_