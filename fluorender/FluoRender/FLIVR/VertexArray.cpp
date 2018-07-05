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
#include "utility.h"

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
		id_(0),
		type_(type),
		valid_(false),
		protected_(false),
		dirty_(false)
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
		if (type_ == VA_Unmanaged)
		{
			for (auto it = buffer_list_.begin();
				it != buffer_list_.end(); ++it)
				delete *it;
		}
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
		std::vector<std::pair<unsigned int, double>> &params)
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

	void VertexArray::set_param(BBox &box)
	{
		if (bbox_ != box)
		{
			bbox_ = box;
			update_bound_cube();
		}
	}

	void VertexArray::set_param(std::vector<Point> &pp)
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
				if (pp[i] !=
					Point(float_list_[i*3],
						float_list_[i*3+1],
						float_list_[i*3+2]))
				{
					float_list_[i * 3] = pp[i].x();
					float_list_[i * 3+1] = pp[i].y();
					float_list_[i * 3+2] = pp[i].z();
					update_vertex = true;
				}
			}
		}

		if (update_vertex)
			update_clip_planes(update_index);
	}

	void VertexArray::set_param(std::vector<float> &vts)
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
		case VA_Norm_Square:
		case VA_Text:
		default:
			break;
		case VA_Norm_Square_d:
			update_buffer_norm_square_d();
			break;
		case VA_Brush_Circles:
			update_buffer_circles();
			break;
		case VA_Bound_Cube:
			update_bound_cube();
			break;
		case VA_Grid:
			update_grid();
			break;
		case VA_Cam_Jack:
			update_cam_jack();
			break;
		case VA_Crop_Frame:
			update_crop_frame();
			break;
		case VA_Scale_Bar:
			update_scale_bar();
			break;
		case VA_Legend_Squares:
			update_legend_squares();
			break;
		case VA_Grad_Bkg:
		case VA_Color_Map:
			update_grad_bkg();
			break;
		}
	}

	void VertexArray::update_buffer_norm_square_d()
	{
		double d = 0.0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			d = param->second;

		float points[] = {
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, float(d),
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, float(d),
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, float(d),
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, float(d) };
		buffer_data(VABuf_Coord,
			sizeof(float) * 24, points, GL_STREAM_DRAW);
	}

	void VertexArray::update_buffer_circles()
	{
		//get parameters
		double r1 = -1.0;
		double r2 = -1.0;
		int secs = 0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			r1 = param->second;
		param = param_list_.find(1);
		if (param != param_list_.end())
			r2 = param->second;
		param = param_list_.find(2);
		if (param != param_list_.end())
			secs = int(param->second + 0.5);

		if (r1 < 0.0 && r2 < 0.0)
			return;

		//generate vertex data for circles
		std::vector<float> vertex;
		if (r1 < 0.0 || r2 < 0.0)
			vertex.reserve(secs * 3);//just one circle
		else
			vertex.reserve(secs * 6);//two circles

		double deg = 0.0;
		//first circle
		if (r1 >= 0.0)
		{
			for (size_t i = 0; i<secs; ++i)
			{
				deg = i * 2 * PI / secs;
				vertex.push_back(r1*sin(deg));
				vertex.push_back(r1*cos(deg));
				vertex.push_back(0.0f);
			}
		}
		if (r2 >= 0.0)
		{
			//second circle
			for (size_t i = 0; i<secs; ++i)
			{
				deg = i * 2 * PI / secs;
				vertex.push_back(r2*sin(deg));
				vertex.push_back(r2*cos(deg));
				vertex.push_back(0.0f);
			}
		}
		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STREAM_DRAW);
	}

	void VertexArray::update_bound_cube()
	{
		//draw 12 sides
		std::vector<float> vertex;
		vertex.reserve(24 * 3);

		//s1-000-100
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.min().z());
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.min().z());
		//s2-100-101
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.min().z());
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.max().z());
		//s3-101-001
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.max().z());
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.max().z());
		//s4-001-000
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.max().z());
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.min().z());
		//s5-000-010
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.min().z());
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.min().z());
		//s6-100-110
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.min().z());
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.min().z());
		//s7-101-111
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.max().z());
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.max().z());
		//s8-001-011
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.min().y()); vertex.push_back(bbox_.max().z());
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.max().z());
		//s9-010-110
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.min().z());
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.min().z());
		//s10-110-111
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.min().z());
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.max().z());
		//s11-111-011
		vertex.push_back(bbox_.max().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.max().z());
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.max().z());
		//s12-011-010
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.max().z());
		vertex.push_back(bbox_.min().x()); vertex.push_back(bbox_.max().y()); vertex.push_back(bbox_.min().z());

		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STREAM_DRAW);
	}

	void VertexArray::update_clip_planes(bool update_index)
	{
		buffer_data(VABuf_Coord,
			sizeof(float)*float_list_.size(),
			&float_list_[0], GL_STREAM_DRAW);
		if (update_index)
		{
			std::vector<uint32_t> index;
			index.reserve(6 * 4 * 2);
			//indices
			index.push_back(4); index.push_back(0); index.push_back(5); index.push_back(1);
			index.push_back(4); index.push_back(0); index.push_back(1); index.push_back(5);
			index.push_back(7); index.push_back(3); index.push_back(6); index.push_back(2);
			index.push_back(7); index.push_back(3); index.push_back(2); index.push_back(6);
			index.push_back(1); index.push_back(0); index.push_back(3); index.push_back(2);
			index.push_back(1); index.push_back(0); index.push_back(2); index.push_back(3);
			index.push_back(4); index.push_back(5); index.push_back(6); index.push_back(7);
			index.push_back(4); index.push_back(5); index.push_back(7); index.push_back(6);
			index.push_back(0); index.push_back(4); index.push_back(2); index.push_back(6);
			index.push_back(0); index.push_back(4); index.push_back(6); index.push_back(2);
			index.push_back(5); index.push_back(1); index.push_back(7); index.push_back(3);
			index.push_back(5); index.push_back(1); index.push_back(3); index.push_back(7);
			buffer_data(VABuf_Index,
				sizeof(uint32_t)*index.size(),
				&index[0], GL_STATIC_DRAW);
		}
	}

	void VertexArray::update_grid()
	{
		//get parameters
		int grid_num = 0;
		double distance = 1.0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			grid_num = int(param->second+0.5);
		param = param_list_.find(1);
		if (param != param_list_.end())
			distance = param->second;
		int line_num = grid_num * 2 + 1;
		std::vector<float> vertex;
		vertex.reserve(line_num * 4 * 3);

		double gap = distance / grid_num;
		int i;
		for (i = 0; i<line_num; ++i)
		{
			vertex.push_back(float(-distance + gap*i));
			vertex.push_back(float(0.0));
			vertex.push_back(float(-distance));
			vertex.push_back(float(-distance + gap*i));
			vertex.push_back(float(0.0));
			vertex.push_back(float(distance));
		}
		for (i = 0; i<line_num; ++i)
		{
			vertex.push_back(float(-distance));
			vertex.push_back(float(0.0));
			vertex.push_back(float(-distance + gap*i));
			vertex.push_back(float(distance));
			vertex.push_back(float(0.0));
			vertex.push_back(float(-distance + gap*i));
		}
		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STREAM_DRAW);
	}

	void VertexArray::update_cam_jack()
	{
		double len = 1.0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			len = param->second;

		std::vector<float> vertex;
		vertex.reserve(6 * 3);

		vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
		vertex.push_back(len); vertex.push_back(0.0); vertex.push_back(0.0);
		vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
		vertex.push_back(0.0); vertex.push_back(len); vertex.push_back(0.0);
		vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(0.0);
		vertex.push_back(0.0); vertex.push_back(0.0); vertex.push_back(len);
		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STATIC_DRAW);
	}

	void VertexArray::update_crop_frame()
	{
		double x = 0.0;
		double y = 0.0;
		double w = 1.0;
		double h = 1.0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			x = param->second;
		param = param_list_.find(1);
		if (param != param_list_.end())
			y = param->second;
		param = param_list_.find(2);
		if (param != param_list_.end())
			w = param->second;
		param = param_list_.find(3);
		if (param != param_list_.end())
			h = param->second;

		std::vector<float> vertex;
		vertex.reserve(4 * 3);

		vertex.push_back(x - 1); vertex.push_back(y - 1); vertex.push_back(0.0);
		vertex.push_back(x + w + 1); vertex.push_back(y - 1); vertex.push_back(0.0);
		vertex.push_back(x + w + 1); vertex.push_back(y + h + 1); vertex.push_back(0.0);
		vertex.push_back(x - 1); vertex.push_back(y + h + 1); vertex.push_back(0.0);
		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STREAM_DRAW);
	}

	void VertexArray::update_scale_bar()
	{
		double x = 0.0;
		double y = 0.0;
		double w = 1.0;
		double h = 1.0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			x = param->second;
		param = param_list_.find(1);
		if (param != param_list_.end())
			y = param->second;
		param = param_list_.find(2);
		if (param != param_list_.end())
			w = param->second;
		param = param_list_.find(3);
		if (param != param_list_.end())
			h = param->second;

		std::vector<float> vertex;
		vertex.reserve(4 * 3);

		vertex.push_back(x); vertex.push_back(y); vertex.push_back(0.0);
		vertex.push_back(x - w); vertex.push_back(y); vertex.push_back(0.0);
		vertex.push_back(x); vertex.push_back(y - h); vertex.push_back(0.0);
		vertex.push_back(x - w); vertex.push_back(y - h); vertex.push_back(0.0);
		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STREAM_DRAW);
	}

	void VertexArray::update_legend_squares()
	{
		double px1 = 0.0;
		double py1 = 0.0;
		double px2 = 1.0;
		double py2 = 1.0;
		auto param = param_list_.find(0);
		if (param != param_list_.end())
			px1 = param->second;
		param = param_list_.find(1);
		if (param != param_list_.end())
			py1 = param->second;
		param = param_list_.find(2);
		if (param != param_list_.end())
			px2 = param->second;
		param = param_list_.find(3);
		if (param != param_list_.end())
			py2 = param->second;

		std::vector<float> vertex;
		vertex.reserve(8 * 3);

		vertex.push_back(px1 - 1.0); vertex.push_back(py2 + 1.0); vertex.push_back(0.0);
		vertex.push_back(px2 + 1.0); vertex.push_back(py2 + 1.0); vertex.push_back(0.0);
		vertex.push_back(px1 - 1.0); vertex.push_back(py1 - 1.0); vertex.push_back(0.0);
		vertex.push_back(px2 + 1.0); vertex.push_back(py1 - 1.0); vertex.push_back(0.0);
		vertex.push_back(px1); vertex.push_back(py2); vertex.push_back(0.0);
		vertex.push_back(px2); vertex.push_back(py2); vertex.push_back(0.0);
		vertex.push_back(px1); vertex.push_back(py1); vertex.push_back(0.0);
		vertex.push_back(px2); vertex.push_back(py1); vertex.push_back(0.0);
		buffer_data(VABuf_Coord,
			sizeof(float) * vertex.size(),
			&vertex[0], GL_STREAM_DRAW);
	}

	void VertexArray::update_grad_bkg()
	{
		buffer_data(VABuf_Coord,
			sizeof(float) * float_list_.size(),
			&float_list_[0], GL_STREAM_DRAW);
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
		//create vertex buffer
		VertexBuffer* vb = new VertexBuffer(VABuf_Coord);
		vb->create();
		//add to list
		vb_list_.push_back(vb);
		//attach buffer
		va->attach_buffer(vb);
		if (type == VA_Norm_Square)
		{
			//assign data
			float points[] = {
				-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f };
			vb->data(sizeof(float) * 24, points, GL_STATIC_DRAW);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
			va->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);
		}
		else if (type == VA_Norm_Square_d)
		{
			//set param
			va->set_param(0, 0.0);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
			va->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);
		}
		else if (type == VA_Brush_Circles)
		{
			//set param
			std::vector<std::pair<unsigned int, double>> params;
			params.push_back(std::pair<unsigned int, double>(0, 10.0));
			params.push_back(std::pair<unsigned int, double>(1, 10.0));
			params.push_back(std::pair<unsigned int, double>(2, 60.0));
			va->set_param(params);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		}
		else if (type == VA_Bound_Cube)
		{
			//set param
			BBox bbox(Point(0.0), Point(1.0));
			va->set_param(bbox);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		}
		else if (type == VA_Clip_Planes)
		{
			//index buffer
			vb = new VertexBuffer(VABuf_Index);
			vb->create();
			vb_list_.push_back(vb);
			//attach buffer
			va->attach_buffer(vb);
			//set param
			std::vector<Point> pp;
			va->set_param(pp);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		}
		else if (type == VA_Grid)
		{
			//set param
			std::vector<std::pair<unsigned int, double>> params;
			params.push_back(std::pair<unsigned int, double>(0, 5.0));
			params.push_back(std::pair<unsigned int, double>(1, 10.0));
			va->set_param(params);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		}
		else if (type == VA_Cam_Jack)
		{
			//set param
			va->set_param(0, 1.0);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		}
		else if (type == VA_Crop_Frame ||
			type == VA_Scale_Bar ||
			type == VA_Legend_Squares)
		{
			//set param
			std::vector<std::pair<unsigned int, double>> params;
			params.push_back(std::pair<unsigned int, double>(0, 0.0));
			params.push_back(std::pair<unsigned int, double>(1, 0.0));
			params.push_back(std::pair<unsigned int, double>(2, 1.0));
			params.push_back(std::pair<unsigned int, double>(3, 1.0));
			va->set_param(params);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const GLvoid*)0);
		}
		else if (type == VA_Grad_Bkg)
		{
			//set param
			std::vector<float> list(48, 0.0);
			va->set_param(list);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
			va->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);
		}
		else if (type == VA_Color_Map)
		{
			//set param
			std::vector<float> list(98, 0.0);
			va->set_param(list);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const GLvoid*)0);
			va->attrib_pointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const GLvoid*)12);
		}
		else if (type == VA_Traces ||
			type == VA_Rulers)
		{
			//set param
			std::vector<float> vertex(12, 0.0);
			va->buffer_data(VABuf_Coord,
				sizeof(float) * vertex.size(),
				&vertex[0], GL_STREAM_DRAW);
			//set attrib
			va->attrib_pointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)0);
			va->attrib_pointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (const GLvoid*)12);
			va->dirty_ = true;
		}
		if (type == VA_Text)
		{
			//assign data
			float points[] = {
				0.0f, 0.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 1.0f,
				1.0f, 1.0f, 1.0f, 1.0f};
			vb->data(sizeof(float) * 16, points, GL_STATIC_DRAW);
			//set attrib
			va->attrib_pointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
		}
		//unbind
		va->unbind();

		return va;
	}

	VertexArray* VertexArrayManager::vertex_array(bool vbuf, bool ibuf)
	{
		//create new vertex array
		VertexArray* va = new VertexArray(VA_Unmanaged);
		va->create();
		va->bind();

		if (vbuf)
		{
			//create vertex buffer
			VertexBuffer* vb = new VertexBuffer(VABuf_Coord);
			vb->create();
			//attach buffer
			va->attach_buffer(vb);
		}
		if (ibuf)
		{
			//create vertex buffer
			VertexBuffer* vb = new VertexBuffer(VABuf_Index);
			vb->create();
			//attach buffer
			va->attach_buffer(vb);
		}
		//unbind
		//va->unbind();

		return va;
	}
}