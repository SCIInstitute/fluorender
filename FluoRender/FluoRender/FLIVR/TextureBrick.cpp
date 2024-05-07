//
//  For more information, please see: http://software.sci.utah.edu
//
//  The MIT License
//
//  Copyright (c) 2024 Scientific Computing and Imaging Institute,
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

#include <Global.h>
#include <compatibility.h>
#include <Texture.h>
#include <math.h>
#include <TextureBrick.h>
#include <TextureRenderer.h>
#include <Types/Utils.h>
#include <utility>
#include <iostream>
#include <fstream>
#include <wx/filefn.h>

using namespace std;

namespace flvr
{
	map<wstring, wstring> TextureBrick::cache_table_ = map<wstring, wstring>();

	TextureBrick::TextureBrick(Nrrd* n0, Nrrd* n1,
		int nx, int ny, int nz, int nc, int* nb,
		int ox, int oy, int oz,
		int mx, int my, int mz,
		const fluo::BBox& bbox, const fluo::BBox& tbox, const fluo::BBox& dbox,
		unsigned int id,
		int findex, long long offset, long long fsize)
		: nx_(nx), ny_(ny), nz_(nz), nc_(nc), ox_(ox), oy_(oy), oz_(oz),
		mx_(mx), my_(my), mz_(mz), bbox_(bbox), tbox_(tbox), dbox_(dbox),
		id_(id),
		findex_(findex), offset_(offset), fsize_(fsize),
		mask_valid_(false), mask_act_(false),
		new_grown_(false)
	{
		for (int i = 0; i < TEXTURE_MAX_COMPONENTS; i++)
		{
			data_[i] = 0;
			nb_[i] = 0;
			ntype_[i] = TYPE_NONE;
		}

		for (int c = 0; c < nc_; c++)
		{
			nb_[c] = nb[c];
		}
		if (nc_ == 1)
		{
			ntype_[0] = TYPE_INT;
		}
		else if (nc_ > 1)
		{
			ntype_[0] = TYPE_INT_GRAD;
			ntype_[1] = TYPE_GM;
		}

		compute_edge_rays(bbox_);
		compute_edge_rays_tex(tbox_);

		data_[0] = n0;
		data_[1] = n1;

		nmask_ = -1;

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
		// Creator of the brick owns the nrrds.
		// This object never deletes that memory.
		data_[0] = 0;
		data_[1] = 0;

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
		vector<float>& vertex, vector<uint32_t>& index,
		vector<uint32_t>& size, bool bricks)
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
		vector<float>& vertex, vector<uint32_t>& index,
		vector<uint32_t>& size)
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
				for (int j = 0; j < degree; j++)
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
				index.push_back(vert_count);
				index.push_back(vert_count + j);
				index.push_back(vert_count + j + 1);
			}
			// save all of the verts
			for (uint32_t j = 0; j < degree; j++)
			{
				vertex.push_back((sorted ? vv[idx[j]] : vv[j]).x());
				vertex.push_back((sorted ? vv[idx[j]] : vv[j]).y());
				vertex.push_back((sorted ? vv[idx[j]] : vv[j]).z());
				vertex.push_back((sorted ? tt[idx[j]] : tt[j]).x());
				vertex.push_back((sorted ? tt[idx[j]] : tt[j]).y());
				vertex.push_back((sorted ? tt[idx[j]] : tt[j]).z());
				vert_count++;
			}

			size.push_back(degree);
		}
	}

	int TextureBrick::sx()
	{
		if (data_[0]->dim == 3)
			return (int)data_[0]->axis[0].size;
		else
			return (int)data_[0]->axis[1].size;
	}

	int TextureBrick::sy()
	{
		if (data_[0]->dim == 3)
			return (int)data_[0]->axis[1].size;
		else
			return (int)data_[0]->axis[2].size;
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

	GLenum TextureBrick::tex_type(int c)
	{
		if (c < nc_)
			return tex_type_aux(data_[c]);
		else if (c == nmask_)
			return GL_UNSIGNED_BYTE;
		else if (c == nlabel_)
			return GL_UNSIGNED_INT;
		else
			return GL_NONE;
	}

	void *TextureBrick::tex_data(int c)
	{
		if (c >= 0 && data_[c])
		{
			unsigned char *ptr = (unsigned char *)(data_[c]->data);
			long long offset = (long long)(oz()) *
				(long long)(sx()) *
				(long long)(sy()) +
				(long long)(oy()) *
				(long long)(sx()) +
				(long long)(ox());
			return ptr + offset * tex_type_size(tex_type(c));
		}
		else
			return NULL;
	}

	void *TextureBrick::tex_data_brk(int c, const FileLocInfo* finfo)
	{
		unsigned char *ptr = NULL;
		if (brkdata_) ptr = (unsigned char *)(brkdata_);
		else
		{
			int bd = tex_type_size(tex_type(c));
			ptr = new unsigned char[(size_t)nx_*(size_t)ny_*(size_t)nz_*(size_t)bd];
			if (!read_brick((char *)ptr, (size_t)nx_*(size_t)ny_*(size_t)nz_*(size_t)bd, finfo))
			{
				delete[] ptr;
				return NULL;
			}
			brkdata_ = (char*)ptr;
		}
		return ptr;
	}

	void TextureBrick::set_priority()
	{
		if (!data_[0])
		{
			priority_ = 0;
			return;
		}
		size_t vs = tex_type_size(tex_type(0));
		size_t sx = data_[0]->axis[0].size;
		size_t sy = data_[0]->axis[1].size;
		if (vs == 1)
		{
			unsigned char max = 0;
			unsigned char *ptr = (unsigned char *)(data_[0]->data);
			for (int i = 0; i < nx_; i++)
				for (int j = 0; j < ny_; j++)
					for (int k = 0; k < nz_; k++)
					{
						long long index = (long long)(sx)* (long long)(sy)*
							(long long)(oz_ + k) + (long long)(sx)*
							(long long)(oy_ + j) + (long long)(ox_ + i);
						max = ptr[index] > max ? ptr[index] : max;
					}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
		}
		else if (vs == 2)
		{
			unsigned short max = 0;
			unsigned short *ptr = (unsigned short *)(data_[0]->data);
			for (int i = 0; i < nx_; i++)
				for (int j = 0; j < ny_; j++)
					for (int k = 0; k < nz_; k++)
					{
						long long index = (long long)(sx)* (long long)(sy)*
							(long long)(oz_ + k) + (long long)(sx)*
							(long long)(oy_ + j) + (long long)(ox_ + i);
						max = ptr[index] > max ? ptr[index] : max;
					}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
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
			ifstream ifs;
			ifs.open(ws2s(finfo->filename), ios::binary);
			if (!ifs) return false;
			if (finfo->datasize > 0 && size != finfo->datasize) return false;
			size_t read_size = finfo->datasize > 0 ? finfo->datasize : size;
			ifs.seekg(finfo->offset, ios_base::beg);
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
			cerr << typeid(e).name() << "\n" << e.what() << endl;
			return false;
		}

		return true;
	}

	bool TextureBrick::read_brick_without_decomp(char* &data, size_t &readsize, FileLocInfo* finfo, wxThread *th)
	{
		readsize = -1;

		if (!finfo) return false;

		if (finfo->isurl)
		{
			bool found_cache = false;
			wxString wxcfname = finfo->cache_filename;
			if (finfo->cached && wxFileExists(wxcfname))
				found_cache = true;
			else
			{
				wstring fcname;

				auto itr = cache_table_.find(fcname);
				if (itr != cache_table_.end())
				{
					wxcfname = itr->second;
					if (wxFileExists(wxcfname))
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
/*				CURLcode ret;

				if (s_curl_ == NULL) {
					cerr << "curl_easy_init() failed" << endl;
					return false;
				}
				curl_easy_reset(s_curl_);
				curl_easy_setopt(s_curl_, CURLOPT_URL, wxString::wxString(finfo->filename).ToStdString().c_str());
				curl_easy_setopt(s_curl_, CURLOPT_TIMEOUT, 10L);
				curl_easy_setopt(s_curl_, CURLOPT_USERAGENT, "libcurl-agent/1.0");
				curl_easy_setopt(s_curl_, CURLOPT_WRITEFUNCTION, WriteFileCallback);
				curl_easy_setopt(s_curl_, CURLOPT_SSL_VERIFYPEER, 0);
				curl_easy_setopt(s_curl_, CURLOPT_NOSIGNAL, 1);
				curl_easy_setopt(s_curl_, CURLOPT_FAILONERROR, 1);
				curl_easy_setopt(s_curl_, CURLOPT_FILETIME, 1);
				curl_easy_setopt(s_curl_, CURLOPT_XFERINFOFUNCTION, xferinfo);
				curl_easy_setopt(s_curl_, CURLOPT_XFERINFODATA, th);
				curl_easy_setopt(s_curl_, CURLOPT_NOPROGRESS, 0L);

				wxString expath = wxStandardPaths::Get().GetExecutablePath();
				expath = expath.BeforeLast(GETSLASH(), NULL);
#ifdef _WIN32
				wxString dft = expath + "\\vvd_cache";
				wxString dft2 = wxStandardPaths::Get().GetUserDataDir() + "\\vvd_cache";
				if (!wxDirExists(dft) && wxDirExists(dft2))
					dft = dft2;
				else if (!wxDirExists(dft))
					wxMkdir(dft);
				dft += L"\\";
#else
				wxString dft = expath + "/../Resources/vvd_cache";
				if (!wxDirExists(dft))
					wxMkdir(dft);
				dft += L"/";
#endif
				wstring randname;
				int len = 16;
				char s[17];
				wxString test;
				do {
					const char alphanum[] =
						"0123456789"
						"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
						"abcdefghijklmnopqrstuvwxyz";

					for (int i = 0; i < len; ++i) {
						s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
					}
					s[len] = 0;
					test = dft;
					test += s;
				} while (wxFileExists(test));
				dft += s;

				wstring cfname = dft.ToStdWstring();
				ofstream ofs(ws2s(cfname), ios::binary);
				if (!ofs) return false;
				curl_easy_setopt(s_curl_, CURLOPT_WRITEDATA, &ofs);
				ret = curl_easy_perform(s_curl_);
				bool succeeded = false;
				if (ret == CURLE_OK)
					succeeded = true;

				ofs.close();

				if (succeeded)
				{
					finfo->cached = true;
					finfo->cache_filename = cfname;
					cache_table_[finfo->filename] = cfname;
				}
				else
				{
					if (wxFileExists(cfname)) wxRemoveFile(cfname);
					return false;
				}
*/			}
		}

		ifstream ifs;
		wstring fn = finfo->cached ? finfo->cache_filename : finfo->filename;
		ifs.open(ws2s(fn), ios::binary);
		if (!ifs) return false;
		size_t zsize = finfo->datasize;
		if (zsize <= 0) zsize = (size_t)ifs.seekg(0, std::ios::end).tellg();
		char *zdata = new char[zsize];
		ifs.seekg(finfo->offset, ios_base::beg);
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
