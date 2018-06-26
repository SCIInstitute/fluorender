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

		void set_param(double);
		inline void draw();

		inline bool match(VAType);

	private:
		unsigned int id_;
		VAType type_;
		bool valid_;
		bool protected_;
		std::vector<VertexBuffer*> buffer_list_;

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
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	}

	inline void VertexArray::draw()
	{
		if (!valid_)
			return;
		glBindVertexArray(id_);
		switch (type_)
		{
		case VA_Norm_Square:
		case VA_Norm_Square_d:
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			break;
		}
		glBindVertexArray(0);
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