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

#include <Texture.h>
#include <TextureBrick.h>
#include <ShaderProgram.h>
#include <TextureRenderer.h>
#include <Global.h>
#include <MainSettings.h>
#include <DataManager.h>
#include <Ray.h>
#include <Utils.h>
#include <algorithm>
#include <inttypes.h>
#include <glm/gtc/type_ptr.hpp>
#include <Debug.h>

namespace flvr
{
	size_t Texture::mask_undo_num_ = 0;
	Texture::Texture():
	build_max_tex_size_(0),
	brick_size_(0),
	sort_bricks_(true),
	nx_(0),
	ny_(0),
	nz_(0),
	nc_(0),
	nmask_(-1),
	nlabel_(-1),
	vmin_(0.0),
	vmax_(0.0),
	gmin_(0.0),
	gmax_(0.0),
	use_priority_(false),
	n_p0_(0),
	mask_undo_pointer_(-1),
	brkxml_(false),
	pyramid_lv_num_(0),
	pyramid_cur_lv_(0),
	pyramid_cur_fr_(0),
	pyramid_cur_ch_(0),
	pyramid_copy_lv_(-1),
	b_spcx_(1.0),
	b_spcy_(1.0),
	b_spcz_(1.0),
	s_spcx_(1.0),
	s_spcy_(1.0),
	s_spcz_(1.0),
	filetype_(BRICK_FILE_TYPE_NONE),
	filename_(NULL)
	{
		for (size_t i = 0; i < TEXTURE_MAX_COMPONENTS; i++)
		{
			nb_[i] = 0;
			data_[i] = 0;
			ntype_[i] = TYPE_NONE;
		}
		bricks_ = &default_vec_;
	}

	Texture::~Texture()
	{
		if (bricks_)
		{
			for (int i = 0; i<(int)(*bricks_).size(); i++)
			{
				if ((*bricks_)[i])
					delete (*bricks_)[i];
			}
			std::vector<TextureBrick *>().swap(*bricks_);
		}

		clear_undos();

		//release other data
		for (int i=0; i<TEXTURE_MAX_COMPONENTS; i++)
		{
			if (data_[i])
			{
				bool existInPyramid = false;
				for (int j = 0; j < pyramid_.size(); j++)
					if (pyramid_[j].data == data_[i]) existInPyramid = true;

				if (!existInPyramid)
				{
					if (ntype_[i] == TYPE_MASK && mask_undo_num_)
						nrrdNix(data_[i]);
					else
						nrrdNuke(data_[i]);
				}
			}
		}

		clearPyramid();
	}

	std::vector<TextureBrick*>* Texture::get_sorted_bricks(
		fluo::Ray& view, bool is_orthographic)
	{
		if (sort_bricks_)
		{
			for (unsigned int i = 0; i < (*bricks_).size(); i++)
			{
				fluo::Point minp((*bricks_)[i]->bbox().Min());
				fluo::Point maxp((*bricks_)[i]->bbox().Max());
				fluo::Vector diag((*bricks_)[i]->bbox().diagonal());
				minp += diag / 1000.;
				maxp -= diag / 1000.;
				fluo::Point corner[8];
				corner[0] = minp;
				corner[1] = fluo::Point(minp.x(), minp.y(), maxp.z());
				corner[2] = fluo::Point(minp.x(), maxp.y(), minp.z());
				corner[3] = fluo::Point(minp.x(), maxp.y(), maxp.z());
				corner[4] = fluo::Point(maxp.x(), minp.y(), minp.z());
				corner[5] = fluo::Point(maxp.x(), minp.y(), maxp.z());
				corner[6] = fluo::Point(maxp.x(), maxp.y(), minp.z());
				corner[7] = maxp;
				double d = 0.0;
				for (unsigned int c = 0; c < 8; c++)
				{
					double dd;
					if (is_orthographic)
					{
						// orthographic: sort bricks based on distance to the view plane
						dd = Dot(corner[c], view.direction());
					}
					else
					{
						// perspective: sort bricks based on distance to the eye point
						dd = (corner[c] - view.origin()).length();
					}
					if (c == 0 || dd < d) d = dd;
				}
				(*bricks_)[i]->set_d(d);
			}
			if (glbin_settings.m_update_order == 0)
				std::sort((*bricks_).begin(), (*bricks_).end(), TextureBrick::sort_asc);
			else if (glbin_settings.m_update_order == 1)
				std::sort((*bricks_).begin(), (*bricks_).end(), TextureBrick::sort_dsc);

			sort_bricks_ = false;
		}
		return bricks_;
	}

	std::vector<TextureBrick*>* Texture::get_closest_bricks(
		fluo::Point& center, int quota, bool skip,
		fluo::Ray& view, bool is_orthographic)
	{
		if (sort_bricks_)
		{
			quota_bricks_.clear();
			unsigned int i;
			if (quota >= (int64_t)(*bricks_).size())
				quota = int((*bricks_).size());
			else
			{
				for (i=0; i<(*bricks_).size(); i++)
				{
					fluo::Point brick_center = (*bricks_)[i]->bbox().center();
					double d = (brick_center - center).length();
					(*bricks_)[i]->set_d(d);
				}
				std::sort((*bricks_).begin(), (*bricks_).end(), TextureBrick::sort_dsc);
			}

			for (i=0; i<(unsigned int)(*bricks_).size(); i++)
			{
				if (!test_against_view((*bricks_)[i]->bbox(), !is_orthographic))
					continue;
				if (skip)
				{
					if ((*bricks_)[i]->get_priority() == 0)
						quota_bricks_.push_back((*bricks_)[i]);
				}
				else
					quota_bricks_.push_back((*bricks_)[i]);
				if (quota_bricks_.size() == (size_t)quota)
					break;
			}

			for (i = 0; i < quota_bricks_.size(); i++)
			{
				fluo::Point minp(quota_bricks_[i]->bbox().Min());
				fluo::Point maxp(quota_bricks_[i]->bbox().Max());
				fluo::Vector diag(quota_bricks_[i]->bbox().diagonal());
				minp += diag / 1000.;
				maxp -= diag / 1000.;
				fluo::Point corner[8];
				corner[0] = minp;
				corner[1] = fluo::Point(minp.x(), minp.y(), maxp.z());
				corner[2] = fluo::Point(minp.x(), maxp.y(), minp.z());
				corner[3] = fluo::Point(minp.x(), maxp.y(), maxp.z());
				corner[4] = fluo::Point(maxp.x(), minp.y(), minp.z());
				corner[5] = fluo::Point(maxp.x(), minp.y(), maxp.z());
				corner[6] = fluo::Point(maxp.x(), maxp.y(), minp.z());
				corner[7] = maxp;
				double d = 0.0;
				for (unsigned int c = 0; c < 8; c++)
				{
					double dd;
					if (is_orthographic)
					{
						// orthographic: sort bricks based on distance to the view plane
						dd = Dot(corner[c], view.direction());
					}
					else
					{
						// perspective: sort bricks based on distance to the eye point
						dd = (corner[c] - view.origin()).length();
					}
					if (c == 0 || dd < d) d = dd;
				}
				quota_bricks_[i]->set_d(d);
			}
			if (glbin_settings.m_update_order == 0)
				std::sort(quota_bricks_.begin(), quota_bricks_.end(), TextureBrick::sort_asc);
			else if (glbin_settings.m_update_order == 1)
				std::sort(quota_bricks_.begin(), quota_bricks_.end(), TextureBrick::sort_dsc);

			sort_bricks_ = false;
		}

		return &quota_bricks_;
	}

	void Texture::set_matrices(glm::mat4 &mv_mat2, glm::mat4 &proj_mat)
	{
		float mvmat_[16];
		float prmat_[16];
		memcpy(mvmat_, glm::value_ptr(mv_mat2), 16 * sizeof(float));
		memcpy(prmat_, glm::value_ptr(proj_mat), 16 * sizeof(float));
		mv_.set_trans(mvmat_);
		pr_.set_trans(prmat_);
	}
	
	bool Texture::test_against_view(const fluo::BBox &bbox, bool persp)
	{
		if (persp)
		{
			const fluo::Point p0_cam(0.0, 0.0, 0.0);
			fluo::Point p0, p0_obj;
			pr_.unproject(p0_cam, p0);
			mv_.unproject(p0, p0_obj);
			if (bbox.inside(p0_obj))
				return true;
		}

		bool overx = true;
		bool overy = true;
		bool overz = true;
		bool underx = true;
		bool undery = true;
		bool underz = true;
		for (int i = 0; i < 8; i++)
		{
			const fluo::Point pold(
				(i & 1) ? bbox.Min().x() : bbox.Max().x(),
				(i & 2) ? bbox.Min().y() : bbox.Max().y(),
				(i & 4) ? bbox.Min().z() : bbox.Max().z());
			const fluo::Point p = pr_.project(mv_.project(pold));
			overx = overx && (p.x() > 1.0);
			overy = overy && (p.y() > 1.0);
			overz = overz && (p.z() > 1.0);
			underx = underx && (p.x() < -1.0);
			undery = undery && (p.y() < -1.0);
			underz = underz && (p.z() < -1.0);
		}

		return !(overx || overy || overz || underx || undery || underz);
	}

	std::vector<TextureBrick*>* Texture::get_bricks()
	{
		return bricks_;
	}

	//get bricks sorted by id
	std::vector<TextureBrick*>* Texture::get_bricks_id()
	{
		std::sort((*bricks_).begin(), (*bricks_).end(), TextureBrick::sort_id);
		return bricks_;
	}

	std::vector<TextureBrick*>* Texture::get_quota_bricks()
	{
		return &quota_bricks_;
	}

	void Texture::set_spacings(double x, double y, double z)
	{
		if (!brkxml_)
		{
			spcx_ = x;
			spcy_ = y;
			spcz_ = z;
			fluo::Transform tform;
			tform.load_identity();
			fluo::Point nmax(nx_*x, ny_*y, nz_*z);
			tform.pre_scale(fluo::Vector(nmax));
			set_transform(tform);
		}
		else
		{
			s_spcx_ = x / b_spcx_;
			s_spcy_ = y / b_spcy_;
			s_spcz_ = z / b_spcz_;
			fluo::Transform tform;
			tform.load_identity();
			fluo::Point nmax(nx_*spcx_*s_spcx_, ny_*spcy_*s_spcy_, nz_*spcz_*s_spcz_);
			tform.pre_scale(fluo::Vector(nmax));
			set_transform(tform);
		}
	}

	void Texture::get_spacings(double &x, double &y, double &z, int lv)
	{
		if (!brkxml_)
		{
			x = spcx_;
			y = spcy_;
			z = spcz_;
		}
		else if (lv < 0 || lv >= pyramid_lv_num_ || pyramid_.empty())
		{
			x = spcx_ * s_spcx_;
			y = spcy_ * s_spcy_;
			z = spcz_ * s_spcz_;
		}
		else if (pyramid_[lv].data)
		{
			int offset = 0;
			if (pyramid_[lv].data->dim > 3) offset = 1;
			x = pyramid_[lv].data->axis[offset + 0].spacing * s_spcx_;
			y = pyramid_[lv].data->axis[offset + 1].spacing * s_spcy_;
			z = pyramid_[lv].data->axis[offset + 2].spacing * s_spcz_;
		}
	}

	void Texture::set_base_spacings(double x, double y, double z)
	{
		spcx_ = x;
		spcy_ = y;
		spcz_ = z;
		b_spcx_ = x;
		b_spcy_ = y;
		b_spcz_ = z;
		fluo::Transform tform;
		tform.load_identity();
		size_t nx, ny, nz;
		if (brkxml_)
		{
			nx = pyramid_[0].data->axis[0].size;
			ny = pyramid_[0].data->axis[1].size;
			nz = pyramid_[0].data->axis[2].size;
		}
		else
		{
			nx = nx_;
			ny = ny_;
			nz = nz_;
		}
		fluo::Point nmax(nx*x, ny*y, nz*z);
		tform.pre_scale(fluo::Vector(nmax));
		set_transform(tform);
	}

	bool Texture::build(Nrrd* nv_nrrd, Nrrd* gm_nrrd,
		double vmn, double vmx,
		double gmn, double gmx,
		std::vector<flvr::TextureBrick*>* brks)
	{
		size_t axis_size[4];
		nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoSize, axis_size);
		double axis_min[4];
		nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoMin, axis_min);
		double axis_max[4];
		nrrdAxisInfoGet_nva(nv_nrrd, nrrdAxisInfoMax, axis_max);

		int numc = gm_nrrd ? 2 : 1;
		int numb[2];
		if (nv_nrrd)
		{
			if (nv_nrrd->type == nrrdTypeChar ||
				nv_nrrd->type == nrrdTypeUChar)
				numb[0] = 1;
			else
				numb[0] = 2;
		}
		else
			numb[0] = 0;
		numb[1] = gm_nrrd ? 1 : 0;

		fluo::BBox bb(fluo::Point(0,0,0), fluo::Point(1,1,1));

		fluo::Transform tform;
		tform.load_identity();

		size_t dim = nv_nrrd->dim;
		std::vector<int> size(dim);

		int offset = 0;
		if (dim > 3) offset = 1; 

		for (size_t p = 0; p < dim; p++) 
			size[p] = (int)nv_nrrd->axis[p + offset].size;

		if (!brks)
		{
			//when all of them equal to old values, the result should be the same as an old one.  
			if (size[0] != nx() || size[1] != ny() || size[2] != nz() ||
				numc != nc() || numb[0] != nb(0) ||
				bb.Min() != bbox()->Min() ||
				bb.Max() != bbox()->Max() ||
				vmn != vmin() ||
				vmx != vmax() ||
				gmn != gmin() ||
				gmx != gmax())
			{
				clearPyramid();
				bricks_ = &default_vec_;
				build_bricks((*bricks_), size[0], size[1], size[2], numc, numb);
				set_size(size[0], size[1], size[2], numc, numb);
			}
			brkxml_ = false;
		}
		else
		{
			bricks_ = brks;
			set_size(size[0], size[1], size[2], numc, numb);
		}

		set_bbox(bb);
		set_minmax(vmn, vmx, gmn, gmx);
		set_transform(tform);

		n_p0_ = 0;
		if (!brks)
		{
			for (unsigned int i = 0; i < (*bricks_).size(); i++)
			{
				TextureBrick *tb = (*bricks_)[i];
				tb->set_nrrd(nv_nrrd, 0);
				tb->set_nrrd(gm_nrrd, 1);
				if (use_priority_ && !brkxml_)
				{
					tb->set_priority();
					n_p0_ += tb->get_priority() == 0 ? 1 : 0;
				}
			}
		}

		fluo::BBox tempb;
		tempb.extend(transform_.project(bbox_.Min()));
		tempb.extend(transform_.project(bbox_.Max()));
		spcx_ = (tempb.Max().x() - tempb.Min().x()) / double(nx_);
		spcy_ = (tempb.Max().y() - tempb.Min().y()) / double(ny_);
		spcz_ = (tempb.Max().z() - tempb.Min().z()) / double(nz_);

		set_nrrd(nv_nrrd, 0);
		set_nrrd(gm_nrrd, 1);

		return true;
	}

	void Texture::build_bricks(std::vector<TextureBrick*> &bricks, 
		int sz_x, int sz_y, int sz_z,
		int numc, int* numb)
	{
		bool force_pow2 = false;
		if (ShaderProgram::init())
			force_pow2 = !ShaderProgram::texture_non_power_of_two();

		int max_texture_size = 2048;

		if (brick_size_ > 1)
			max_texture_size = brick_size_;
		else
		{
			if (ShaderProgram::init())
				max_texture_size = ShaderProgram::max_texture_size();

			//further determine the max texture size
			double data_size = double(sz_x)*double(sz_y)*double(sz_z)/1.04e6;
			glbin_data_manager.UpdateStreamMode(data_size);
			if (glbin_settings.m_mem_swap)
				max_texture_size = glbin_settings.m_force_brick_size;
		}

		if (max_texture_size > 1)
			build_max_tex_size_ = max_texture_size;
		else
			max_texture_size = 2048;

		// Initial brick size
		int bsize[3];

		if (force_pow2)
		{
			if (fluo::Pow2(sz_x) > (unsigned)sz_x)
				bsize[0] = fluo::Min(int(fluo::Pow2(sz_x))/2, max_texture_size);
			if (fluo::Pow2(sz_y) > (unsigned)sz_y)
				bsize[1] = fluo::Min(int(fluo::Pow2(sz_x))/2, max_texture_size);
			if (fluo::Pow2(sz_z) > (unsigned)sz_z)
				bsize[2] = fluo::Min(int(fluo::Pow2(sz_x))/2, max_texture_size);
		}
		else
		{
			bsize[0] = fluo::Min(int(fluo::Pow2(sz_x)), max_texture_size);
			bsize[1] = fluo::Min(int(fluo::Pow2(sz_y)), max_texture_size);
			bsize[2] = fluo::Min(int(fluo::Pow2(sz_z)), max_texture_size);
		}

		bszx_ = bsize[0]; bszy_ = bsize[1]; bszz_ = bsize[2];
		bnx_ = bszx_>1?((sz_x-1) / (bszx_-1) + (((sz_x-1) % (bszx_-1)) ? 1 : 0)):1;
		bny_ = bszy_>1?((sz_y-1) / (bszy_-1) + (((sz_y-1) % (bszy_-1)) ? 1 : 0)):1;
		bnz_ = bszz_>1?((sz_z-1) / (bszz_-1) + (((sz_z-1) % (bszz_-1)) ? 1 : 0)):1;

		bricks.clear();

		int i, j, k;
		int mx, my, mz, mx2, my2, mz2, ox, oy, oz;
		double tx0, ty0, tz0, tx1, ty1, tz1;
		double bx1, by1, bz1;
		double dx0, dy0, dz0, dx1, dy1, dz1;
		for (k = 0; k < sz_z; k += bsize[2])
		{
			if (k) k--;
			for (j = 0; j < sz_y; j += bsize[1])
			{
				if (j) j--;
				for (i = 0; i < sz_x; i += bsize[0])
				{
					if (i) i--;
					mx = fluo::Min(bsize[0], sz_x - i);
					my = fluo::Min(bsize[1], sz_y - j);
					mz = fluo::Min(bsize[2], sz_z - k);

					mx2 = mx;
					my2 = my;
					mz2 = mz;
					if (force_pow2)
					{
						mx2 = fluo::Pow2(mx);
						my2 = fluo::Pow2(my);
						mz2 = fluo::Pow2(mz);
					}

					// Compute Texture Box.
					tx0 = i?((mx2 - mx + 0.5) / mx2): 0.0;
					ty0 = j?((my2 - my + 0.5) / my2): 0.0;
					tz0 = k?((mz2 - mz + 0.5) / mz2): 0.0;

					tx1 = 1.0 - 0.5 / mx2;
					if (mx < bsize[0]) tx1 = 1.0;
					if (sz_x - i == bsize[0]) tx1 = 1.0;

					ty1 = 1.0 - 0.5 / my2;
					if (my < bsize[1]) ty1 = 1.0;
					if (sz_y - j == bsize[1]) ty1 = 1.0;

					tz1 = 1.0 - 0.5 / mz2;
					if (mz < bsize[2]) tz1 = 1.0;
					if (sz_z - k == bsize[2]) tz1 = 1.0;

					fluo::BBox tbox(fluo::Point(tx0, ty0, tz0), fluo::Point(tx1, ty1, tz1));

					// Compute BBox.
					bx1 = fluo::Min((i + bsize[0] - 0.5) / (double)sz_x, 1.0);
					if (sz_x - i == bsize[0]) bx1 = 1.0;

					by1 = fluo::Min((j + bsize[1] - 0.5) / (double)sz_y, 1.0);
					if (sz_y - j == bsize[1]) by1 = 1.0;

					bz1 = fluo::Min((k + bsize[2] - 0.5) / (double)sz_z, 1.0);
					if (sz_z - k == bsize[2]) bz1 = 1.0;

					fluo::BBox bbox(fluo::Point(i==0?0:(i+0.5) / (double)sz_x,
						j==0?0:(j+0.5) / (double)sz_y,
						k==0?0:(k+0.5) / (double)sz_z),
						fluo::Point(bx1, by1, bz1));

					ox = i - (mx2 - mx);
					oy = j - (my2 - my);
					oz = k - (mz2 - mz);
					dx0 = (double)ox / sz_x;
					dy0 = (double)oy / sz_y;
					dz0 = (double)oz / sz_z;
					dx1 = (double)(ox + mx2) / sz_x;
					dy1 = (double)(oy + my2) / sz_y;
					dz1 = (double)(oz + mz2) / sz_z;

					fluo::BBox dbox(fluo::Point(dx0, dy0, dz0), fluo::Point(dx1, dy1, dz1));
					TextureBrick *b = new TextureBrick(0, 0, mx2, my2, mz2, numc, numb,
						ox, oy, oz, mx2, my2, mz2, bbox, tbox, dbox, static_cast<unsigned int>(bricks.size()));
					bricks.push_back(b);
				}
			}
		}
	}

	//! Interface that does not expose flvr::BBox.
	void Texture::get_bounds(double &xmin, double &ymin, double &zmin,
		double &xmax, double &ymax, double &zmax) const 
	{
		fluo::BBox b;
		get_bounds(b);
		xmin = b.Min().x();
		ymin = b.Min().y();
		zmin = b.Min().z();

		xmax = b.Max().x();
		ymax = b.Max().y();
		zmax = b.Max().z();
	}

	void Texture::get_bounds(fluo::BBox &b) const
	{
		b.extend(transform_.project(bbox_.Min()));
		b.extend(transform_.project(bbox_.Max()));
	}

	//add one more texture component as the volume mask
	bool Texture::add_empty_mask()
	{
		if (nc_>0 && nc_<=2 && nmask_==-1)
		{
			//fix to texture2
			nmask_ = 2;
			nb_[nmask_] = 1;
			ntype_[nmask_] = TYPE_MASK;

			int i;
			for (i=0; i<(int)(*bricks_).size(); i++)
			{
				(*bricks_)[i]->nmask(nmask_);
				(*bricks_)[i]->nb(1, nmask_);
				(*bricks_)[i]->ntype(TextureBrick::TYPE_MASK, nmask_);
			}
			return true;
		}
		else
			return false;
	}

	//add one more texture component as the labeling volume
	bool Texture::add_empty_label()
	{
		if (nc_>0 && nc_<=2 && nlabel_==-1)
		{
			if (nmask_==-1)	//no mask
				nlabel_ = 3;
			else			//label is after mask
				nlabel_ = nmask_+1;
			nb_[nlabel_] = 4;

			int i;
			for (i=0; i<(int)(*bricks_).size(); i++)
			{
				(*bricks_)[i]->nlabel(nlabel_);
				(*bricks_)[i]->nb(4, nlabel_);
				(*bricks_)[i]->ntype(TextureBrick::TYPE_LABEL, nlabel_);
			}
			return true;
		}
		else
			return false;
	}

	void Texture::deact_all_mask()
	{
		for (size_t i = 0; i < bricks_->size(); ++i)
			(*bricks_)[i]->deact_mask();
	}

	void Texture::act_all_mask()
	{
		for (size_t i = 0; i < bricks_->size(); ++i)
			(*bricks_)[i]->act_mask();
	}

	void Texture::invalid_all_mask()
	{
		for (size_t i = 0; i < bricks_->size(); ++i)
			(*bricks_)[i]->invalid_mask();
	}

	void Texture::valid_all_mask()
	{
		for (size_t i = 0; i < bricks_->size(); ++i)
			(*bricks_)[i]->valid_mask();
	}

	int Texture::GetLevelNum()
	{
		return static_cast<int>(pyramid_.size());
	}

	TextureBrick* Texture::get_brick(unsigned int bid)
	{
		for (size_t i=0; i<(*bricks_).size(); ++i)
		{
			if ((*bricks_)[i]->get_id() == bid)
				return (*bricks_)[i];
		}
		return 0;
	}

	bool Texture::buildPyramid(std::vector<Pyramid_Level> &pyramid, std::vector<std::vector<std::vector<std::vector<FileLocInfo *>>>> &filenames, bool useURL)
	{
		if (pyramid.empty()) return false;
		if (pyramid.size() != filenames.size()) return false;

		brkxml_ = true;
		//useURL_ = useURL;

		clearPyramid();

		pyramid_lv_num_ = static_cast<int>(pyramid.size());
		pyramid_ = pyramid;
		filenames_ = filenames;
		for (int i = 0; i < pyramid_.size(); i++)
		{
			if (!pyramid_[i].data || pyramid_[i].bricks.empty())
			{
				clearPyramid();
				return false;
			}

			for (int j = 0; j < pyramid_[i].bricks.size(); j++)
			{
				pyramid_[i].bricks[j]->set_nrrd(pyramid_[i].data, 0);
				pyramid_[i].bricks[j]->set_nrrd(0, 1);
			}
		}
		setLevel(pyramid_lv_num_ - 1);
		pyramid_cur_lv_ = -1;

		return true;
	}

	void Texture::clearPyramid()
	{
		if (!brkxml_) return;

		if (pyramid_.empty()) return;

		for (int i = 0; i<(int)pyramid_.size(); i++)
		{
			for (int j = 0; j<(int)pyramid_[i].bricks.size(); j++)
			{
				if (pyramid_[i].bricks[j]) delete pyramid_[i].bricks[j];
			}
			std::vector<TextureBrick *>().swap(pyramid_[i].bricks);
			nrrdNix(pyramid_[i].data);
		}
		std::vector<Pyramid_Level>().swap(pyramid_);

		for (int i = 0; i<(int)filenames_.size(); i++)
			for (int j = 0; j<(int)filenames_[i].size(); j++)
				for (int k = 0; k<(int)filenames_[i][j].size(); k++)
					for (int n = 0; n<(int)filenames_[i][j][k].size(); n++)
						if (filenames_[i][j][k][n]) delete filenames_[i][j][k][n];

		std::vector<std::vector<std::vector<std::vector<FileLocInfo *>>>>().swap(filenames_);

		pyramid_lv_num_ = 0;
		pyramid_cur_lv_ = -1;
	}

	void Texture::setLevel(int lv)
	{
		if (lv < 0 || lv >= pyramid_lv_num_ || !brkxml_ || pyramid_cur_lv_ == lv) return;
		pyramid_cur_lv_ = lv;
		build(pyramid_[pyramid_cur_lv_].data, 0, 0, 256, 0, 0, &pyramid_[pyramid_cur_lv_].bricks);
		set_data_file(pyramid_[pyramid_cur_lv_].filenames, pyramid_[pyramid_cur_lv_].filetype);

		int offset = 0;
		if (pyramid_[lv].data->dim > 3) offset = 1;
		spcx_ = pyramid_[lv].data->axis[offset + 0].spacing;
		spcy_ = pyramid_[lv].data->axis[offset + 1].spacing;
		spcz_ = pyramid_[lv].data->axis[offset + 2].spacing;
		fluo::Transform tform;
		tform.load_identity();
		fluo::Point nmax(nx_*spcx_*s_spcx_, ny_*spcy_*s_spcy_, nz_*spcz_*s_spcz_);
		tform.pre_scale(fluo::Vector(nmax));
		set_transform(tform);
		//additional information
		nx_ = pyramid_[lv].szx;
		ny_ = pyramid_[lv].szy;
		nz_ = pyramid_[lv].szz;
		bszx_ = pyramid_[lv].bszx;
		bszy_ = pyramid_[lv].bszy;
		bszz_ = pyramid_[lv].bszz;
		bnx_ = pyramid_[lv].bnx;
		bny_ = pyramid_[lv].bny;
		bnz_ = pyramid_[lv].bnz;
	}

	void Texture::set_data_file(std::vector<FileLocInfo *> *fname, int type)
	{
		filename_ = fname;
		filetype_ = type;
	}

	FileLocInfo *Texture::GetFileName(int id)
	{
		if (id < 0 || !filename_ || id >= filename_->size()) return NULL;
		return (*filename_)[id];
	}

	void Texture::set_FrameAndChannel(int fr, int ch)
	{
		if (!brkxml_) return;

		pyramid_cur_fr_ = fr;
		pyramid_cur_ch_ = ch;

		int lv = pyramid_cur_lv_;

		for (int i = 0; i < pyramid_.size(); i++)
		{
			if (fr < 0 || fr >= filenames_[i].size()) continue;
			if (ch < 0 || ch >= filenames_[i][fr].size()) continue;
			pyramid_[i].filenames = &filenames_[i][fr][ch];
		}
		if (lv == pyramid_cur_lv_) set_data_file(pyramid_[lv].filenames, pyramid_[lv].filetype);

	}

	//set nrrd
	void Texture::set_nrrd(Nrrd* data, int index)
	{
		if (index>=0&&index<TEXTURE_MAX_COMPONENTS)
		{
			bool existInPyramid = false;
			for (int i = 0; i < pyramid_.size(); i++)
				if (pyramid_[i].data == data) existInPyramid = true;

			if (data_[index] && data && !existInPyramid)
			{
				if (index == nmask_ && mask_undo_num_)
					nrrdNix(data_[index]);
				else
					nrrdNuke(data_[index]);
			}

			data_[index] = data;
			if (!existInPyramid)
			{
				for (int i = 0; i < (int)(*bricks_).size(); i++)
					(*bricks_)[i]->set_nrrd(data, index);

				//add to undo list
				if (index == nmask_)
					set_mask(data->data);
			}
		}
	}

	Nrrd* Texture::get_nrrd(int index)
	{
		if (index>=0&&index<TEXTURE_MAX_COMPONENTS)
			return data_[index];
		else return 0;
	}

	//mask undo management
	bool Texture::trim_mask_undos_head()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return true;
		if (mask_undos_.size() <= mask_undo_num_+1)
			return true;
		if (mask_undo_pointer_ == 0)
			return false;
		while (mask_undos_.size()>mask_undo_num_+1 &&
			mask_undo_pointer_>0 &&
			mask_undo_pointer_<mask_undos_.size())
		{
			delete[] (unsigned char*)(mask_undos_.front());
			mask_undos_.erase(mask_undos_.begin());
			mask_undo_pointer_--;
		}
		return true;
	}

	bool Texture::trim_mask_undos_tail()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return true;
		if (mask_undos_.size() <= mask_undo_num_+1)
			return true;
		if (mask_undo_pointer_ == mask_undos_.size()-1)
			return false;
		while (mask_undos_.size()>mask_undo_num_+1 &&
			mask_undo_pointer_>=0 &&
			mask_undo_pointer_<mask_undos_.size()-1)
		{
			delete[] (unsigned char*)(mask_undos_.back());
			mask_undos_.pop_back();
		}
		return true;
	}

	bool Texture::get_undo()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return false;
		if (mask_undo_pointer_ <= 0)
			return false;
		return true;
	}

	bool Texture::get_redo()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return false;
		if (mask_undo_pointer_ >= mask_undos_.size()-1)
			return false;
		return true;
	}

	void Texture::set_mask(void* mask_data)
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return;

		if (mask_undo_pointer_>-1 &&
			mask_undo_pointer_<mask_undos_.size()-1)
		{
			mask_undos_.insert(
				mask_undos_.begin()+mask_undo_pointer_+1,
				mask_data);
			mask_undo_pointer_++;
			if (!trim_mask_undos_head())
				trim_mask_undos_tail();
		}
		else
		{
			mask_undos_.push_back(mask_data);
			mask_undo_pointer_ = static_cast<int>(mask_undos_.size())-1;
			trim_mask_undos_head();
		}
	}

	void Texture::push_mask()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return;
		if (mask_undo_pointer_<0 ||
			mask_undo_pointer_>mask_undos_.size()-1)
			return;

		//duplicate at pointer position
		unsigned long long mem_size = (unsigned long long)nx_*
			(unsigned long long)ny_*(unsigned long long)nz_;
		void* new_data = (void*)new (std::nothrow) unsigned char[mem_size];
		memcpy(new_data, mask_undos_[mask_undo_pointer_], size_t(mem_size));
		if (mask_undo_pointer_<mask_undos_.size()-1)
		{
			mask_undos_.insert(
				mask_undos_.begin()+mask_undo_pointer_+1,
				new_data);
			mask_undo_pointer_++;
			if (!trim_mask_undos_head())
				trim_mask_undos_tail();
		}
		else
		{
			mask_undos_.push_back(new_data);
			mask_undo_pointer_++;
			trim_mask_undos_head();
		}

//#ifdef _DEBUG
//		DBMIUINT8 img;
//		img.nx = nx_; img.ny = ny_; img.nc = 1; img.nt = img.nx;
//		img.data = (unsigned char*)(mask_undos_[0]);
//		DBMIUINT8 img2;
//		img2.nx = nx_; img2.ny = ny_; img2.nc = 1; img2.nt = img2.nx;
//		img2.data = (unsigned char*)(mask_undos_[1]);
//#endif
		//update mask data
		nrrdWrap_va(data_[nmask_],
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)nx_,
			(size_t)ny_, (size_t)nz_);
	}

	void Texture::pop_mask()
	{
		if (nmask_ <= -1 || mask_undo_num_ == 0)
			return;
		if (mask_undo_pointer_ <= 0 ||
			mask_undo_pointer_ > mask_undos_.size() - 1)
			return;

		delete[](unsigned char*)(mask_undos_.back());
		mask_undos_.pop_back();
		mask_undo_pointer_--;

		//update mask data
		nrrdWrap_va(data_[nmask_],
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)nx_,
			(size_t)ny_, (size_t)nz_);
	}

	void Texture:: mask_undos_backward()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return;
		if (mask_undo_pointer_<=0 ||
			mask_undo_pointer_>mask_undos_.size()-1)
			return;

		//move pointer
		mask_undo_pointer_--;

		//update mask data
		nrrdWrap_va(data_[nmask_],
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)nx_,
			(size_t)ny_, (size_t)nz_);
	}

	void Texture::mask_undos_forward()
	{
		if (nmask_<=-1 || mask_undo_num_==0)
			return;
		if (mask_undo_pointer_<0 ||
			mask_undo_pointer_>mask_undos_.size()-2)
			return;

		//move pointer
		mask_undo_pointer_++;

		//update mask data
		nrrdWrap_va(data_[nmask_],
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)nx_,
			(size_t)ny_, (size_t)nz_);
	}

	void Texture::clear_undos()
	{
		//mask data now managed by the undos
		for (size_t i=0; i<mask_undos_.size(); ++i)
		{
			if (mask_undos_[i])
				delete[] (unsigned char*)(mask_undos_[i]);
		}
		mask_undos_.clear();
		mask_undo_pointer_ = -1;
	}

	unsigned int Texture::negxid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		if (x == 0)
			return id;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + y * bnx_ + x - 1;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	unsigned int Texture::negyid(unsigned int id)
	{
		int y = (id % (bnx_ * bny_)) / bnx_;
		if (y == 0)
			return id;
		int x = (id % (bnx_ * bny_)) % bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + (y - 1) * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	unsigned int Texture::negzid(unsigned int id)
	{
		int z = id / (bnx_ * bny_);
		if (z == 0)
			return id;
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int r = (z - 1) * bnx_ * bny_ + y * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	unsigned int Texture::posxid(unsigned int id)
	{
		int x = (id % (bnx_ * bny_)) % bnx_;
		if (x == bnx_ - 1)
			return id;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + y * bnx_ + x + 1;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	unsigned int Texture::posyid(unsigned int id)
	{
		int y = (id % (bnx_ * bny_)) / bnx_;
		if (y == bny_ - 1)
			return id;
		int x = (id % (bnx_ * bny_)) % bnx_;
		int z = id / (bnx_ * bny_);
		int r = z * bnx_ * bny_ + (y + 1) * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	unsigned int Texture::poszid(unsigned int id)
	{
		int z = id / (bnx_ * bny_);
		if (z == bnz_ - 1)
			return id;
		int x = (id % (bnx_ * bny_)) % bnx_;
		int y = (id % (bnx_ * bny_)) / bnx_;
		int r = (z + 1) * bnx_ * bny_ + y * bnx_ + x;
		if (r < 0 || r >= bnx_ * bny_ * bnz_)
			return id;
		else
			return r;
	}

	unsigned int Texture::get_brick_id(unsigned long long index)
	{
		unsigned long long x, y, z;
		z = index / (nx_ * ny_);
		y = index % (nx_ * ny_);
		x = y % nx_;
		y = y / nx_;
		//get brick indices
		x = bszx_ <= 1 ? 0 : x / (bszx_-1);
		y = bszy_ <= 1 ? 0 : y / (bszy_-1);
		z = bszz_ <= 1 ? 0 : z / (bszz_-1);
		return static_cast<unsigned int>(z * bnx_ * bny_ + y * bnx_ + x);
	}
} // namespace flvr
