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
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

__kernel void kernel_0(
	__read_only image3d_t volume,               // 3D scalar field
	__global int* active_voxel_counter,         // Atomic counter
	float isovalue,                             // Isosurface threshold
	int volume_width,                           // nx
	int volume_height,                          // ny
	int volume_depth,                           // nz
	int xy_factor,                              // Coarsening factor for X and Y
	int z_factor                                // Coarsening factor for Z
)
{
	int gx = get_global_id(0);
	int gy = get_global_id(1);
	int gz = get_global_id(2);

	int x = gx * xy_factor;
	int y = gy * xy_factor;
	int z = gz * z_factor;

	if (x >= volume_width - xy_factor || y >= volume_height - xy_factor || z >= volume_depth - z_factor)
		return;

	// Sample the 8 corners of the coarse voxel
	float cube[8];
	cube[0] = read_imagef(volume, samp, (int4)(x, y, z, 0)).x;
	cube[1] = read_imagef(volume, samp, (int4)(x+xy_factor, y, z, 0)).x;
	cube[2] = read_imagef(volume, samp, (int4)(x+xy_factor, y+xy_factor, z, 0)).x;
	cube[3] = read_imagef(volume, samp, (int4)(x, y+xy_factor, z, 0)).x;
	cube[4] = read_imagef(volume, samp, (int4)(x, y, z+z_factor, 0)).x;
	cube[5] = read_imagef(volume, samp, (int4)(x+xy_factor, y, z+z_factor, 0)).x;
	cube[6] = read_imagef(volume, samp, (int4)(x+xy_factor, y+xy_factor, z+z_factor, 0)).x;
	cube[7] = read_imagef(volume, samp, (int4)(x, y+xy_factor, z+z_factor, 0)).x;

	// Compute cube index
	int cube_index = 0;
	for (int i = 0; i < 8; ++i)
		if (cube[i] < isovalue)
			cube_index |= (1 << i);

	// Skip empty or full cubes
	if (cube_index == 0 || cube_index == 255)
		return;

	// Count active voxel
	atomic_inc(active_voxel_counter);
}

int3 get_edge_vertex(
	int x, int y, int z,
	int edge, int endpoint,
	__constant int* edge_pairs,
	__constant int* cube_table
)
{
	// Each edge has two vertex indices: edge_pairs[edge * 2 + endpoint]
	int vertex_index = edge_pairs[edge * 2 + endpoint];

	// Each vertex has 3 components: cube_table[vertex_index * 3 + {0,1,2}]
	int dx = cube_table[vertex_index * 3 + 0];
	int dy = cube_table[vertex_index * 3 + 1];
	int dz = cube_table[vertex_index * 3 + 2];

	return (int3)(x + dx, y + dy, z + dz);
}

float3 interpolate_vertex_scaled_offset(
	int x, int y, int z, int edge, float t,
	int xy_factor, int z_factor,
	int x_offset, int y_offset, int z_offset,
	__constant int* edge_pairs,
	__constant int* cube_table
)
{
	int3 p0 = get_edge_vertex(x, y, z, edge, 0, edge_pairs, cube_table);
	int3 p1 = get_edge_vertex(x, y, z, edge, 1, edge_pairs, cube_table);

	float3 world_p0 = (float3)(
		p0.x * xy_factor + x_offset,
		p0.y * xy_factor + y_offset,
		p0.z * z_factor + z_offset
	);

	float3 world_p1 = (float3)(
		p1.x * xy_factor + x_offset,
		p1.y * xy_factor + y_offset,
		p1.z * z_factor + z_offset
	);

	return world_p0 + t * (world_p1 - world_p0);
}

__kernel void kernel_1(
	__read_only image3d_t volume,
	__global float3* vertex_buffer,
	__global int* vertex_counter,
	__constant int* cube_table,
	__constant int* edge_table,
	__constant int* tri_table,
	__constant int* edge_pairs,
	float isovalue,
	int volume_width,
	int volume_height,
	int volume_depth,
	int xy_factor,
	int z_factor,
	int x_offset,
	int y_offset,
	int z_offset
)
{
	int gx = get_global_id(0);
	int gy = get_global_id(1);
	int gz = get_global_id(2);

	int x = gx * xy_factor;
	int y = gy * xy_factor;
	int z = gz * z_factor;

	if (x >= volume_width - xy_factor || y >= volume_height - xy_factor || z >= volume_depth - z_factor)
		return;

	float cube[8];
	cube[0] = read_imagef(volume, samp, (int4)(x, y, z, 0)).x;
	cube[1] = read_imagef(volume, samp, (int4)(x+xy_factor, y, z, 0)).x;
	cube[2] = read_imagef(volume, samp, (int4)(x+xy_factor, y+xy_factor, z, 0)).x;
	cube[3] = read_imagef(volume, samp, (int4)(x, y+xy_factor, z, 0)).x;
	cube[4] = read_imagef(volume, samp, (int4)(x, y, z+z_factor, 0)).x;
	cube[5] = read_imagef(volume, samp, (int4)(x+xy_factor, y, z+z_factor, 0)).x;
	cube[6] = read_imagef(volume, samp, (int4)(x+xy_factor, y+xy_factor, z+z_factor, 0)).x;
	cube[7] = read_imagef(volume, samp, (int4)(x, y+xy_factor, z+z_factor, 0)).x;

	int cube_index = 0;
	for (int i = 0; i < 8; ++i)
		if (cube[i] < isovalue)
			cube_index |= (1 << i);

if (cube_index == 0 || cube_index == 255)
	return;
// Emit a dummy triangle for each active voxel
int idx = atomic_add(vertex_counter, 3);

// Define three arbitrary vertices within the voxel bounds
float3 v0 = (float3)(x + x_offset,     y + y_offset,     z + z_offset);
float3 v1 = (float3)(x + x_offset + 1, y + y_offset,     z + z_offset);
float3 v2 = (float3)(x + x_offset,     y + y_offset + 1, z + z_offset);
vertex_buffer[idx + 0] = v0;
vertex_buffer[idx + 1] = v1;
vertex_buffer[idx + 2] = v2;
return;

	int edges = edge_table[cube_index];
	if (edges == 0)
		return;

	float3 vert_list[12];
	for (int i = 0; i < 12; ++i) {
		if (edges & (1 << i)) {
			int v0 = edge_pairs[i * 2 + 0];
			int v1 = edge_pairs[i * 2 + 1];
			float t = (isovalue - cube[v0]) / (cube[v1] - cube[v0]);

			vert_list[i] = interpolate_vertex_scaled_offset(
				x, y, z, i, t,
				xy_factor, z_factor,
				x_offset, y_offset, z_offset,
				edge_pairs, cube_table
			);
		}
	}
	
	// Output triangles
	for (int i = 0; ; i += 3) {
		int t0 = tri_table[cube_index * 16 + i + 0];
		if (t0 == -1)
			break;
		int t1 = tri_table[cube_index * 16 + i + 1];
		int t2 = tri_table[cube_index * 16 + i + 2];

		int idx = atomic_add(vertex_counter, 3);
		vertex_buffer[idx + 0] = vert_list[t0];
		vertex_buffer[idx + 1] = vert_list[t1];
		vertex_buffer[idx + 2] = vert_list[t2];
	}
}
)CLKER";

//merge vertices
inline constexpr const char* str_cl_merge_vertices = R"CLKER(
__kernel void kernel_0(
	__global float4* raw_vertices,
	__global float4* merged_vertices,
	__global int* indices,
	__global int* hash_table,
	__global int* merge_counter,
	int num_raw_vertices,
	float voxel_size
)
{
	int id = get_global_id(0);
	if (id >= num_raw_vertices) return;

	float4 pos = raw_vertices[id];

	// Quantize position to grid
	int3 key = (int3)(
		floor(pos.x / voxel_size),
		floor(pos.y / voxel_size),
		floor(pos.z / voxel_size)
	);

	// Simple hash function
	int hash = (key.x * 73856093) ^ (key.y * 19349663) ^ (key.z * 83492791);
	hash = abs(hash) % HASH_TABLE_SIZE;

	// Try to insert into hash table
	int existing = atomic_cmpxchg(&hash_table[hash], -1, id);

	int final_index;
	if (existing == -1)
	{
		// First time this position is seen
		int new_idx = atomic_add(merge_counter, 1);
		merged_vertices[new_idx] = pos;
		final_index = new_idx;
	}
	else
	{
		// Already inserted
		final_index = existing;
	}

	// Write index for triangle
	indices[id] = final_index;
}
)CLKER";

//simplify mesh
inline constexpr const char* str_cl_simplify_mesh = R"CLKER(
__kernel void kernel_0(
	__global float4* vertices,
	__global int* indices,
	__global int* valid_flags,
	int num_triangles,
	float threshold
)
{
	int tid = get_global_id(0);
	if (tid >= num_triangles) return;

	int i0 = indices[tid * 3 + 0];
	int i1 = indices[tid * 3 + 1];
	int i2 = indices[tid * 3 + 2];

	float4 v0 = vertices[i0];
	float4 v1 = vertices[i1];
	float4 v2 = vertices[i2];

	float d01 = length(v0.xyz - v1.xyz);
	float d12 = length(v1.xyz - v2.xyz);
	float d20 = length(v2.xyz - v0.xyz);

	if (d01 < threshold || d12 < threshold || d20 < threshold)
	{
		valid_flags[tid] = 0; // mark for removal
	}
	else
	{
		valid_flags[tid] = 1;
	}
}
)CLKER";

//laplacian smooth
inline constexpr const char* str_cl_laplacian_smooth = R"CLKER(
__kernel void kernel_0(
	__global float4* vertices,
	__global int* adjacency,
	__global int* adj_count,
	__global float4* smoothed_vertices,
	int num_vertices
)
{
	int id = get_global_id(0);
	if (id >= num_vertices) return;

	float4 v = vertices[id];
	int count = adj_count[id];
	if (count == 0)
	{
		smoothed_vertices[id] = v;
		return;
	}

	float4 sum = (float4)(0.0f);
	for (int i = 0; i < count; ++i)
	{
		int neighbor_id = adjacency[id * MAX_NEIGHBORS + i];
		sum += vertices[neighbor_id];
	}

	float4 avg = sum / (float)count;
	smoothed_vertices[id] = mix(v, avg, 0.5f); // blend factor = 0.5
}
)CLKER";

#endif//MESH_CL_H