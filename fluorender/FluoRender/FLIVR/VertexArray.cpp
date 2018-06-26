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
//  

#include "VertexArray.h"

namespace FLIVR
{
	VertexBuffer::VertexBuffer(VABufferType type) :
		id_(0), type_(type), valid_(false)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
		destroy();
	}

	void VertexBuffer::create()
	{
		glGenBuffers(1, &id_);
		valid_ = true;
	}

	void VertexBuffer::destroy()
	{
		glDeleteBuffers(1, &id_);
		id_ = 0;
		valid_ = false;
	}

	VertexArray::VertexArray(VAType type) :
		id_(0), type_(type), valid_(false), protected_(false)
	{
	}

	VertexArray::~VertexArray()
	{
		destroy();
	}

	void VertexArray::create()
	{
		glGenVertexArrays(1, &id_);
		valid_ = true;
	}

	void VertexArray::destroy()
	{
		glDeleteVertexArrays(1, &id_);
		id_ = 0;
		valid_ = false;
		protected_ = false;
	}

	void VertexArray::buffer_data(
		VABufferType type, GLsizeiptr size,
		const GLvoid* data, GLenum usage)
	{
		VertexBuffer* vb = 0;
		for (auto it = buffer_list_.begin();
			it != buffer_list_.end(); ++it)
		{
			if ((*it)->type_ == type)
			{
				vb = *it;
				break;
			}
		}
		if (vb)
			vb->data(size, data, usage);
	}

	void VertexArray::set_param(double val)
	{
		if (type_ == VA_Norm_Square_d)
		{
			float points[] = {
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, float(val),
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f, float(val),
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, float(val),
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f, float(val) };
			buffer_data(VABuf_Coord,
				sizeof(float) * 24, points, GL_STREAM_DRAW);
		}
	}

	VertexArrayManager::VertexArrayManager()
	{
	}

	VertexArrayManager::~VertexArrayManager()
	{
		//release all opengl resources managed by the manager
		for (auto it = va_list_.begin();
			it != va_list_.end(); ++it)
			delete *it;
		for (auto it = vb_list_.begin();
			it != vb_list_.end(); ++it)
			delete *it;
	}

	VertexArray* VertexArrayManager::vertex_array(VAType type)
	{
		//find one that matches and return
		for (auto it = va_list_.begin();
			it != va_list_.end(); ++it)
		{
			if ((*it)->match(type))
				return *it;
		}

		//create new vertex array
		VertexArray* va = new VertexArray(type);
		va->create();
		//add to list
		va_list_.push_back(va);
		va->bind();
		if (type == VA_Norm_Square ||
			type == VA_Norm_Square_d)
		{
			//create vertex buffer
			VertexBuffer* vb = new VertexBuffer(VABuf_Coord);
			vb->create();
			//assign data
			float points[] = {
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
			vb->data(sizeof(float) * 24, points, GL_STATIC_DRAW);
			//attach buffer
			va->attach_buffer(vb);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
			va->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);
			vb_list_.push_back(vb);
		}
		//unbind
		va->unbind();

		return va;
	}
}