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
#ifndef MESH_CL_H
#define MESH_CL_H

//marching cubes
inline constexpr const char* str_cl_marching_cubes = R"CLKER(
__constant int edge_flags[256];       // 256 entries, each a 12-bit mask
__constant int tri_table[256][16];    // Each row lists up to 5 triangles (15 indices + -1 terminator)
__constant int edge_pairs[12][2];     // Maps edge index to corner indices

inline float4 interpolate_vertex(int x, int y, int z, int edge_idx, float t)
{
	// Each edge connects two cube corners; we define their relative positions
	const int3 edge_offsets[12][2] = {
		{{0,0,0}, {1,0,0}}, {{1,0,0}, {1,1,0}}, {{1,1,0}, {0,1,0}}, {{0,1,0}, {0,0,0}},
		{{0,0,1}, {1,0,1}}, {{1,0,1}, {1,1,1}}, {{1,1,1}, {0,1,1}}, {{0,1,1}, {0,0,1}},
		{{0,0,0}, {0,0,1}}, {{1,0,0}, {1,0,1}}, {{1,1,0}, {1,1,1}}, {{0,1,0}, {0,1,1}}
	};

	int3 p0 = edge_offsets[edge_idx][0];
	int3 p1 = edge_offsets[edge_idx][1];

	float3 pos0 = (float3)(x + p0.x, y + p0.y, z + p0.z);
	float3 pos1 = (float3)(x + p1.x, y + p1.y, z + p1.z);

	float3 interp = pos0 + t * (pos1 - pos0);
	return (float4)(interp.x, interp.y, interp.z, 1.0f);
}

__kernel void kernel_0(
	__read_only image3d_t volume,         // 3D volume texture
	__global float4* vertex_buffer,       // Output vertex buffer (shared with OpenGL)
	__global int* vertex_counter,         // Atomic counter for vertex allocation
	float isovalue,                       // Isosurface threshold
	__constant float4* edge_table,        // Precomputed edge interpolation table
	int volume_width,
	int volume_height,
	int volume_depth
)
{
	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);

	if (x >= volume_width - 1 || y >= volume_height - 1 || z >= volume_depth - 1)
		return;

	// Sample voxel values at cube corners
	float cube[8];
	cube[0] = read_imagef(volume, sampler, (int4)(x, y, z, 0)).x;
	cube[1] = read_imagef(volume, sampler, (int4)(x+1, y, z, 0)).x;
	cube[2] = read_imagef(volume, sampler, (int4)(x+1, y+1, z, 0)).x;
	cube[3] = read_imagef(volume, sampler, (int4)(x, y+1, z, 0)).x;
	cube[4] = read_imagef(volume, sampler, (int4)(x, y, z+1, 0)).x;
	cube[5] = read_imagef(volume, sampler, (int4)(x+1, y, z+1, 0)).x;
	cube[6] = read_imagef(volume, sampler, (int4)(x+1, y+1, z+1, 0)).x;
	cube[7] = read_imagef(volume, sampler, (int4)(x, y+1, z+1, 0)).x;

	// Determine cube index
	int cube_index = 0;
	for (int i = 0; i < 8; ++i)
		if (cube[i] < isovalue)
			cube_index |= (1 << i);

	// Lookup edge configuration
	int edges = edge_flags[cube_index];
	if (edges == 0)
		return;

	// Interpolate vertices along edges
	float4 vert_list[12];
	for (int i = 0; i < 12; ++i) {
		if (edges & (1 << i)) {
			int v0 = edge_pairs[i][0];
			int v1 = edge_pairs[i][1];
			float t = (isovalue - cube[v0]) / (cube[v1] - cube[v0]);
			vert_list[i] = interpolate_vertex(x, y, z, i, t);
		}
	}

	// Emit triangles
	for (int i = 0; tri_table[cube_index][i] != -1; i += 3) {
		int idx = atomic_add(vertex_counter, 3);
		vertex_buffer[idx + 0] = vert_list[tri_table[cube_index][i + 0]];
		vertex_buffer[idx + 1] = vert_list[tri_table[cube_index][i + 1]];
		vertex_buffer[idx + 2] = vert_list[tri_table[cube_index][i + 2]];
	}
}
)CLKER";

#endif//MESH_CL_H