//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
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

#include <GL/glew.h>
#include <string>
#include <vector>
#include <map>
#include <FLIVR/BBox.h>
#include <FLIVR/Point.h>

namespace FLIVR
{
	enum VABufferType
	{
		VABuf_Coord = 0,
		VABuf_index,
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
		VA_Norm_Square = 0,
		VA_Norm_Square_d,
		VA_Brush_Circles,
		VA_Bound_Cube,
		VA_Clip_Planes,
		VA_Grid,
		VA_Cam_Jack,
		VA_Crop_Frame,
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
		void set_param(BBox &box);//for bounding box
		void set_param(std::vector<Point> &pp);//for clipping planes

		inline void draw_begin();
		inline void draw_end();
		inline void draw();
		inline void draw_norm_square();
		inline void draw_circles();
		inline void draw_bound_cube();
		inline void draw_grid();
		inline void draw_crop_frame();
		//clipping planes are drawn differently
		inline void draw_clip_plane(int plane, bool border);
		inline void draw_cam_jack(int axis);//0-x; 1-y; 2-z

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
		//parameters: x, y, w, h
		void update_crop_frame();

	private:
		unsigned int id_;
		VAType type_;
		bool valid_;
		bool protected_;
		std::vector<VertexBuffer*> buffer_list_;
		std::vector<GLuint> attrib_pointer_list_;
		//parameters
		std::map<unsigned int, double> param_list_;//generic
		BBox bbox_;//for bounding box
		std::vector<float> clip_points_;//for clipping planes

		friend class VertexArrayManager;
	};

	class VertexArrayManager
	{
	public:
		VertexArrayManager();
		~VertexArrayManager();

		VertexArray* vertex_array(VAType type);

	private:
		std::vector<VertexArray*> va_list_;
		std::vector<VertexBuffer*> vb_list_;
	};

	inline bool VertexBuffer::bind()
	{
		if (valid_)
		{
			glBindBuffer(GL_ARRAY_BUFFER, id_);
			return true;
		}
		else
			return false;
	}

	inline void VertexBuffer::unbind()
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	inline bool VertexBuffer::valid()
	{
		return valid_;
	}

	inline void VertexBuffer::data(GLsizeiptr size, const GLvoid* data, GLenum usage)
	{
		if (valid_)
		{
			switch (type_)
			{
			case VABuf_Coord:
				glBindBuffer(GL_ARRAY_BUFFER, id_);
				glBufferData(GL_ARRAY_BUFFER,
					size, data, usage);
				break;
			case VABuf_index:
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER,
					size, data, usage);
				break;
			}
		}
	}

	inline void VertexArray::bind()
	{
		if (valid_)
			glBindVertexArray(id_);
	}

	inline void VertexArray::unbind()
	{
		glBindVertexArray(0);
	}

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

	inline bool VertexArray::attach_buffer(VertexBuffer* buf)
	{
		if (!valid_)
			return false;
		//doesn't do anything here, just remembers the relationship
		buffer_list_.push_back(buf);
		return true;
	}

	inline void VertexArray::attrib_pointer(GLuint index,
		GLint size, GLenum type, GLboolean normalized,
		GLsizei stride, const GLvoid* pointer)
	{
		//glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalized, stride, pointer);
		attrib_pointer_list_.push_back(index);
	}

	inline void VertexArray::draw_begin()
	{
		if (!valid_)
			return;
		glBindVertexArray(id_);
		//enable attrib array
		for (auto it = attrib_pointer_list_.begin();
			it != attrib_pointer_list_.end(); ++it)
			glEnableVertexAttribArray(*it);
	}

	inline void VertexArray::draw_end()
	{
		if (!valid_)
			return;
		//disable attrib array
		for (auto it = attrib_pointer_list_.begin();
			it != attrib_pointer_list_.end(); ++it)
			glDisableVertexAttribArray(*it);
		glBindVertexArray(0);
	}

	inline void VertexArray::draw()
	{
		if (!valid_)
			return;
		glBindVertexArray(id_);
		//enable attrib array
		for (auto it = attrib_pointer_list_.begin();
			it != attrib_pointer_list_.end(); ++it)
			glEnableVertexAttribArray(*it);
		switch (type_)
		{
		case VA_Norm_Square:
		case VA_Norm_Square_d:
			draw_norm_square();
			break;
		case VA_Brush_Circles:
			draw_circles();
			break;
		case VA_Bound_Cube:
			draw_bound_cube();
			break;
		case VA_Grid:
			draw_grid();
			break;
		case VA_Crop_Frame:
			draw_crop_frame();
			break;
		}
		//disable attrib array
		for (auto it = attrib_pointer_list_.begin();
			it != attrib_pointer_list_.end(); ++it)
			glDisableVertexAttribArray(*it);
		glBindVertexArray(0);
	}

	inline void VertexArray::draw_norm_square()
	{
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	inline void VertexArray::draw_circles()
	{
		int count = 0;
		//first, determine how many circles to draw
		auto param = param_list_.find(0);
		if (param != param_list_.end() &&
			param->second >= 0.0)
			count++;
		param = param_list_.find(1);
		if (param != param_list_.end() &&
			param->second >= 0.0)
			count++;
		//determine secs
		int secs = 0;
		param = param_list_.find(2);
		if (param != param_list_.end())
			secs = int(param->second + 0.5);

		//draw
		for (int i=0; i<count; ++i)
			glDrawArrays(GL_LINE_LOOP, i*secs, secs);
	}

	inline void VertexArray::draw_bound_cube()
	{
		glDrawArrays(GL_LINES, 0, 24);
	}

	inline void VertexArray::draw_clip_plane(int plane, bool border)
	{
		if (border)
			glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT,
				reinterpret_cast<const GLvoid*>(plane));
		else
			glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,
				reinterpret_cast<const GLvoid*>(plane));
	}

	inline void VertexArray::draw_grid()
	{
		//get line number
		int line_num = 0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			line_num = int(param->second+0.5) * 2 + 1;

		//draw
		glDrawArrays(GL_LINES, 0, line_num * 4);
	}

	inline void VertexArray::draw_cam_jack(int axis)
	{
		switch (axis)
		{
		case 0:
			glDrawArrays(GL_LINES, 0, 2);
			break;
		case 1:
			glDrawArrays(GL_LINES, 2, 2);
			break;
		case 2:
			glDrawArrays(GL_LINES, 4, 2);
			break;
		}
	}

	inline void VertexArray::draw_crop_frame()
	{
		glDrawArrays(GL_LINE_LOOP, 0, 4);
	}

	inline bool VertexArray::match(VAType type)
	{
		if (protected_)
			return false;
		if (type_ == type)
			return true;
		return false;
	}
}

#endif//VertexArray_h