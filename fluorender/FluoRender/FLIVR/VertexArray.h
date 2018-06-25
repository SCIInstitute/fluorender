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
		VABuf_Norm_Square = 0,
	};
	class VertexArray;
	class VertexArrayManager;
	class VertexBuffer
	{
	public:
		VertexBuffer();
		~VertexBuffer();

		bool create();
		void destroy();
		inline bool bind();
		inline void unbind();
		inline bool valid();
		inline void data(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);

	private:
		unsigned int id_;
		VABufferType type_;
		bool valid_;

		friend class VertexArray;
	};

	enum VAType
	{
		VA_Norm_Square = 0,
	};
	class VertexArray
	{
	public:
		VertexArray();
		~VertexArray();

		bool create();
		void destroy();
		inline void bind();
		inline void unbind();
		inline void protect();
		inline void unprotect();
		inline bool valid();
		inline unsigned int id();

		inline void attrib_pointer(GLuint index,
			GLint size, GLenum type, GLboolean normalized,
			GLsizei stride, const GLvoid* pointer);
		inline void draw(GLenum mode,
			GLint first, GLsizei count);

		inline bool match();

	private:
		unsigned int id_;
		VAType type_;
		bool valid_;
		bool protected_;
		std::vector<std::pair<unsigned int, VertexBuffer*>> buffer_list_;
		std::vector<GLuint> attrib_ptr_list_;

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

	inline void VertexBuffer::data(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
	{
		if (valid_)
		{
			glBindBuffer(target, id_);
			glBufferData(target,
				size, data, usage);
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

	inline void VertexArray::attrib_pointer(GLuint index,
		GLint size, GLenum type, GLboolean normalized,
		GLsizei stride, const GLvoid* pointer)
	{
		glEnableVertexAttribArray(index);
		glVertexAttribPointer(index, size, type, normalized, stride, pointer);
	}

	inline void VertexArray::draw(GLenum mode,
		GLint first, GLsizei count)
	{

	}
}

#endif//VertexArray_h