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
inline constexpr const char* str_cl_marching_cubes_head = R"CLKER(
#define VSCL 255
#define MAX_INT 255
const sampler_t samp =
	CLK_NORMALIZED_COORDS_FALSE|
	CLK_ADDRESS_CLAMP_TO_EDGE|
	CLK_FILTER_NEAREST;

)CLKER";

inline constexpr const char* str_cl_marching_cubes_clip = R"CLKER(
float clip_plane(
	int3 coord,
	int volume_width,                           // nx
	int volume_height,                          // ny
	int volume_depth,                           // nz
	float3 scl,   //volume scale
	float3 trl,   //volume translate
	float4 loc10, //plane 0
	float4 loc11, //plane 1
	float4 loc12, //plane 2
	float4 loc13, //plane 3
	float4 loc14, //plane 4
	float4 loc15  //plane 5
)
{
	// Clipping planes
	float3 pt = (float3)((float)(coord.x) / (float)(volume_width), (float)(coord.y) / (float)(volume_height), (float)(coord.z) / (float)(volume_depth));
	pt = pt * scl + trl;
	if (dot(pt, loc10.xyz)+loc10.w < 0.0f ||
		dot(pt, loc11.xyz)+loc11.w < 0.0f ||
		dot(pt, loc12.xyz)+loc12.w < 0.0f ||
		dot(pt, loc13.xyz)+loc13.w < 0.0f ||
		dot(pt, loc14.xyz)+loc14.w < 0.0f ||
		dot(pt, loc15.xyz)+loc15.w < 0.0f)
		return 0.0f;
	return 1.0f;
}

)CLKER";

inline constexpr const char* str_cl_marching_cubes_tf = R"CLKER(
float transfer_function(
	__read_only image3d_t volume,               // 3D scalar field
	int3 coord,
	int volume_width,    // nx
	int volume_height,   // ny
	int volume_depth,    // nz
	float4 loc2,         // (scalar_scale, gm_scale, left_thresh, right_thresh)
	float4 loc3,         // (gamma, left_offset, right_offset, sw)
	float4 loc17         // (gm_low, gm_high, gm_max, 0)
)
{
	// Define local linear sampler (unnormalized coordinates, clamp, linear filter)
	const sampler_t linear_sampler = CLK_NORMALIZED_COORDS_FALSE |
									 CLK_ADDRESS_CLAMP |
									 CLK_FILTER_LINEAR;

	// Compute anisotropic z spacing
	float dz = fmin((float)volume_depth / (float)volume_width, 1.0f);

	// Convert coord to float for interpolation
	float x = (float)coord.x;
	float y = (float)coord.y;
	float z = (float)coord.z;

	// Sample center voxel intensity
	float3 v;
	v.x = read_imagef(volume, linear_sampler, (float4)(x, y, z, 1.0f)).x;

	// Compute gradient using central differences and float indexing
	float3 n = (float3)(0.0f);
	n.x = (read_imagef(volume, linear_sampler, (float4)(x + 1.0f, y, z, 1.0f)).x -
		   read_imagef(volume, linear_sampler, (float4)(x - 1.0f, y, z, 1.0f)).x) / 2.0f;
	n.y = (read_imagef(volume, linear_sampler, (float4)(x, y + 1.0f, z, 1.0f)).x -
		   read_imagef(volume, linear_sampler, (float4)(x, y - 1.0f, z, 1.0f)).x) / 2.0f;
	n.z = (read_imagef(volume, linear_sampler, (float4)(x, y, z + dz, 1.0f)).x -
		   read_imagef(volume, linear_sampler, (float4)(x, y, z - dz, 1.0f)).x) / (2.0f * dz);

	// Store gradient magnitude
	v.y = length(n);

	float c;
	v.x = loc2.x < 0.0f ? (1.0f + v.x * loc2.x) : v.x * loc2.x;
	if (v.x < loc2.z - loc3.w || (loc2.w < 1.0f && v.x > loc2.w + loc3.w))
		c = 0.0f;
	else
	{
		v.x *= v.x<loc2.z?(loc3.w-loc2.z+v.x)/loc3.w:(loc2.w<1.0f && v.x>loc2.w?(loc3.w-v.x+loc2.w)/loc3.w:1.0f);
		float gmf = 5.0f*(v.y-loc17.x)*(loc17.z-loc17.y)/loc17.z/(loc17.y-loc17.x);
		v.x *= v.y<loc17.x?v.y/loc17.x:1.0f+gmf*gmf;
		c = pow(clamp((v.x-loc3.y)/(loc3.z-loc3.y),
			loc3.x<1.0f?-(loc3.x-1.0f)*0.00001f:0.0f, 1.0f), loc3.x);
	}
	return c;
}

)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel0_nomask = R"CLKER(
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
)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel0_mask = R"CLKER(
__kernel void kernel_0(
	__read_only image3d_t volume,               // 3D scalar field
	__read_only image3d_t mask,                 // 3D mask field
	__global int* active_voxel_counter,         // Atomic counter
	float isovalue,                             // Isosurface threshold
	int volume_width,                           // nx
	int volume_height,                          // ny
	int volume_depth,                           // nz
	int xy_factor,                              // Coarsening factor for X and Y
	int z_factor                                // Coarsening factor for Z
)
)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel0_tf = R"CLKER(
__kernel void kernel_0(
	__read_only image3d_t volume,               // 3D scalar field
	__global int* active_voxel_counter,         // Atomic counter
	float isovalue,                             // Isosurface threshold
	int volume_width,                           // nx
	int volume_height,                          // ny
	int volume_depth,                           // nz
	int xy_factor,                              // Coarsening factor for X and Y
	int z_factor,                               // Coarsening factor for Z
	float4 loc2,  //(scalar_scale, gm_scale, left_thresh, right_thresh)
	float4 loc3,  //(gamma, left_offset, right_offset, sw)
	float4 loc17, //(gm_low, gm_high, gm_max, 0)
	float3 scl,   //volume scale
	float3 trl,   //volume translate
	float4 loc10, //plane 0
	float4 loc11, //plane 1
	float4 loc12, //plane 2
	float4 loc13, //plane 3
	float4 loc14, //plane 4
	float4 loc15  //plane 5
)
)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel_head = R"CLKER(
{
	int gx = get_global_id(0);
	int gy = get_global_id(1);
	int gz = get_global_id(2);

	int x = gx * xy_factor;
	int y = gy * xy_factor;
	int z = gz * z_factor;

	if (x >= volume_width - xy_factor || y >= volume_height - xy_factor || z >= volume_depth - z_factor)
		return;

	isovalue = isovalue * MAX_INT / VSCL;
)CLKER";

inline constexpr const char* str_cl_marching_cubes_read_volume = R"CLKER(
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

)CLKER";

inline constexpr const char* str_cl_marching_cubes_read_mask = R"CLKER(
	// Sample the 8 corners of the coarse voxel
	cube[0] = read_imagef(mask, samp, (int4)(x, y, z, 0)).x > 0.0 ? cube[0] : 0.0;
	cube[1] = read_imagef(mask, samp, (int4)(x+xy_factor, y, z, 0)).x > 0.0 ? cube[1] : 0.0;
	cube[2] = read_imagef(mask, samp, (int4)(x+xy_factor, y+xy_factor, z, 0)).x > 0.0 ? cube[2] : 0.0;
	cube[3] = read_imagef(mask, samp, (int4)(x, y+xy_factor, z, 0)).x > 0.0 ? cube[3] : 0.0;
	cube[4] = read_imagef(mask, samp, (int4)(x, y, z+z_factor, 0)).x > 0.0 ? cube[4] : 0.0;
	cube[5] = read_imagef(mask, samp, (int4)(x+xy_factor, y, z+z_factor, 0)).x > 0.0 ? cube[5] : 0.0;
	cube[6] = read_imagef(mask, samp, (int4)(x+xy_factor, y+xy_factor, z+z_factor, 0)).x > 0.0 ? cube[6] : 0.0;
	cube[7] = read_imagef(mask, samp, (int4)(x, y+xy_factor, z+z_factor, 0)).x > 0.0 ? cube[7] : 0.0;

)CLKER";

inline constexpr const char* str_cl_marching_cubes_apply_clip_tf = R"CLKER(
	// Apply clipping planes and transfer function
	int3 coords[8] = {
		(int3)(x,               y,               z           ), // corner 0
		(int3)(x + xy_factor,   y,               z           ), // corner 1
		(int3)(x + xy_factor,   y + xy_factor,   z           ), // corner 2
		(int3)(x,               y + xy_factor,   z           ), // corner 3
		(int3)(x,               y,               z + z_factor), // corner 4
		(int3)(x + xy_factor,   y,               z + z_factor), // corner 5
		(int3)(x + xy_factor,   y + xy_factor,   z + z_factor), // corner 6
		(int3)(x,               y + xy_factor,   z + z_factor)  // corner 7
	};
	for (int i = 0; i < 8; ++i) {
		cube[i] *= clip_plane(coords[i], volume_width, volume_height, volume_depth, scl, trl,
			loc10, loc11, loc12, loc13, loc14, loc15);
		cube[i] *= transfer_function(volume, coords[i], volume_width, volume_height, volume_depth,
			loc2, loc3, loc17);
	}

)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel0_tail = R"CLKER(
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

)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel1_func = R"CLKER(
float3 interpolate_vertex_scaled_offset(
	int x, int y, int z, int edge, float t,
	int xy_factor, int z_factor,
	int x_offset, int y_offset, int z_offset,
	__constant int* edge_pairs,
	__constant int* cube_table
)
{
	int vertex_index0 = edge_pairs[edge * 2 + 0];
	int vertex_index1 = edge_pairs[edge * 2 + 1];

	int3 delta0 = (int3)(
		cube_table[vertex_index0 * 3 + 0],
		cube_table[vertex_index0 * 3 + 1],
		cube_table[vertex_index0 * 3 + 2]
	);

	int3 delta1 = (int3)(
		cube_table[vertex_index1 * 3 + 0],
		cube_table[vertex_index1 * 3 + 1],
		cube_table[vertex_index1 * 3 + 2]
	);

	float3 base = (float3)(
		x + x_offset,
		y + y_offset,
		z + z_offset
	);

	float3 world_p0 = base + (float3)(
		delta0.x * xy_factor,
		delta0.y * xy_factor,
		delta0.z * z_factor
	);

	float3 world_p1 = base + (float3)(
		delta1.x * xy_factor,
		delta1.y * xy_factor,
		delta1.z * z_factor
	);

	return world_p0 + t * (world_p1 - world_p0);
}

)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel1_nomask = R"CLKER(
__kernel void kernel_1(
	__read_only image3d_t volume,
	__global float* vertex_buffer,
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
)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel1_mask = R"CLKER(
__kernel void kernel_1(
	__read_only image3d_t volume,
	__read_only image3d_t mask,
	__global float* vertex_buffer,
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
)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel1_tf = R"CLKER(
__kernel void kernel_1(
	__read_only image3d_t volume,
	__global float* vertex_buffer,
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
	int z_offset,
	float4 loc2,  //(scalar_scale, gm_scale, left_thresh, right_thresh)
	float4 loc3,  //(gamma, left_offset, right_offset, sw)
	float4 loc17, //(gm_low, gm_high, gm_max, 0)
	float3 scl,   //volume scale
	float3 trl,   //volume translate
	float4 loc10, //plane 0
	float4 loc11, //plane 1
	float4 loc12, //plane 2
	float4 loc13, //plane 3
	float4 loc14, //plane 4
	float4 loc15  //plane 5
)
)CLKER";

inline constexpr const char* str_cl_marching_cubes_kernel1_tail = R"CLKER(
	int cube_index = 0;
	for (int i = 0; i < 8; ++i)
		if (cube[i] < isovalue)
			cube_index |= (1 << i);

	int edges = edge_table[cube_index];
	if (edges == 0)
		return;

	float3 vert_list[12];
	for (int i = 0; i < 12; ++i) {
		if (edges & (1 << i)) {
			int v0 = edge_pairs[i * 2 + 0];
			int v1 = edge_pairs[i * 2 + 1];
			float denom = cube[v1] - cube[v0];

			if (fabs(denom) < 1e-6f)
				continue; // Skip this edge

			float t = (isovalue - cube[v0]) / denom;
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

		int float_idx = atomic_add(vertex_counter, 3) * 3;

		vertex_buffer[float_idx + 0] = vert_list[t0].x + 0.5;
		vertex_buffer[float_idx + 1] = vert_list[t0].y + 0.5;
		vertex_buffer[float_idx + 2] = vert_list[t0].z + 0.5;

		vertex_buffer[float_idx + 3] = vert_list[t1].x + 0.5;
		vertex_buffer[float_idx + 4] = vert_list[t1].y + 0.5;
		vertex_buffer[float_idx + 5] = vert_list[t1].z + 0.5;

		vertex_buffer[float_idx + 6] = vert_list[t2].x + 0.5;
		vertex_buffer[float_idx + 7] = vert_list[t2].y + 0.5;
		vertex_buffer[float_idx + 8] = vert_list[t2].z + 0.5;
	}
}
)CLKER";

//merge vertices
inline constexpr const char* str_cl_merge_vertices = R"CLKER(
__kernel void kernel_0(
	__global const float* vertex_buffer,   // [vertex_count * 3]
	__global int* remap_table,             // [vertex_count]
	const int vertex_count,
	const float merge_tolerance,
	const int window_size
)
{
	int i = get_global_id(0);
	if (i >= vertex_count) return;

	int base_idx = i * 3;
	float3 vi = (float3)(vertex_buffer[base_idx], vertex_buffer[base_idx + 1], vertex_buffer[base_idx + 2]);

	// Default: vertex maps to itself
	int remap_target = i;

	// Scan backward to enforce canonical remap direction (i > j)
	for (int j = max(0, i - window_size); j < i; ++j) {
		int cmp_idx = j * 3;
		float3 vj = (float3)(vertex_buffer[cmp_idx], vertex_buffer[cmp_idx + 1], vertex_buffer[cmp_idx + 2]);

		float dist2 = dot(vi - vj, vi - vj);
		if (dist2 < merge_tolerance * merge_tolerance) {
			remap_target = j;
			break; // First match wins
		}
	}

	remap_table[i] = remap_target;
}

__kernel void kernel_1(
	__global const int* remap_table,   // [vertex_count]
	__global int* unique_flags,        // [vertex_count]
	const int vertex_count
)
{
	//mark unique vertices
	int i = get_global_id(0);
	if (i >= vertex_count) return;

	// A vertex is unique if it maps to itself
	unique_flags[i] = (remap_table[i] == i) ? 1 : 0;
}

__kernel void kernel_2(
	__global const int* input,       // [vertex_count] -> unique_flags
	__global int* output,            // [vertex_count] -> prefix_sum
	__local int* temp,               // [2 * local_size] -> shared memory
	const int vertex_count
)
{
	// Blelloch scan (exclusive prefix sum) in shared memory
	int tid = get_local_id(0);
	int gid = get_global_id(0);
	int offset = 1;

	// Load input into shared memory
	if (gid < vertex_count)
		temp[2 * tid]     = input[gid];
	else
		temp[2 * tid]     = 0;

	temp[2 * tid + 1] = 0;

	barrier(CLK_LOCAL_MEM_FENCE);

	// Up-sweep (reduce) phase
	for (int d = get_local_size(0); d > 0; d >>= 1) {
		barrier(CLK_LOCAL_MEM_FENCE);
		if (tid < d) {
			int ai = offset * (2 * tid + 1) - 1;
			int bi = offset * (2 * tid + 2) - 1;
			temp[bi] += temp[ai];
		}
		offset <<= 1;
	}

	// Clear last element for exclusive scan
	if (tid == 0) {
		temp[2 * get_local_size(0) - 1] = 0;
	}

	// Down-sweep phase
	for (int d = 1; d < get_local_size(0) * 2; d <<= 1) {
		offset >>= 1;
		barrier(CLK_LOCAL_MEM_FENCE);
		if (tid < d) {
			int ai = offset * (2 * tid + 1) - 1;
			int bi = offset * (2 * tid + 2) - 1;
			int t = temp[ai];
			temp[ai] = temp[bi];
			temp[bi] += t;
		}
	}

	barrier(CLK_LOCAL_MEM_FENCE);

	// Write result to global memory
	if (gid < vertex_count)
		output[gid] = temp[2 * tid];
}

__kernel void kernel_3(
	__global const float* vertex_buffer,   // [vertex_count * 3]
	__global const int* remap_table,       // [vertex_count]
	__global int* unique_flags,            // [vertex_count]
	__global int* prefix_sum,              // [vertex_count]
	__global int* remap_to_compact,        // [vertex_count]
	__global float* compacted_vbo,         // [new_vertex_count * 3]
	__global int* final_indices,           // [vertex_count]
	const int vertex_count
)
{
	// compact and remap
	int i = get_global_id(0);
	if (i >= vertex_count) return;

	// Step 1: Identify unique vertices
	if (remap_table[i] == i) {
		unique_flags[i] = 1;
	} else {
		unique_flags[i] = 0;
	}

	barrier(CLK_GLOBAL_MEM_FENCE); // Ensure flags are written

	// Step 2: Use prefix_sum[i] as compacted index (assume precomputed)
	if (unique_flags[i]) {
		int new_idx = prefix_sum[i];
		int base = i * 3;
		int out_base = new_idx * 3;
		compacted_vbo[out_base + 0] = vertex_buffer[base + 0];
		compacted_vbo[out_base + 1] = vertex_buffer[base + 1];
		compacted_vbo[out_base + 2] = vertex_buffer[base + 2];
	}

	barrier(CLK_GLOBAL_MEM_FENCE); // Ensure VBO is written

	// Step 3: Update final index array
	int target = remap_table[i];
	final_indices[i] = remap_to_compact[target];
}

__kernel void kernel_4(
	__global const float* vertex_buffer,   // [vertex_count * 3]
	__global int* remap_table,             // [vertex_count]
	const int vertex_count,
	const float merge_tolerance,
	const int lower_bound,
	const int upper_bound
)
{
	int i = get_global_id(0) + lower_bound;
	if (i >= upper_bound || i >= vertex_count) return;

	int base_idx = i * 3;
	float3 vi = (float3)(vertex_buffer[base_idx], vertex_buffer[base_idx + 1], vertex_buffer[base_idx + 2]);

	int remap_target = i;

	for (int j = 0; j < i; ++j) {
		int cmp_idx = j * 3;
		float3 vj = (float3)(vertex_buffer[cmp_idx], vertex_buffer[cmp_idx + 1], vertex_buffer[cmp_idx + 2]);

		float dist2 = dot(vi - vj, vi - vj);
		if (dist2 < merge_tolerance * merge_tolerance) {
			remap_target = j;
			break;
		}
	}

	remap_table[i] = remap_target;
}

__kernel void kernel_5(
	__global const float* vertex_buffer,     // [vertex_count * 3] - compacted VBO from windowed pass
	__global const int* old_ibo,             // [index_count] - index buffer from windowed pass
	__global const int* remap_table,         // [vertex_count] - global remap table
	__global int* unique_flags,              // [vertex_count] - output: flags for unique vertices
	__global int* prefix_sum,                // [vertex_count] - precomputed prefix sum
	__global int* remap_to_compact,          // [vertex_count] - global ID -> compacted index
	__global float* compacted_vbo,           // [new_vertex_count * 3] - output: globally compacted VBO
	__global int* final_indices,             // [index_count] - output: globally remapped IBO
	const int vertex_count,
	const int index_count
)
{
	int i = get_global_id(0);

	// Step 1: Compact VBO (vertex-side work)
	if (i < vertex_count) {
		// Identify unique vertices
		if (remap_table[i] == i) {
			unique_flags[i] = 1;
		} else {
			unique_flags[i] = 0;
		}

		barrier(CLK_GLOBAL_MEM_FENCE); // Ensure flags are written

		// Write compacted vertex
		if (unique_flags[i]) {
			int new_idx = prefix_sum[i];
			int base = i * 3;
			int out_base = new_idx * 3;
			compacted_vbo[out_base + 0] = vertex_buffer[base + 0];
			compacted_vbo[out_base + 1] = vertex_buffer[base + 1];
			compacted_vbo[out_base + 2] = vertex_buffer[base + 2];
		}
	}

	barrier(CLK_GLOBAL_MEM_FENCE); // Ensure VBO is written

	// Step 2: Remap IBO (index-side work)
	if (i < index_count) {
		int old_idx = old_ibo[i];               // index from windowed pass
		int global_idx = remap_table[old_idx];  // canonical global vertex ID
		int compact_idx = remap_to_compact[global_idx]; // final compacted index
		final_indices[i] = compact_idx;
	}
}
)CLKER";

//compute normals for smooth shading
inline constexpr const char* str_cl_smooth_normals_float_atomic_supported = R"CLKER(
inline void atomic_add3(__global float* ptr, float3 value)
{
	atomic_add(&ptr[0], value.x);
	atomic_add(&ptr[1], value.y);
	atomic_add(&ptr[2], value.z);
}
)CLKER";

inline constexpr const char* str_cl_smooth_normals_float_atomic_unsupported = R"CLKER(
inline void atomic_add_float(__global float* ptr, float value)
{
	__global int* iptr = (__global int*)(__global void*)ptr;
	union { float f; int i; } old_val, new_val;

	int max_iter = 100;
	while (--max_iter > 0)
	{
		old_val.i = atomic_cmpxchg(iptr, 0, 0); // atomic read
		new_val.f = as_float(old_val.i) + value;
		new_val.i = as_int(new_val.f);
		if (atomic_cmpxchg(iptr, old_val.i, new_val.i) == old_val.i)
			break;
	}
}
inline void atomic_add3(__global float* ptr, float3 value)
{
	atomic_add_float(&ptr[0], value.x);
	atomic_add_float(&ptr[1], value.y);
	atomic_add_float(&ptr[2], value.z);
}
)CLKER";

inline constexpr const char* str_cl_smooth_normals = R"CLKER(
__kernel void kernel_0(
	__global const float* vertex_buffer,   // [vertex_count * 3]
	__global const int* index_buffer,      // [idx_count]
	__global float* normal_buffer,         // [vertex_count * 3]
	const int idx_count)
{
	// Accumulation kernel
	int i = get_global_id(0);
	if (i >= idx_count / 3) return;

	int idx0 = index_buffer[i * 3 + 0];
	int idx1 = index_buffer[i * 3 + 1];
	int idx2 = index_buffer[i * 3 + 2];

	float3 v0 = vload3(idx0, vertex_buffer);
	float3 v1 = vload3(idx1, vertex_buffer);
	float3 v2 = vload3(idx2, vertex_buffer);

	float3 face_normal = normalize(cross(v1 - v0, v2 - v0));

	// Atomically accumulate to each vertex
	atomic_add3(&normal_buffer[idx0 * 3], face_normal);
	atomic_add3(&normal_buffer[idx1 * 3], face_normal);
	atomic_add3(&normal_buffer[idx2 * 3], face_normal);
}

__kernel void kernel_1(
	__global float* normal_buffer,
	const int vertex_count)
{
	// Normalization kernel
	int i = get_global_id(0);
	if (i >= vertex_count) return;

	float3 n = vload3(i, normal_buffer);
	n = normalize(n);
	vstore3(n, i, normal_buffer);
}
)CLKER";

//simplify mesh
inline constexpr const char* str_cl_mesh_simplify = R"CLKER(
__kernel void kernel_0(
	__global const float* vertex_buffer,   // [vertex_count * 3]
	__global const int* index_buffer,      // [idx_count]
	__global int* tri_mask,                // [idx_count/3], 1 = keep, 0 = remove
	const float collapse_threshold,
	const int idx_count)
{
	//mark triangles for removal
	int i = get_global_id(0);
	if (i >= idx_count / 3) return;

	int idx0 = index_buffer[i * 3 + 0];
	int idx1 = index_buffer[i * 3 + 1];
	int idx2 = index_buffer[i * 3 + 2];

	float3 v0 = vload3(idx0, vertex_buffer);
	float3 v1 = vload3(idx1, vertex_buffer);
	float3 v2 = vload3(idx2, vertex_buffer);

	// Compute squared edge lengths
	float e0 = dot(v1 - v0, v1 - v0);
	float e1 = dot(v2 - v1, v2 - v1);
	float e2 = dot(v0 - v2, v0 - v2);

	float min_edge = fmin(e0, fmin(e1, e2));

	// If the smallest edge is below threshold, mark for collapse
	tri_mask[i] = (min_edge < collapse_threshold * collapse_threshold) ? 0 : 1;
}

__kernel void kernel_1(
	__global const int* index_buffer,      // original
	__global const int* tri_mask,          // 1 = keep, 0 = remove
	__global const int* tri_prefix_sum,    // prefix sum of tri_mask
	__global int* new_index_buffer,        // compacted output
	const int idx_count)
{
	//rewrite index buffer
	int i = get_global_id(0);
	if (i >= idx_count / 3) return;

	if (tri_mask[i] == 0) return;

	int new_tri_id = tri_prefix_sum[i];
	int dst = new_tri_id * 3;

	new_index_buffer[dst + 0] = index_buffer[i * 3 + 0];
	new_index_buffer[dst + 1] = index_buffer[i * 3 + 1];
	new_index_buffer[dst + 2] = index_buffer[i * 3 + 2];
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