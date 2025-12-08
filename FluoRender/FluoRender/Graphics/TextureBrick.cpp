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
#include <TextureBrick.h>
#include <Texture.h>
#include <TextureRenderer.h>
#include <Global.h>
#include <MainSettings.h>
#include <Utils.h>
#include <compatibility.h>
#include <math.h>
#include <utility>
#include <iostream>
#include <fstream>
#include <filesystem>

namespace flvr
{
	std::map<std::wstring, std::wstring> TextureBrick::cache_table_ = std::map<std::wstring, std::wstring>();

	TextureBrick::TextureBrick(Nrrd* n0,
		const fluo::Vector& size, int byte,
		const fluo::Vector& off_size,
		const fluo::Vector& msize,
		const fluo::BBox& bbox,
		const fluo::BBox& tbox,
		const fluo::BBox& dbox,
		unsigned int id,
		int findex,
		long long offset,
		long long fsize)
		: size_(size),
		off_size_(off_size),
		msize_(msize),
		bbox_(bbox),
		tbox_(tbox),
		dbox_(dbox),
		id_(id),
		findex_(findex),
		offset_(offset),
		fsize_(fsize),
		mask_valid_(false),
		mask_act_(false),
		new_grown_(false)
	{
		compute_edge_rays(bbox_);
		compute_edge_rays_tex(tbox_);

		TexComp comp = { CompType::Data, byte, n0 };
		set_nrrd(CompType::Data, comp);

		//if it's been drawn in a full update loop
		for (int i = 0; i < TEXTURE_RENDER_MODES; i++)
			drawn_[i] = false;

		//priority
		priority_ = 0;

		//brkxml
		brkdata_ = NULL;
		id_in_loadedbrks = -1;
		loading_ = false;
		disp_ = true;
		//prevent_tex_deletion_ = false;
	}

	TextureBrick::~TextureBrick()
	{
		if (brkdata_) delete[] brkdata_;
	}

	/* The cube is numbered in the following way

 Corners:

 2________6        y
 /|        |        |
 / |       /|        |
 /  |      / |        |
 /   0_____/__4        |
 3---------7   /        |_________ x
 |  /      |  /         /
 | /       | /         /
 |/        |/         /
 1_________5         /
 z

 Edges:

 ,____1___,        y
 /|        |        |
 / 0       /|        |
 9  |     10 2        |
 /   |__3__/__|        |
 /_____5___/   /        |_________ x
 |  /      | 11         /
 4 8       6 /         /
 |/        |/         /
 |____7____/         /
 z
	 */

	void TextureBrick::compute_edge_rays(fluo::BBox &bbox)
	{
		// set up vertices
		fluo::Point corner[8];
		corner[0] = bbox.Min();
		corner[1] = fluo::Point(bbox.Min().x(), bbox.Min().y(), bbox.Max().z());
		corner[2] = fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Min().z());
		corner[3] = fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Max().z());
		corner[4] = fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Min().z());
		corner[5] = fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Max().z());
		corner[6] = fluo::Point(bbox.Max().x(), bbox.Max().y(), bbox.Min().z());
		corner[7] = bbox.Max();

		// set up edges
		edge_[0] = fluo::Ray(corner[0], corner[2] - corner[0]);
		edge_[1] = fluo::Ray(corner[2], corner[6] - corner[2]);
		edge_[2] = fluo::Ray(corner[4], corner[6] - corner[4]);
		edge_[3] = fluo::Ray(corner[0], corner[4] - corner[0]);
		edge_[4] = fluo::Ray(corner[1], corner[3] - corner[1]);
		edge_[5] = fluo::Ray(corner[3], corner[7] - corner[3]);
		edge_[6] = fluo::Ray(corner[5], corner[7] - corner[5]);
		edge_[7] = fluo::Ray(corner[1], corner[5] - corner[1]);
		edge_[8] = fluo::Ray(corner[0], corner[1] - corner[0]);
		edge_[9] = fluo::Ray(corner[2], corner[3] - corner[2]);
		edge_[10] = fluo::Ray(corner[6], corner[7] - corner[6]);
		edge_[11] = fluo::Ray(corner[4], corner[5] - corner[4]);
	}

	void TextureBrick::compute_edge_rays_tex(fluo::BBox &bbox)
	{
		// set up vertices
		fluo::Point corner[8];
		corner[0] = bbox.Min();
		corner[1] = fluo::Point(bbox.Min().x(), bbox.Min().y(), bbox.Max().z());
		corner[2] = fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Min().z());
		corner[3] = fluo::Point(bbox.Min().x(), bbox.Max().y(), bbox.Max().z());
		corner[4] = fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Min().z());
		corner[5] = fluo::Point(bbox.Max().x(), bbox.Min().y(), bbox.Max().z());
		corner[6] = fluo::Point(bbox.Max().x(), bbox.Max().y(), bbox.Min().z());
		corner[7] = bbox.Max();

		// set up edges
		tex_edge_[0] = fluo::Ray(corner[0], corner[2] - corner[0]);
		tex_edge_[1] = fluo::Ray(corner[2], corner[6] - corner[2]);
		tex_edge_[2] = fluo::Ray(corner[4], corner[6] - corner[4]);
		tex_edge_[3] = fluo::Ray(corner[0], corner[4] - corner[0]);
		tex_edge_[4] = fluo::Ray(corner[1], corner[3] - corner[1]);
		tex_edge_[5] = fluo::Ray(corner[3], corner[7] - corner[3]);
		tex_edge_[6] = fluo::Ray(corner[5], corner[7] - corner[5]);
		tex_edge_[7] = fluo::Ray(corner[1], corner[5] - corner[1]);
		tex_edge_[8] = fluo::Ray(corner[0], corner[1] - corner[0]);
		tex_edge_[9] = fluo::Ray(corner[2], corner[3] - corner[2]);
		tex_edge_[10] = fluo::Ray(corner[6], corner[7] - corner[6]);
		tex_edge_[11] = fluo::Ray(corner[4], corner[5] - corner[4]);
	}

	// compute polygon of edge plane intersections
	void TextureBrick::compute_polygons(fluo::Ray& view, double dt,
		std::vector<float>& vertex, std::vector<uint32_t>& index,
		std::vector<uint32_t>& size, bool bricks)
	{
		if (dt <= 0.0)
			return;

		fluo::Point corner[8];
		corner[0] = bbox_.Min();
		corner[1] = fluo::Point(bbox_.Min().x(), bbox_.Min().y(), bbox_.Max().z());
		corner[2] = fluo::Point(bbox_.Min().x(), bbox_.Max().y(), bbox_.Min().z());
		corner[3] = fluo::Point(bbox_.Min().x(), bbox_.Max().y(), bbox_.Max().z());
		corner[4] = fluo::Point(bbox_.Max().x(), bbox_.Min().y(), bbox_.Min().z());
		corner[5] = fluo::Point(bbox_.Max().x(), bbox_.Min().y(), bbox_.Max().z());
		corner[6] = fluo::Point(bbox_.Max().x(), bbox_.Max().y(), bbox_.Min().z());
		corner[7] = bbox_.Max();

		double tmin = fluo::Dot(corner[0] - view.origin(), view.direction());
		double tmax = tmin;
		uint32_t maxi = 0;
		double t;
		for (uint32_t i = 1; i < 8; i++)
		{
			t = fluo::Dot(corner[i] - view.origin(), view.direction());
			tmin = std::min(t, tmin);
			if (t > tmax) { maxi = i; tmax = t; }
		}

		// Make all of the slices consistent by offsetting them to a fixed
		// position in space (the origin).  This way they are consistent
		// between bricks and don't change with camera zoom.
		if (bricks)
		{
			double tanchor = Dot(corner[maxi], view.direction());
			double tanchor0 = floor(tanchor / dt)*dt;
			double tanchordiff = tanchor - tanchor0;
			tmax -= tanchordiff;
		}

		compute_polygons(view, tmin, tmax, dt, vertex, index, size);
	}

	// compute polygon list of edge plane intersections
	//
	// This is never called externally and could be private.
	//
	// The representation returned is not efficient, but it appears a
	// typical rendering only contains about 1k triangles.
	void TextureBrick::compute_polygons(fluo::Ray& view,
		double tmin, double tmax, double dt,
		std::vector<float>& vertex, std::vector<uint32_t>& index,
		std::vector<uint32_t>& size)
	{
		if (dt <= 0.0)
			return;

		fluo::Vector vv[12], tt[12]; // temp storage for vertices and texcoords

		uint32_t degree = 0;

		// find up and right vectors
		fluo::Vector vdir = view.direction();
		view_vector_ = vdir;
		fluo::Vector up;
		fluo::Vector right;
		switch (fluo::MinIndex(fabs(vdir.x()),
			fabs(vdir.y()),
			fabs(vdir.z())))
		{
		case 0:
			up.x(0.0); up.y(-vdir.z()); up.z(vdir.y());
			break;
		case 1:
			up.x(-vdir.z()); up.y(0.0); up.z(vdir.x());
			break;
		case 2:
			up.x(-vdir.y()); up.y(vdir.x()); up.z(0.0);
			break;
		}
		up.normalize();
		right = Cross(vdir, up);
		bool order = glbin_settings.m_update_order;
		size_t vert_count = 0;
		for (double t = order ? tmin : tmax;
		order ? (t < tmax) : (t > tmin);
			t += order ? dt : -dt)
		{
			// we compute polys back to front
			// find intersections
			degree = 0;
			for (size_t j = 0; j < 12; j++)
			{
				double u;

				fluo::Vector vec = -view.direction();
				fluo::Point pnt = view.parameter(t);
				bool intersects = edge_[j].planeIntersectParameter
					(vec, pnt, u);
				if (intersects && u >= 0.0 && u <= 1.0)
				{
					fluo::Point p;
					p = edge_[j].parameter(u);
					vv[degree] = (fluo::Vector)p;
					p = tex_edge_[j].parameter(u);
					tt[degree] = (fluo::Vector)p;
					degree++;
				}
			}

			if (degree < 3 || degree >6) continue;
			bool sorted = degree > 3;
			uint32_t idx[6];
			if (sorted) {
				// compute centroids
				fluo::Vector vc(0.0, 0.0, 0.0), tc(0.0, 0.0, 0.0);
				for (size_t j = 0; j < degree; j++)
				{
					vc += vv[j]; tc += tt[j];
				}
				vc /= (double)degree; tc /= (double)degree;

				// sort vertices
				double pa[6];
				for (uint32_t i = 0; i < degree; i++)
				{
					double vx = Dot(vv[i] - vc, right);
					double vy = Dot(vv[i] - vc, up);

					// compute pseudo-angle
					pa[i] = vy / (fabs(vx) + fabs(vy));
					if (vx < 0.0) pa[i] = 2.0 - pa[i];
					else if (vy < 0.0) pa[i] = 4.0 + pa[i];
					// init idx
					idx[i] = i;
				}
				fluo::Sort(pa, idx, degree);
			}
			// save all of the indices
			for (uint32_t j = 1; j < degree - 1; j++) {
				index.push_back(static_cast<unsigned int>(vert_count));
				index.push_back(static_cast<unsigned int>(vert_count + j));
				index.push_back(static_cast<unsigned int>(vert_count + j + 1));
			}
			// save all of the verts
			for (uint32_t j = 0; j < degree; j++)
			{
				vertex.push_back(static_cast<float>((sorted ? vv[idx[j]] : vv[j]).x()));
				vertex.push_back(static_cast<float>((sorted ? vv[idx[j]] : vv[j]).y()));
				vertex.push_back(static_cast<float>((sorted ? vv[idx[j]] : vv[j]).z()));
				vertex.push_back(static_cast<float>((sorted ? tt[idx[j]] : tt[j]).x()));
				vertex.push_back(static_cast<float>((sorted ? tt[idx[j]] : tt[j]).y()));
				vertex.push_back(static_cast<float>((sorted ? tt[idx[j]] : tt[j]).z()));
				vert_count++;
			}

			size.push_back(degree);
		}
	}

	GLenum TextureBrick::tex_type_aux(Nrrd* n)
	{
		// GL_BITMAP!?
		if (n->type == nrrdTypeChar)   return GL_BYTE;
		if (n->type == nrrdTypeUChar)  return GL_UNSIGNED_BYTE;
		if (n->type == nrrdTypeShort)  return GL_SHORT;
		if (n->type == nrrdTypeUShort) return GL_UNSIGNED_SHORT;
		if (n->type == nrrdTypeInt)    return GL_INT;
		if (n->type == nrrdTypeUInt)   return GL_UNSIGNED_INT;
		if (n->type == nrrdTypeFloat)  return GL_FLOAT;
		return GL_NONE;
	}

	size_t TextureBrick::tex_type_size(GLenum t)
	{
		if (t == GL_BYTE) { return sizeof(GLbyte); }
		if (t == GL_UNSIGNED_BYTE) { return sizeof(GLubyte); }
		if (t == GL_SHORT) { return sizeof(GLshort); }
		if (t == GL_UNSIGNED_SHORT) { return sizeof(GLushort); }
		if (t == GL_INT) { return sizeof(GLint); }
		if (t == GL_UNSIGNED_INT) { return sizeof(GLuint); }
		if (t == GL_FLOAT) { return sizeof(GLfloat); }
		return 0;
	}

	GLenum TextureBrick::tex_type(CompType type)
	{
		auto c = data_.find(type);
		if (c == data_.end())
			return GL_NONE;

		Nrrd* nrrd = c->second.data;
		if (!nrrd)
			return GL_NONE;
		return tex_type_aux(nrrd);
	}

	void *TextureBrick::tex_data(CompType type)
	{
		auto c = data_.find(type);
		if (c == data_.end())
			return nullptr;
		Nrrd* nrrd = c->second.data;
		if (!nrrd)
			return nullptr;

		int bytes = c->second.bytes;
		auto stride = get_stride();
		unsigned char *ptr = (unsigned char *)(nrrd->data);
		unsigned long long offset =
			(unsigned long long)(off_size_.intz()) *
			(unsigned long long)(stride.intx()) *
			(unsigned long long)(stride.inty()) +
			(unsigned long long)(off_size_.inty()) *
			(unsigned long long)(stride.intx()) +
			(unsigned long long)(off_size_.intx());
		return ptr + offset * bytes;
	}

	void* TextureBrick::tex_data(CompType type, void* raw_data)
	{
		auto c = data_.find(type);
		if (c == data_.end())
			return nullptr;
		Nrrd* nrrd = c->second.data;
		if (!nrrd)
			return nullptr;

		int bytes = c->second.bytes;
		auto stride = get_stride();
		unsigned char *ptr = (unsigned char *)(raw_data);
		unsigned long long offset =
			(unsigned long long)(off_size_.intz()) *
			(unsigned long long)(stride.intx()) *
			(unsigned long long)(stride.inty()) +
			(unsigned long long)(off_size_.inty()) *
			(unsigned long long)(stride.intx()) +
			(unsigned long long)(off_size_.intx());
		return ptr + offset * bytes;
	}

	void *TextureBrick::tex_data_brk(CompType type, const FileLocInfo* finfo)
	{
		auto c = data_.find(type);
		if (c == data_.end())
			return nullptr;
		int bytes = c->second.bytes;

		unsigned char *ptr = nullptr;
		if (brkdata_)
			ptr = (unsigned char *)(brkdata_);
		else
		{
			size_t mem_size = (size_t)size_.intx() * (size_t)size_.inty() * (size_t)size_.intz() * bytes;
			ptr = new unsigned char[mem_size];
			if (!read_brick((char *)ptr, mem_size, finfo))
			{
				delete[] ptr;
				return nullptr;
			}
			brkdata_ = (char*)ptr;
		}
		return ptr;
	}

	void TextureBrick::set_priority()
	{
		CompType type = CompType::Data;
		auto c = data_.find(type);
		if (c == data_.end())
			return;
		Nrrd* nrrd = c->second.data;
		if (!nrrd)
		{
			priority_ = 0;
			return;
		}

		int bytes = c->second.bytes;
		switch (bytes)
		{
		case 1:
		{
			unsigned char max = 0;
			unsigned char *ptr = (unsigned char *)(nrrd->data);
			for (int i = 0; i < size_.intx(); ++i) for (int j = 0; j < size_.inty(); ++j) for (int k = 0; k < size_.intz(); ++k)
			{
				unsigned long long index =
					(unsigned long long)(size_.intx()) *
					(unsigned long long)(size_.inty()) *
					(unsigned long long)(off_size_.intz() + k) +
					(unsigned long long)(size_.intx()) *
					(unsigned long long)(off_size_.inty() + j) +
					(unsigned long long)(off_size_.intx() + i);
				max = ptr[index] > max ? ptr[index] : max;
			}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
		}
			break;
		case 2:
		{
			unsigned short max = 0;
			unsigned short *ptr = (unsigned short *)(nrrd->data);
			for (int i = 0; i < size_.intx(); ++i) for (int j = 0; j < size_.inty(); ++j) for (int k = 0; k < size_.intz(); ++k)
			{
				unsigned long long index =
					(unsigned long long)(size_.intx()) *
					(unsigned long long)(size_.inty()) *
					(unsigned long long)(off_size_.intz() + k) +
					(unsigned long long)(size_.intx()) *
					(unsigned long long)(off_size_.inty() + j) +
					(unsigned long long)(off_size_.intx() + i);
				max = ptr[index] > max ? ptr[index] : max;
			}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
		}
			break;
		case 4:
		{
			unsigned int max = 0;
			unsigned int *ptr = (unsigned int *)(nrrd->data);
			for (int i = 0; i < size_.intx(); ++i) for (int j = 0; j < size_.inty(); ++j) for (int k = 0; k < size_.intz(); ++k)
			{
				unsigned long long index =
					(unsigned long long)(size_.intx()) *
					(unsigned long long)(size_.inty()) *
					(unsigned long long)(off_size_.intz() + k) +
					(unsigned long long)(size_.intx()) *
					(unsigned long long)(off_size_.inty() + j) +
					(unsigned long long)(off_size_.intx() + i);
				max = ptr[index] > max ? ptr[index] : max;
			}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
		}
			break;
		}
	}

	void TextureBrick::freeBrkData()
	{
		if (brkdata_) delete[] brkdata_;
		brkdata_ = NULL;
	}

	bool TextureBrick::read_brick(char* data, size_t size, const FileLocInfo* finfo)
	{
		if (!finfo) return false;

		//if (finfo->isurl)
		//{
		//	if (finfo->type == BRICK_FILE_TYPE_RAW)  return raw_brick_reader_url(data, size, finfo);
		//	if (finfo->type == BRICK_FILE_TYPE_JPEG) return jpeg_brick_reader_url(data, size, finfo);
		//	if (finfo->type == BRICK_FILE_TYPE_ZLIB) return zlib_brick_reader_url(data, size, finfo);
		//}
		//else
		//{
			if (finfo->type == BRICK_FILE_TYPE_RAW)  return raw_brick_reader(data, size, finfo);
		//	if (finfo->type == BRICK_FILE_TYPE_JPEG) return jpeg_brick_reader(data, size, finfo);
		//	if (finfo->type == BRICK_FILE_TYPE_ZLIB) return zlib_brick_reader(data, size, finfo);
		//}

		return false;
	}

	bool TextureBrick::raw_brick_reader(char* data, size_t size, const FileLocInfo* finfo)
	{
		try
		{
			std::ifstream ifs;
			ifs.open(ws2s(finfo->filename), std::ios::binary);
			if (!ifs) return false;
			if (finfo->datasize > 0 && size != finfo->datasize) return false;
			size_t read_size = finfo->datasize > 0 ? finfo->datasize : size;
			ifs.seekg(finfo->offset, std::ios_base::beg);
			ifs.read(data, read_size);
			if (ifs) ifs.close();
			/*
			FILE* fp = fopen(ws2s(finfo->filename).c_str(), "rb");
			if (!fp) return false;
			if (finfo->datasize > 0 && size != finfo->datasize) return false;
			size_t read_size = finfo->datasize > 0 ? finfo->datasize : size;
			setvbuf(fp, NULL, _IONBF, 0);
			fseek(fp, finfo->offset, SEEK_SET);
			fread(data, 0x1, read_size, fp);
			if (fp) fclose(fp);
			*//*
			ofstream ofs1;
			wstring str = *fname + wstring(L".txt");
			ofs1.open(str);
			for(int i=0; i < size/2; i++){
			ofs1 << ((unsigned short *)data)[i] << "\n";
			}
			ofs1.close();
			*/
		}
		catch (std::exception &e)
		{
			std::cerr << typeid(e).name() << "\n" << e.what() << std::endl;
			return false;
		}

		return true;
	}

	bool TextureBrick::read_brick_without_decomp(char* &data, size_t &readsize, FileLocInfo* finfo, void *th)
	{
		readsize = -1;

		if (!finfo) return false;

		if (finfo->isurl)
		{
			bool found_cache = false;
			std::wstring cfname = finfo->cache_filename;
			if (finfo->cached && std::filesystem::exists(cfname))
				found_cache = true;
			else
			{
				std::wstring fcname;

				auto itr = cache_table_.find(fcname);
				if (itr != cache_table_.end())
				{
					cfname = itr->second;
					if (std::filesystem::exists(cfname))
						found_cache = true;
				}

				if (found_cache)
				{
					finfo->cached = true;
					finfo->cache_filename = fcname;
				}
			}

			if (!found_cache)
			{
				//network
			}
		}

		std::ifstream ifs;
		std::wstring fn = finfo->cached ? finfo->cache_filename : finfo->filename;
		ifs.open(ws2s(fn), std::ios::binary);
		if (!ifs) return false;
		size_t zsize = finfo->datasize;
		if (zsize <= 0) zsize = (size_t)ifs.seekg(0, std::ios::end).tellg();
		char *zdata = new char[zsize];
		ifs.seekg(finfo->offset, std::ios_base::beg);
		ifs.read((char*)zdata, zsize);
		if (ifs) ifs.close();
		data = zdata;
		readsize = zsize;

		return true;
	}

	bool TextureBrick::is_nbmask_valid(Texture* tex)
	{
		if (mask_valid_) return true;
		//check neighbors
		unsigned int nid;
		TextureBrick* nb;
		nid = tex->negxid(id_);
		//negx
		if (nid != id_)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->mask_valid_)
				return true;
		}
		//negy
		nid = tex->negyid(id_);
		if (nid != id_)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->mask_valid_)
				return true;
		}
		//negz
		nid = tex->negzid(id_);
		if (nid != id_)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->mask_valid_)
				return true;
		}
		//posx
		nid = tex->posxid(id_);
		if (nid != id_)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->mask_valid_)
				return true;
		}
		//posy
		nid = tex->posyid(id_);
		if (nid != id_)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->mask_valid_)
				return true;
		}
		//posz
		nid = tex->poszid(id_);
		if (nid != id_)
		{
			nb = tex->get_brick(nid);
			if (nb && nb->mask_valid_)
				return true;
		}
		return false;
	}

} // end namespace flvr
