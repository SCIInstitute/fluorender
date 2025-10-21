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

#ifndef VertexArray_h
#define VertexArray_h

#include <BBox.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <set>

namespace fluo
{
	class Point;
}
namespace flvr
{
	enum class VABufferType
	{
		VABuf_Coord,
		VABuf_Index,
		VABuf_Normal,
		VABuf_Tex,
		VABuf_Color
	};
	enum class VAAttribIndex
	{
		VAAttrib_Coord,
		VAAttrib_Normal,
		VAAttrib_Tex,
		VAAttrib_Color
	};

	enum class BufferAccess
	{
		ReadOnly,       // GL_READ_ONLY
		WriteOnly,      // GL_WRITE_ONLY
		ReadWrite       // GL_READ_WRITE
	};

	enum class BufferUsage
	{
		StreamDraw,   // GL_STREAM_DRAW
		StreamRead,   // GL_STREAM_READ
		StreamCopy,   // GL_STREAM_COPY
		StaticDraw,   // GL_STATIC_DRAW
		StaticRead,   // GL_STATIC_READ
		StaticCopy,   // GL_STATIC_COPY
		DynamicDraw,  // GL_DYNAMIC_DRAW
		DynamicRead,  // GL_DYNAMIC_READ
		DynamicCopy   // GL_DYNAMIC_COPY
	};

	enum class VertexAttribType
	{
		Byte,           // GL_BYTE
		UnsignedByte,   // GL_UNSIGNED_BYTE
		Short,          // GL_SHORT
		UnsignedShort,  // GL_UNSIGNED_SHORT
		Int,            // GL_INT
		UnsignedInt,    // GL_UNSIGNED_INT
		Float,          // GL_FLOAT
		Double          // GL_DOUBLE
	};

	enum class PrimitiveType
	{
		Points,         // GL_POINTS
		Lines,          // GL_LINES
		LineStrip,      // GL_LINE_STRIP
		Triangles,      // GL_TRIANGLES
		TriangleStrip,  // GL_TRIANGLE_STRIP
		TriangleFan     // GL_TRIANGLE_FAN
	};

	enum class IndexType
	{
		UnsignedByte,   // GL_UNSIGNED_BYTE
		UnsignedShort,  // GL_UNSIGNED_SHORT
		UnsignedInt     // GL_UNSIGNED_INT
	};

	class VertexArray;
	class VertexArrayManager;
	class VertexBuffer
	{
	public:
		VertexBuffer(VABufferType type);
		~VertexBuffer();

		void create();
		void destroy();
		bool bind();
		void unbind();
		bool valid();
		void data(size_t size, const void* data, BufferUsage usage);
		//read back
		void* map(BufferAccess access);
		void unmap();

	private:
		unsigned int id_;
		VABufferType type_;
		bool valid_;

		friend class VertexArray;
	};

	enum class VAType
	{
		VA_Unmanaged = 1,
		VA_Norm_Square,
		VA_Rectangle,
		VA_Norm_Square_d,
		VA_Left_Square,
		VA_Right_Square,
		VA_Brush_Circles,
		VA_Bound_Cube,
		VA_Clip_Planes,
		VA_Grid,
		VA_Cam_Jack,
		VA_Cam_Center,
		VA_Crop_Frame,
		VA_Scale_Bar,
		VA_Legend_Squares,
		VA_Grad_Bkg,
		VA_Color_Map,
		VA_Traces,
		VA_Rulers,
		VA_Text,
	};
	class VertexArray
	{
	public:
		VertexArray(VAType type);
		~VertexArray();

		void create();
		void destroy();
		void bind();
		void unbind();
		void protect();
		void unprotect();
		bool valid();
		unsigned int id_array();
		unsigned int id_buffer(VABufferType type);
		void add_index_buffer();
		void delete_index_buffer();
		bool is_indexed() { return indexed_; }
		bool is_interleaved() { return interleaved_; }
		void add_buffer(VABufferType type);
		void delete_buffer(VABufferType type);

		bool attach_buffer(const std::shared_ptr<VertexBuffer>& buf);
		void buffer_data(VABufferType type,
			size_t size, const void* data, BufferUsage usage);
		void attrib_pointer(unsigned int index,
			int size, VertexAttribType type, bool normalized,
			size_t stride, const void* pointer);
		void remove_attrib_pointer(unsigned int index);

		//read back
		void* map_buffer(VABufferType type, BufferAccess access);
		void unmap_buffer(VABufferType type);

		//set parameters to generate vertices
		void set_param(unsigned int, double);
		void set_param(std::vector<std::pair<unsigned int, double>>& params);
		void set_param(fluo::BBox &box);//for bounding box
		void set_param(std::vector<fluo::Point> &pp);//for clipping planes
		void set_param(std::vector<float> &vts);//for floats
		bool get_dirty();//dirty array needs new data and update

		void draw_begin();
		void draw_end();
		void draw_arrays(PrimitiveType, int, size_t);
		void draw_elements(PrimitiveType, size_t, IndexType, const void*);
		void draw();
		void draw_unmanaged(int pos, int tri_num);
		void draw_norm_square();
		void draw_circles();
		void draw_bound_cube();
		void draw_grid();
		void draw_crop_frame();
		void draw_scale_bar();
		void draw_grad_bkg();
		void draw_color_map();
		void draw_traces();
		void draw_rulers();
		//clipping planes are drawn differently
		void draw_clip_plane(int plane, bool border);
		void draw_cam_jack(int axis);//0-x; 1-y; 2-z
		void draw_cam_center();
		void draw_legend_square(int index);//0-outside; 1-inside

		bool match(VAType);

	private:
		void update_buffer();
		//parameters: 0-tex_aspect; 1-view aspect
		void update_buffer_rectangle();
		//parameters: 0-tex z depth
		void update_buffer_norm_square_d();
		//parameters: 0-r1; 1-r2; 2-sections
		void update_buffer_circles();
		//parameters: the bounding box
		void update_bound_cube();
		//parameters: vector of points (8)
		void update_clip_planes(bool update_index = false);
		//parameters: 0-grid num; 1-distance
		void update_grid();
		//parameters: 0-length
		void update_cam_jack();
		//parameters: 0-length
		void update_cam_center();
		//parameters: x, y, w, h
		void update_crop_frame();
		//parameters: x, y, w, h
		void update_scale_bar();
		//parameters: px1, py1, px2, py2
		void update_legend_squares();
		//parameters:vector
		void update_grad_bkg();

	private:
		unsigned int id_;
		VAType type_;
		bool valid_;
		bool protected_;
		bool dirty_;
		bool indexed_;
		bool interleaved_;//normals and tex coords are saved with vertex coords
		std::vector<std::shared_ptr<VertexBuffer>> buffer_list_;
		std::set<VAAttribIndex> attrib_pointer_list_;
		//parameters
		std::map<unsigned int, double> param_list_;//generic
		fluo::BBox bbox_;//for bounding box
		std::vector<float> float_list_;//for clipping planes

		friend class VertexArrayManager;
	};

	class VertexArrayManager
	{
	public:
		VertexArrayManager();
		~VertexArrayManager();
		void clear();

		std::shared_ptr<VertexArray> vertex_array(VAType type);//managed
		std::shared_ptr<VertexArray> vertex_array(bool, bool);//unmanaged
		void set_dirty(VAType type);
		void set_all_dirty();

	private:
		std::vector<std::shared_ptr<VertexArray>> va_list_;
	};

}

#endif//VertexArray_h