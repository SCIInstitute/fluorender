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

#ifndef __glew_h__
typedef ptrdiff_t GLsizeiptr;
typedef void GLvoid;
typedef unsigned int GLenum;
typedef int GLint;
typedef unsigned int GLuint;
typedef unsigned char GLboolean;
typedef int GLsizei; 
#endif

namespace fluo
{
	class Point;
}
namespace flvr
{
	enum VABufferType
	{
		VABuf_Coord = 0,
		VABuf_Index,
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
		void data(GLsizeiptr size, const GLvoid* data, GLenum usage);
		//read back
		void* map(GLenum access);
		void unmap();

	private:
		unsigned int id_;
		VABufferType type_;
		bool valid_;

		friend class VertexArray;
	};

	enum VAType
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

		bool attach_buffer(VertexBuffer* buf);
		void buffer_data(VABufferType type,
			GLsizeiptr size, const GLvoid* data, GLenum usage);
		void attrib_pointer(GLuint index,
			GLint size, GLenum type, GLboolean normalized,
			GLsizei stride, const GLvoid* pointer);

		//read back
		void* map_buffer(VABufferType type, GLenum access);
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
		void draw_arrays(GLenum, GLint, GLsizei);
		void draw_elements(GLenum, GLsizei, GLenum, const GLvoid*);
		void draw();
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
		std::vector<VertexBuffer*> buffer_list_;
		std::vector<GLuint> attrib_pointer_list_;
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

		VertexArray* vertex_array(VAType type);//managed
		VertexArray* vertex_array(bool, bool);//unmanaged
		void set_dirty(VAType type);
		void set_all_dirty();

	private:
		std::vector<VertexArray*> va_list_;
		std::vector<VertexBuffer*> vb_list_;
	};

}

#endif//VertexArray_h