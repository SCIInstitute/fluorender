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
//  

#include <GL/glew.h>
#include <VertexArray.h>
#include <Point.h>
#include <compatibility.h>

using namespace flvr;

namespace {
	GLenum ToGLAccess(BufferAccess access)
	{
		switch (access)
		{
		case BufferAccess::ReadOnly:  return GL_READ_ONLY;
		case BufferAccess::WriteOnly: return GL_WRITE_ONLY;
		case BufferAccess::ReadWrite: return GL_READ_WRITE;
		default:                      return GL_READ_ONLY;
		}
	}

	GLenum ToGLVertexAttribType(VertexAttribType type)
	{
		switch (type)
		{
		case VertexAttribType::Byte:          return GL_BYTE;
		case VertexAttribType::UnsignedByte:  return GL_UNSIGNED_BYTE;
		case VertexAttribType::Short:         return GL_SHORT;
		case VertexAttribType::UnsignedShort: return GL_UNSIGNED_SHORT;
		case VertexAttribType::Int:           return GL_INT;
		case VertexAttribType::UnsignedInt:   return GL_UNSIGNED_INT;
		case VertexAttribType::Float:         return GL_FLOAT;
		case VertexAttribType::Double:        return GL_DOUBLE;
		default:                              return GL_FLOAT;
		}
	}

	GLenum ToGLUsage(BufferUsage usage)
	{
		switch (usage)
		{
		case BufferUsage::StreamDraw:   return GL_STREAM_DRAW;
		case BufferUsage::StreamRead:   return GL_STREAM_READ;
		case BufferUsage::StreamCopy:   return GL_STREAM_COPY;
		case BufferUsage::StaticDraw:   return GL_STATIC_DRAW;
		case BufferUsage::StaticRead:   return GL_STATIC_READ;
		case BufferUsage::StaticCopy:   return GL_STATIC_COPY;
		case BufferUsage::DynamicDraw:  return GL_DYNAMIC_DRAW;
		case BufferUsage::DynamicRead:  return GL_DYNAMIC_READ;
		case BufferUsage::DynamicCopy:  return GL_DYNAMIC_COPY;
		default:                        return GL_STATIC_DRAW;
		}
	}

	GLenum ToGLPrimitiveType(PrimitiveType mode)
	{
		switch (mode)
		{
		case PrimitiveType::Points:         return GL_POINTS;
		case PrimitiveType::Lines:          return GL_LINES;
		case PrimitiveType::LineStrip:      return GL_LINE_STRIP;
		case PrimitiveType::Triangles:      return GL_TRIANGLES;
		case PrimitiveType::TriangleStrip:  return GL_TRIANGLE_STRIP;
		case PrimitiveType::TriangleFan:    return GL_TRIANGLE_FAN;
		default:                            return GL_TRIANGLES;
		}
	}

	GLenum ToGLIndexType(IndexType type)
	{
		switch (type)
		{
		case IndexType::UnsignedByte:   return GL_UNSIGNED_BYTE;
		case IndexType::UnsignedShort:  return GL_UNSIGNED_SHORT;
		case IndexType::UnsignedInt:    return GL_UNSIGNED_INT;
		default:                        return GL_UNSIGNED_INT;
		}
	}
}

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

bool VertexBuffer::bind()
{
	if (valid_)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		return true;
	}
	else
		return false;
}

void VertexBuffer::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

bool VertexBuffer::valid()
{
	return valid_;
}

void VertexBuffer::data(size_t size, const void* data, BufferUsage usage)
{
	if (valid_)
	{
		switch (type_)
		{
		case VABufferType::VABuf_Coord:
		case VABufferType::VABuf_Normal:
		case VABufferType::VABuf_Tex:
		case VABufferType::VABuf_Color:
			glBindBuffer(GL_ARRAY_BUFFER, id_);
			glBufferData(GL_ARRAY_BUFFER,
				size, data, ToGLUsage(usage));
			break;
		case VABufferType::VABuf_Index:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				size, data, ToGLUsage(usage));
			break;
		}
	}
}

void* VertexBuffer::map(BufferAccess access)
{
	if (!valid_)
		return nullptr;

	switch (type_)
	{
	case VABufferType::VABuf_Coord:
	case VABufferType::VABuf_Normal:
	case VABufferType::VABuf_Tex:
	case VABufferType::VABuf_Color:
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		return glMapBuffer(GL_ARRAY_BUFFER, ToGLAccess(access));
	case VABufferType::VABuf_Index:
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
		return glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, ToGLAccess(access));
	default:
		return nullptr;
	}
}

void VertexBuffer::unmap()
{
	if (!valid_)
		return;

	switch (type_)
	{
	case VABufferType::VABuf_Coord:
	case VABufferType::VABuf_Normal:
	case VABufferType::VABuf_Tex:
	case VABufferType::VABuf_Color:
		glBindBuffer(GL_ARRAY_BUFFER, id_);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		break;
	case VABufferType::VABuf_Index:
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_);
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
		break;
	}
}

VertexArray::VertexArray(VAType type) :
	id_(0),
	type_(type),
	valid_(false),
	protected_(false),
	dirty_(false),
	indexed_(false),
	interleaved_(true)
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
	buffer_list_.clear();
	glDeleteVertexArrays(1, &id_);
	id_ = 0;
	valid_ = false;
	protected_ = false;
	indexed_ = false;
}

void VertexArray::bind()
{
	if (valid_)
		glBindVertexArray(id_);
}

void VertexArray::unbind()
{
	glBindVertexArray(0);
}

void VertexArray::buffer_data(
	VABufferType type, size_t size, const void* data, BufferUsage usage)
{
	VertexBuffer* vb = 0;
	for (auto it = buffer_list_.begin();
		it != buffer_list_.end(); ++it)
	{
		if ((*it)->type_ == type)
		{
			vb = it->get();
			break;
		}
	}
	if (vb)
	{
		bind();
		vb->data(size, data, usage);
	}
	dirty_ = false;
}

void VertexArray::set_param(unsigned int index, double val)
{
	bool changed = false;
	//add to list
	auto param = param_list_.find(index);
	if (param == param_list_.end())
	{
		param_list_.insert(
			std::pair<unsigned int,
			double>(index, val));
		changed = true;
	}
	else
	{
		if (param->second != val)
		{
			param->second = val;
			changed = true;
		}
	}

	//change buffer data according to the parameter
	if (changed)
		update_buffer();
}

void VertexArray::set_param(
	std::vector<std::pair<unsigned int, double>>& params)
{
	bool changed = false;
	for (auto it = params.begin();
		it != params.end(); ++it)
	{
		auto param = param_list_.find(it->first);
		if (param == param_list_.end())
		{
			param_list_.insert(*it);
			changed = true;
		}
		else
		{
			if (param->second != it->second)
			{
				param->second = it->second;
				changed = true;
			}
		}
	}

	//change buffer data according to the parameter
	if (changed)
		update_buffer();
}

void VertexArray::set_param(fluo::BBox& box)
{
	if (bbox_ != box)
	{
		bbox_ = box;
		update_bound_cube();
	}
}

void VertexArray::set_param(std::vector<fluo::Point>& pp)
{
	bool update_vertex = false;
	bool update_index = false;
	if (float_list_.empty())
	{
		//first time
		float_list_.push_back(0); float_list_.push_back(0); float_list_.push_back(0);
		float_list_.push_back(0); float_list_.push_back(0); float_list_.push_back(1);
		float_list_.push_back(1); float_list_.push_back(0); float_list_.push_back(0);
		float_list_.push_back(1); float_list_.push_back(0); float_list_.push_back(1);
		float_list_.push_back(0); float_list_.push_back(1); float_list_.push_back(0);
		float_list_.push_back(0); float_list_.push_back(1); float_list_.push_back(1);
		float_list_.push_back(1); float_list_.push_back(1); float_list_.push_back(0);
		float_list_.push_back(1); float_list_.push_back(1); float_list_.push_back(1);
		update_vertex = true;
		update_index = true;
	}
	else if (pp.size() == 8 && float_list_.size() == 24)
	{
		for (size_t i = 0; i < 8; ++i)
		{
			if (pp[i] != fluo::Point(
				float_list_[i * 3],
				float_list_[i * 3 + 1],
				float_list_[i * 3 + 2]))
			{
				float_list_[i * 3] = static_cast<float>(pp[i].x());
				float_list_[i * 3 + 1] = static_cast<float>(pp[i].y());
				float_list_[i * 3 + 2] = static_cast<float>(pp[i].z());
				update_vertex = true;
			}
		}
	}

	if (update_vertex)
		update_clip_planes(update_index);
}

void VertexArray::set_param(std::vector<float>& vts)
{
	bool update_vertex = false;
	if (float_list_.empty())
	{
		for (auto i = vts.begin(); i != vts.end(); ++i)
			float_list_.push_back(*i);
		update_vertex = true;
	}
	else
	{
		auto i = float_list_.begin();
		auto j = vts.begin();
		while (i != float_list_.end() && j != vts.end())
		{
			if (*i != *j)
			{
				*i = *j;
				update_vertex = true;
			}
			++i; ++j;
		}
	}

	if (update_vertex)
		update_buffer();
}

void VertexArray::update_buffer()
{
	switch (type_)
	{
	case VAType::VA_Norm_Square:
	case VAType::VA_Left_Square:
	case VAType::VA_Right_Square:
	case VAType::VA_Text:
	default:
		break;
	case VAType::VA_Rectangle:
		update_buffer_rectangle();
		break;
	case VAType::VA_Norm_Square_d:
		update_buffer_norm_square_d();
		break;
	case VAType::VA_Brush_Circles:
		update_buffer_circles();
		break;
	case VAType::VA_Bound_Cube:
		update_bound_cube();
		break;
	case VAType::VA_Grid:
		update_grid();
		break;
	case VAType::VA_Cam_Jack:
		update_cam_jack();
		break;
	case VAType::VA_Cam_Center:
		update_cam_center();
		break;
	case VAType::VA_Crop_Frame:
		update_crop_frame();
		break;
	case VAType::VA_Scale_Bar:
		update_scale_bar();
		break;
	case VAType::VA_Legend_Squares:
		update_legend_squares();
		break;
	case VAType::VA_Grad_Bkg:
	case VAType::VA_Color_Map:
		update_grad_bkg();
		break;
	}
}

void VertexArray::update_buffer_rectangle()
{
	float tex_aspect = 1.0f;
	float view_aspect = 1.0f;
	float scale_x = 1.0f;
	float scale_y = 1.0f;
	float u0 = 0.0f, u1 = 1.0f, v0 = 0.0f, v1 = 1.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		tex_aspect = static_cast<float>(param->second);
	param = param_list_.find(1);
	if (param != param_list_.end())
		view_aspect = static_cast<float>(param->second);
	param = param_list_.find(2);
	if (param != param_list_.end())
		u0 = static_cast<float>(param->second);
	param = param_list_.find(3);
	if (param != param_list_.end())
		u1 = static_cast<float>(param->second);
	param = param_list_.find(4);
	if (param != param_list_.end())
		v0 = static_cast<float>(param->second);
	param = param_list_.find(5);
	if (param != param_list_.end())
		v1 = static_cast<float>(param->second);
	if (tex_aspect > view_aspect)
	{
		scale_y = view_aspect / tex_aspect;
	}
	else
	{
		scale_x = tex_aspect / view_aspect;
	}
	float points[] = {
		-scale_x, -scale_y, 0.0f,  u0, v0, 0.0f,
		 scale_x, -scale_y, 0.0f,  u1, v0, 0.0f,
		-scale_x,  scale_y, 0.0f,  u0, v1, 0.0f,
		 scale_x,  scale_y, 0.0f,  u1, v1, 0.0f
	};
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * 24, points, BufferUsage::StreamDraw);
}

void VertexArray::update_buffer_norm_square_d()
{
	float d = 0.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		d = static_cast<float>(param->second);

	float points[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, d,
		1.0f, -1.0f, 0.0f, 1.0f, 0.0f, d,
		-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, d,
		1.0f, 1.0f, 0.0f, 1.0f, 1.0f, d };
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * 24, points, BufferUsage::StreamDraw);
}

void VertexArray::update_buffer_circles()
{
	//get parameters
	float r1 = -1.0f;
	float r2 = -1.0f;
	int secs = 0;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		r1 = static_cast<float>(param->second);
	param = param_list_.find(1);
	if (param != param_list_.end())
		r2 = static_cast<float>(param->second);
	param = param_list_.find(2);
	if (param != param_list_.end())
		secs = static_cast<int>(std::round(param->second));

	if (r1 < 0.0 && r2 < 0.0)
		return;

	//generate vertex data for circles
	std::vector<float> vertex;
	if (r1 < 0.0 || r2 < 0.0)
		vertex.reserve(secs * 3);//just one circle
	else
		vertex.reserve(secs * 6);//two circles

	float deg = 0.0f;
	//first circle
	if (r1 >= 0.0)
	{
		for (size_t i = 0; i < secs; ++i)
		{
			deg = static_cast<float>(i * 2 * PI / secs);
			vertex.push_back(r1 * sin(deg));
			vertex.push_back(r1 * cos(deg));
			vertex.push_back(0.0f);
		}
	}
	if (r2 >= 0.0)
	{
		//second circle
		for (size_t i = 0; i < secs; ++i)
		{
			deg = static_cast<float>(i * 2 * PI / secs);
			vertex.push_back(r2 * sin(deg));
			vertex.push_back(r2 * cos(deg));
			vertex.push_back(0.0f);
		}
	}
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StreamDraw);
}

void VertexArray::update_bound_cube()
{
	//draw 12 sides
	std::vector<float> vertex;
	vertex.reserve(24 * 3);

	float x1 = static_cast<float>(bbox_.Min().x());
	float y1 = static_cast<float>(bbox_.Min().y());
	float z1 = static_cast<float>(bbox_.Min().z());
	float x2 = static_cast<float>(bbox_.Max().x());
	float y2 = static_cast<float>(bbox_.Max().y());
	float z2 = static_cast<float>(bbox_.Max().z());

	//s1-000-100
	vertex.push_back(x1); vertex.push_back(y1); vertex.push_back(z1);
	vertex.push_back(x2); vertex.push_back(y1); vertex.push_back(z1);
	//s2-100-101
	vertex.push_back(x2); vertex.push_back(y1); vertex.push_back(z1);
	vertex.push_back(x2); vertex.push_back(y1); vertex.push_back(z2);
	//s3-101-001
	vertex.push_back(x2); vertex.push_back(y1); vertex.push_back(z2);
	vertex.push_back(x1); vertex.push_back(y1); vertex.push_back(z2);
	//s4-001-000
	vertex.push_back(x1); vertex.push_back(y1); vertex.push_back(z2);
	vertex.push_back(x1); vertex.push_back(y1); vertex.push_back(z1);
	//s5-000-010
	vertex.push_back(x1); vertex.push_back(y1); vertex.push_back(z1);
	vertex.push_back(x1); vertex.push_back(y2); vertex.push_back(z1);
	//s6-100-110
	vertex.push_back(x2); vertex.push_back(y1); vertex.push_back(z1);
	vertex.push_back(x2); vertex.push_back(y2); vertex.push_back(z1);
	//s7-101-111
	vertex.push_back(x2); vertex.push_back(y1); vertex.push_back(z2);
	vertex.push_back(x2); vertex.push_back(y2); vertex.push_back(z2);
	//s8-001-011
	vertex.push_back(x1); vertex.push_back(y1); vertex.push_back(z2);
	vertex.push_back(x1); vertex.push_back(y2); vertex.push_back(z2);
	//s9-010-110
	vertex.push_back(x1); vertex.push_back(y2); vertex.push_back(z1);
	vertex.push_back(x2); vertex.push_back(y2); vertex.push_back(z1);
	//s10-110-111
	vertex.push_back(x2); vertex.push_back(y2); vertex.push_back(z1);
	vertex.push_back(x2); vertex.push_back(y2); vertex.push_back(z2);
	//s11-111-011
	vertex.push_back(x2); vertex.push_back(y2); vertex.push_back(z2);
	vertex.push_back(x1); vertex.push_back(y2); vertex.push_back(z2);
	//s12-011-010
	vertex.push_back(x1); vertex.push_back(y2); vertex.push_back(z2);
	vertex.push_back(x1); vertex.push_back(y2); vertex.push_back(z1);

	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StreamDraw);
}

void VertexArray::update_clip_planes(bool update_index)
{
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * float_list_.size(),
		&float_list_[0], BufferUsage::StreamDraw);
	if (update_index)
	{
		std::vector<uint32_t> index;
		index.reserve(6 * 4);

		// -X face
		index.insert(index.end(), { 4,0,1,5 });
		// +X face
		index.insert(index.end(), { 7,3,2,6 });
		// -Y face
		index.insert(index.end(), { 1,0,2,3 });
		// +Y face
		index.insert(index.end(), { 4,5,7,6 });
		// -Z face
		index.insert(index.end(), { 0,4,2,6 });
		// +Z face
		index.insert(index.end(), { 5,1,3,7 });

		buffer_data(VABufferType::VABuf_Index,
			sizeof(uint32_t) * index.size(),
			&index[0], BufferUsage::StaticDraw);
	}
}

void VertexArray::update_grid()
{
	//get parameters
	int grid_num = 0;
	float distance = 1.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		grid_num = int(param->second + 0.5);
	param = param_list_.find(1);
	if (param != param_list_.end())
		distance = static_cast<float>(param->second);
	int line_num = grid_num * 2 + 1;
	std::vector<float> vertex;
	vertex.reserve(line_num * 4 * 3);

	float gap = distance / grid_num;
	int i;
	for (i = 0; i < line_num; ++i)
	{
		vertex.push_back(float(-distance + gap * i));
		vertex.push_back(float(0.0));
		vertex.push_back(float(-distance));
		vertex.push_back(float(-distance + gap * i));
		vertex.push_back(float(0.0));
		vertex.push_back(float(distance));
	}
	for (i = 0; i < line_num; ++i)
	{
		vertex.push_back(float(-distance));
		vertex.push_back(float(0.0));
		vertex.push_back(float(-distance + gap * i));
		vertex.push_back(float(distance));
		vertex.push_back(float(0.0));
		vertex.push_back(float(-distance + gap * i));
	}
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StreamDraw);
}

void VertexArray::update_cam_jack()
{
	float len = 1.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		len = static_cast<float>(param->second);

	std::vector<float> vertex;
	vertex.reserve(6 * 3);

	vertex.push_back(0.0f); vertex.push_back(0.0f); vertex.push_back(0.0f);
	vertex.push_back(len);  vertex.push_back(0.0f); vertex.push_back(0.0f);
	vertex.push_back(0.0f); vertex.push_back(0.0f); vertex.push_back(0.0f);
	vertex.push_back(0.0f); vertex.push_back(len);  vertex.push_back(0.0f);
	vertex.push_back(0.0f); vertex.push_back(0.0f); vertex.push_back(0.0f);
	vertex.push_back(0.0f); vertex.push_back(0.0f); vertex.push_back(len);
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StaticDraw);
}

void VertexArray::update_cam_center()
{
	float len = 10.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		len = static_cast<float>(param->second);
	float gap = 5.0f;

	std::vector<float> vertex;
	vertex.reserve(24);

	vertex.push_back(-len - gap); vertex.push_back(0); vertex.push_back(0);
	vertex.push_back(-gap); vertex.push_back(0); vertex.push_back(0);
	vertex.push_back(len + gap); vertex.push_back(0); vertex.push_back(0);
	vertex.push_back(gap); vertex.push_back(0); vertex.push_back(0);
	vertex.push_back(0); vertex.push_back(-len - gap); vertex.push_back(0);
	vertex.push_back(0); vertex.push_back(-gap); vertex.push_back(0);
	vertex.push_back(0); vertex.push_back(len + gap); vertex.push_back(0);
	vertex.push_back(0); vertex.push_back(gap); vertex.push_back(0);
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StaticDraw);
}

void VertexArray::update_crop_frame()
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 1.0f;
	float h = 1.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		x = static_cast<float>(param->second);
	param = param_list_.find(1);
	if (param != param_list_.end())
		y = static_cast<float>(param->second);
	param = param_list_.find(2);
	if (param != param_list_.end())
		w = static_cast<float>(param->second);
	param = param_list_.find(3);
	if (param != param_list_.end())
		h = static_cast<float>(param->second);

	std::vector<float> vertex;
	vertex.reserve(4 * 3);

	vertex.push_back(x - 1); vertex.push_back(y - 1); vertex.push_back(0.0);
	vertex.push_back(x + w + 1); vertex.push_back(y - 1); vertex.push_back(0.0);
	vertex.push_back(x + w + 1); vertex.push_back(y + h + 1); vertex.push_back(0.0);
	vertex.push_back(x - 1); vertex.push_back(y + h + 1); vertex.push_back(0.0);
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StreamDraw);
}

void VertexArray::update_scale_bar()
{
	float x = 0.0f;
	float y = 0.0f;
	float w = 1.0f;
	float h = 1.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		x = static_cast<float>(param->second);
	param = param_list_.find(1);
	if (param != param_list_.end())
		y = static_cast<float>(param->second);
	param = param_list_.find(2);
	if (param != param_list_.end())
		w = static_cast<float>(param->second);
	param = param_list_.find(3);
	if (param != param_list_.end())
		h = static_cast<float>(param->second);

	std::vector<float> vertex;
	vertex.reserve(4 * 3);

	vertex.push_back(x); vertex.push_back(y); vertex.push_back(0.0);
	vertex.push_back(x - w); vertex.push_back(y); vertex.push_back(0.0);
	vertex.push_back(x); vertex.push_back(y - h); vertex.push_back(0.0);
	vertex.push_back(x - w); vertex.push_back(y - h); vertex.push_back(0.0);
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StreamDraw);
}

void VertexArray::update_legend_squares()
{
	float px1 = 0.0f;
	float py1 = 0.0f;
	float px2 = 1.0f;
	float py2 = 1.0f;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		px1 = static_cast<float>(param->second);
	param = param_list_.find(1);
	if (param != param_list_.end())
		py1 = static_cast<float>(param->second);
	param = param_list_.find(2);
	if (param != param_list_.end())
		px2 = static_cast<float>(param->second);
	param = param_list_.find(3);
	if (param != param_list_.end())
		py2 = static_cast<float>(param->second);

	std::vector<float> vertex;
	vertex.reserve(8 * 3);

	vertex.push_back(px1 - 1.0f); vertex.push_back(py2 + 1.0f); vertex.push_back(0.0f);
	vertex.push_back(px2 + 1.0f); vertex.push_back(py2 + 1.0f); vertex.push_back(0.0f);
	vertex.push_back(px1 - 1.0f); vertex.push_back(py1 - 1.0f); vertex.push_back(0.0f);
	vertex.push_back(px2 + 1.0f); vertex.push_back(py1 - 1.0f); vertex.push_back(0.0f);
	vertex.push_back(px1); vertex.push_back(py2); vertex.push_back(0.0f);
	vertex.push_back(px2); vertex.push_back(py2); vertex.push_back(0.0f);
	vertex.push_back(px1); vertex.push_back(py1); vertex.push_back(0.0f);
	vertex.push_back(px2); vertex.push_back(py1); vertex.push_back(0.0f);
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * vertex.size(),
		&vertex[0], BufferUsage::StreamDraw);
}

void VertexArray::update_grad_bkg()
{
	buffer_data(VABufferType::VABuf_Coord,
		sizeof(float) * float_list_.size(),
		&float_list_[0], BufferUsage::StreamDraw);
}

bool VertexArray::attach_buffer(const std::shared_ptr<VertexBuffer>& buf)
{
	if (!valid_)
		return false;
	//doesn't do anything here, just remembers the relationship
	buffer_list_.push_back(buf);
	return true;
}

void VertexArray::attrib_pointer(unsigned int index,
	int size, VertexAttribType type, bool normalized,
	size_t stride, const void* pointer)
{
	glVertexAttribPointer(index, size, ToGLVertexAttribType(type), normalized, stride, pointer);
	attrib_pointer_list_.insert(static_cast<VAAttribIndex>(index));
}

void VertexArray::remove_attrib_pointer(GLuint index)
{
	attrib_pointer_list_.erase(static_cast<VAAttribIndex>(index));
}

void* VertexArray::map_buffer(VABufferType type, BufferAccess access)
{
	VertexBuffer* vb = nullptr;
	for (auto it = buffer_list_.begin(); it != buffer_list_.end(); ++it)
	{
		if ((*it)->type_ == type)
		{
			vb = it->get();
			break;
		}
	}
	return vb ? vb->map(access) : nullptr;
}

void VertexArray::unmap_buffer(VABufferType type)
{
	VertexBuffer* vb = nullptr;
	for (auto it = buffer_list_.begin(); it != buffer_list_.end(); ++it)
	{
		if ((*it)->type_ == type)
		{
			vb = it->get();
			break;
		}
	}
	if (vb)
		vb->unmap();
}

bool VertexArray::get_dirty()
{
	return dirty_;
}

void VertexArray::draw_begin()
{
	if (!valid_)
		return;
	glBindVertexArray(id_);
	//enable attrib array
	for (auto it = attrib_pointer_list_.begin();
		it != attrib_pointer_list_.end(); ++it)
		glEnableVertexAttribArray(static_cast<unsigned int>(*it));
}

void VertexArray::draw_arrays(PrimitiveType mode, int first, size_t count)
{
	if (!valid_)
		return;
	glDrawArrays(ToGLPrimitiveType(mode), first, count);
}

void VertexArray::draw_elements(PrimitiveType mode, size_t count, IndexType type, const void* indices)
{
	if (!valid_)
		return;
	glDrawElements(ToGLPrimitiveType(mode), count, ToGLIndexType(type), indices);
}

void VertexArray::draw_end()
{
	if (!valid_)
		return;
	//disable attrib array
	for (auto it = attrib_pointer_list_.begin();
		it != attrib_pointer_list_.end(); ++it)
		glDisableVertexAttribArray(static_cast<unsigned int>(*it));
	glBindVertexArray(0);
}

void VertexArray::draw()
{
	draw_begin();

	switch (type_)
	{
	case VAType::VA_Unmanaged:
		//don't use it to draw unmanaged
		break;
	case VAType::VA_Norm_Square:
	case VAType::VA_Rectangle:
	case VAType::VA_Norm_Square_d:
	case VAType::VA_Text:
	case VAType::VA_Left_Square:
	case VAType::VA_Right_Square:
		draw_norm_square();
		break;
	case VAType::VA_Brush_Circles:
		draw_circles();
		break;
	case VAType::VA_Bound_Cube:
		draw_bound_cube();
		break;
	case VAType::VA_Grid:
		draw_grid();
		break;
	case VAType::VA_Crop_Frame:
		draw_crop_frame();
		break;
	case VAType::VA_Scale_Bar:
		draw_scale_bar();
		break;
	case VAType::VA_Grad_Bkg:
		draw_grad_bkg();
		break;
	case VAType::VA_Color_Map:
		draw_color_map();
		break;
	case VAType::VA_Traces:
		draw_traces();
		break;
	case VAType::VA_Rulers:
		draw_rulers();
		break;
	}

	draw_end();
}

void VertexArray::draw_unmanaged(int pos, int tri_num)
{
	if (indexed_)
	{
		draw_elements(PrimitiveType::Triangles, tri_num * 3, IndexType::UnsignedInt, (void*)(pos * sizeof(GLuint)));
	}
	else
	{
		draw_arrays(PrimitiveType::Triangles, pos, (size_t)(tri_num * 3));
	}
}

void VertexArray::draw_norm_square()
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void VertexArray::draw_circles()
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
	for (int i = 0; i < count; ++i)
		glDrawArrays(GL_LINE_LOOP, i * secs, secs);
}

void VertexArray::draw_bound_cube()
{
	glDrawArrays(GL_LINES, 0, 24);
}

void VertexArray::draw_clip_plane(int plane, bool border)
{
	if (border)
		glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_INT,
			reinterpret_cast<const GLvoid*>(plane * 4 * sizeof(uint32_t)));
	else
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT,
			reinterpret_cast<const GLvoid*>(plane * 4 * sizeof(uint32_t)));
}

void VertexArray::draw_grid()
{
	//get line number
	int line_num = 0;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		line_num = int(param->second + 0.5) * 2 + 1;

	//draw
	glDrawArrays(GL_LINES, 0, line_num * 4);
}

void VertexArray::draw_cam_jack(int axis)
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

void VertexArray::draw_cam_center()
{
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);
	glDrawArrays(GL_LINES, 6, 2);
}

void VertexArray::draw_crop_frame()
{
	glDrawArrays(GL_LINE_LOOP, 0, 4);
}

void VertexArray::draw_scale_bar()
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void VertexArray::draw_legend_square(int index)
{
	if (index == 0)
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	else if (index == 1)
		glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);
}

void VertexArray::draw_grad_bkg()
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 8);
}

void VertexArray::draw_color_map()
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
}

void VertexArray::draw_traces()
{
	//get array number
	int num = 0;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		num = int(param->second + 0.5);

	//draw
	glDrawArrays(GL_LINES, 0, num);
}

void VertexArray::draw_rulers()
{
	//get array number
	int num = 0;
	auto param = param_list_.find(0);
	if (param != param_list_.end())
		num = int(param->second + 0.5);

	//draw
	glDrawArrays(GL_LINES, 0, num);
}

VertexArrayManager::VertexArrayManager()
{
}

VertexArrayManager::~VertexArrayManager()
{
	va_list_.clear();
}

void VertexArrayManager::clear()
{
	va_list_.clear();
}

std::shared_ptr<VertexArray> VertexArrayManager::vertex_array(VAType type)
{
	//find one that matches and return
	for (auto it = va_list_.begin();
		it != va_list_.end(); ++it)
	{
		if ((*it)->match(type))
			return *it;
	}

	//create new vertex array
	auto va = std::make_shared<VertexArray>(type);
	va->create();
	//add to list
	va_list_.push_back(va);
	va->bind();
	//create vertex buffer
	auto vb = std::make_shared<VertexBuffer>(VABufferType::VABuf_Coord);
	vb->create();
	//attach buffer
	va->attach_buffer(vb);
	if (type == VAType::VA_Norm_Square)
	{
		//assign data
		float points[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
		vb->data(sizeof(float) * 24, points, BufferUsage::StaticDraw);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Rectangle)
	{
		//set param
		va->set_param(0, 1.0);
		va->set_param(1, 1.0);
		va->set_param(2, 0.0);
		va->set_param(3, 1.0);
		va->set_param(4, 0.0);
		va->set_param(5, 1.0);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Norm_Square_d)
	{
		//set param
		va->set_param(0, 0.0);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Left_Square)
	{
		//assign data
		float points[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
		vb->data(sizeof(float) * 24, points, BufferUsage::StaticDraw);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Right_Square)
	{
		//assign data
		float points[] = {
			0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
		vb->data(sizeof(float) * 24, points, BufferUsage::StaticDraw);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Brush_Circles)
	{
		//set param
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, 10.0));
		params.push_back(std::pair<unsigned int, double>(1, 10.0));
		params.push_back(std::pair<unsigned int, double>(2, 60.0));
		va->set_param(params);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Bound_Cube)
	{
		//set param
		fluo::BBox bbox(fluo::Point(0.0), fluo::Point(1.0));
		va->set_param(bbox);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Clip_Planes)
	{
		//index buffer
		vb = std::make_shared<VertexBuffer>(VABufferType::VABuf_Index);
		vb->create();
		//attach buffer
		va->attach_buffer(vb);
		va->indexed_ = true;
		//set param
		std::vector<fluo::Point> pp;
		va->set_param(pp);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Grid)
	{
		//set param
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, 5.0));
		params.push_back(std::pair<unsigned int, double>(1, 10.0));
		va->set_param(params);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Cam_Jack)
	{
		//set param
		va->set_param(0, 1.0);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Cam_Center)
	{
		//set param
		va->set_param(0, 1.0);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Crop_Frame ||
		type == VAType::VA_Scale_Bar ||
		type == VAType::VA_Legend_Squares)
	{
		//set param
		std::vector<std::pair<unsigned int, double>> params;
		params.push_back(std::pair<unsigned int, double>(0, 0.0));
		params.push_back(std::pair<unsigned int, double>(1, 0.0));
		params.push_back(std::pair<unsigned int, double>(2, 1.0));
		params.push_back(std::pair<unsigned int, double>(3, 1.0));
		va->set_param(params);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 3 * sizeof(float), (const void*)0);
	}
	else if (type == VAType::VA_Grad_Bkg)
	{
		//set param
		std::vector<float> list(48, 0.0);
		va->set_param(list);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Color_Map)
	{
		//set param
		std::vector<float> list(98, 0.0);
		va->set_param(list);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 7 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 4, VertexAttribType::Float, false, 7 * sizeof(float), (const void*)12);
	}
	else if (type == VAType::VA_Traces ||
		type == VAType::VA_Rulers)
	{
		//set param
		std::vector<float> vertex(12, 0.0);
		va->buffer_data(VABufferType::VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], BufferUsage::StreamDraw);
		//set attrib
		va->attrib_pointer(0, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)0);
		va->attrib_pointer(1, 3, VertexAttribType::Float, false, 6 * sizeof(float), (const void*)12);
		va->dirty_ = true;
	}
	if (type == VAType::VA_Text)
	{
		//assign data
		float points[] = {
			0.0f, 0.0f, 0.0f, 0.0f,
			1.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 1.0f, 1.0f };
		vb->data(sizeof(float) * 16, points, BufferUsage::StaticDraw);
		//set attrib
		va->attrib_pointer(0, 4, VertexAttribType::Float, false, 0, (const void*)0);
	}
	//unbind
	va->unbind();

	return va;
}

std::shared_ptr<VertexArray> VertexArrayManager::vertex_array(bool vbuf, bool ibuf)
{
	//create new vertex array
	auto va = std::make_shared<VertexArray>(VAType::VA_Unmanaged);
	va->create();
	va->bind();
	va_list_.push_back(va);

	if (vbuf)
	{
		//create vertex buffer
		auto vb = std::make_shared<VertexBuffer>(VABufferType::VABuf_Coord);
		vb->create();
		//attach buffer
		va->attach_buffer(vb);
	}
	if (ibuf)
	{
		//create vertex buffer
		auto vb = std::make_shared<VertexBuffer>(VABufferType::VABuf_Index);
		vb->create();
		//attach buffer
		va->attach_buffer(vb);
		va->indexed_ = true;
	}
	//unbind
	//va->unbind();

	return va;
}

void VertexArray::protect()
{
	protected_ = true;
}

void VertexArray::unprotect()
{
	protected_ = false;
}

bool VertexArray::valid()
{
	return valid_;
}

unsigned int VertexArray::id_array()
{
	return id_;
}

unsigned int VertexArray::id_buffer(VABufferType type)
{
	VertexBuffer* vb = nullptr;
	for (auto it = buffer_list_.begin(); it != buffer_list_.end(); ++it)
	{
		if ((*it)->type_ == type)
		{
			vb = it->get();
			break;
		}
	}
	if (vb)
		return vb->id_;
	return 0;
}

void VertexArray::add_index_buffer()
{
	for (auto& it : buffer_list_)
	{
		//return if index buffer already exists
		if (it->type_ == VABufferType::VABuf_Index)
			return;
	}
	//create index buffer
	auto vb = std::make_shared<VertexBuffer>(VABufferType::VABuf_Index);
	vb->create();
	attach_buffer(vb);
	indexed_ = true;
}

void VertexArray::delete_index_buffer()
{
	indexed_ = false;

	// Destroy matching buffers first
	for (auto& it : buffer_list_)
	{
		if (it->type_ == VABufferType::VABuf_Index)
		{
			bind();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			unbind();
		}
	}

	// Remove them from the vector
	buffer_list_.erase(
		std::remove_if(buffer_list_.begin(), buffer_list_.end(),
			[](const std::shared_ptr<VertexBuffer>& buf) {
				return buf->type_ == VABufferType::VABuf_Index;
			}),
		buffer_list_.end()
	);
}

void VertexArray::add_buffer(VABufferType type)
{
	for (auto& it : buffer_list_)
	{
		//return if buffer already exists
		if (it->type_ == type)
			return;
	}
	//create buffer
	auto vb = std::make_shared<VertexBuffer>(type);
	vb->create();
	attach_buffer(vb);
	interleaved_ = false;
}

void VertexArray::delete_buffer(VABufferType type)
{
	// Remove them from the vector
	buffer_list_.erase(
		std::remove_if(buffer_list_.begin(), buffer_list_.end(),
			[&type](const std::shared_ptr<VertexBuffer>& buf) {
				return buf->type_ == type;
			}),
		buffer_list_.end()
	);

	bool found = false;
	for (auto& it : buffer_list_)
	{
		if (it->type_ == VABufferType::VABuf_Normal ||
			it->type_ == VABufferType::VABuf_Tex ||
			it->type_ == VABufferType::VABuf_Color)
		{
			found = true;
			break;
		}
	}
	if (!found)
		interleaved_ = true;
}

bool VertexArray::match(VAType type)
{
	if (protected_)
		return false;
	if (type_ == type)
		return true;
	return false;
}

void VertexArrayManager::set_dirty(VAType type)
{
	for (auto it = va_list_.begin();
		it != va_list_.end(); ++it)
	{
		if ((*it)->type_ == type)
			(*it)->dirty_ = true;
	}
}

void VertexArrayManager::set_all_dirty()
{
	for (auto it = va_list_.begin();
		it != va_list_.end(); ++it)
		(*it)->dirty_ = true;
}
