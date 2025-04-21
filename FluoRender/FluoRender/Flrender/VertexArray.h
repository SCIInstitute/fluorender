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
		inline bool bind();
		inline void unbind();
		inline bool valid();
		inline void data(GLsizeiptr size, const GLvoid* data, GLenum usage);

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
		inline void bind();
		inline void unbind();
		inline void protect();
		inline void unprotect();
		inline bool valid();
		inline unsigned int id();

		inline bool attach_buffer(VertexBuffer* buf);
		void buffer_data(VABufferType type,
			GLsizeiptr size, const GLvoid* data, GLenum usage);
		inline void attrib_pointer(GLuint index,
			GLint size, GLenum type, GLboolean normalized,
			GLsizei stride, const GLvoid* pointer);

		//set parameters to generate vertices
		void set_param(unsigned int, double);
		void set_param(std::vector<std::pair<unsigned int, double>>& params);
		void set_param(fluo::BBox &box);//for bounding box
		void set_param(std::vector<fluo::Point> &pp);//for clipping planes
		void set_param(std::vector<float> &vts);//for floats
		inline bool get_dirty();//dirty array needs new data and update

		inline void draw_begin();
		inline void draw_end();
		inline void draw_arrays(GLenum, GLint, GLsizei);
		inline void draw_elements(GLenum, GLsizei, GLenum, const GLvoid*);
		inline void draw();
		inline void draw_norm_square();
		inline void draw_circles();
		inline void draw_bound_cube();
		inline void draw_grid();
		inline void draw_crop_frame();
		inline void draw_scale_bar();
		inline void draw_grad_bkg();
		inline void draw_color_map();
		inline void draw_traces();
		inline void draw_rulers();
		//clipping planes are drawn differently
		inline void draw_clip_plane(int plane, bool border);
		inline void draw_cam_jack(int axis);//0-x; 1-y; 2-z
		inline void draw_cam_center();
		inline void draw_legend_square(int index);//0-outside; 1-inside

		inline bool match(VAType);

	private:
		void update_buffer();
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

	inline void VertexArray::protect()
	{
		protected_ = true;
	}

	inline void VertexArray::unprotect()
	{
		protected_ = false;
	}

	inline bool VertexArray::valid()
	{
		return valid_;
	}

	inline unsigned int VertexArray::id()
	{
		return id_;
	}

	inline bool VertexArray::match(VAType type)
	{
		if (protected_)
			return false;
		if (type_ == type)
			return true;
		return false;
	}

	inline void VertexArrayManager::set_dirty(VAType type)
	{
		for (auto it = va_list_.begin();
			it != va_list_.end(); ++it)
		{
			if ((*it)->type_ == type)
				(*it)->dirty_ = true;
		}
	}

	inline void VertexArrayManager::set_all_dirty()
	{
		for (auto it = va_list_.begin();
			it != va_list_.end(); ++it)
			(*it)->dirty_ = true;
	}
}

#endif//VertexArray_h