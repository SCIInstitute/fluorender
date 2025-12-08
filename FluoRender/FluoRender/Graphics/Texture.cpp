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
		brick_planned_size_(0),
		sort_bricks_(true),
		res_(0.0),
		brick_res_(0.0),
		brick_num_(0.0),
		vmin_(0.0),
		vmax_(0.0),
		use_priority_(false),
		n_p0_(0),
		mask_undo_pointer_(-1),
		brkxml_(false),
		pyramid_lv_num_(0),
		pyramid_cur_lv_(0),
		pyramid_cur_fr_(0),
		pyramid_cur_ch_(0),
		pyramid_copy_lv_(-1),
		spacing_(1.0),
		base_spacing_(1.0),
		spacing_scale_(1.0),
		filetype_(BRICK_FILE_TYPE_NONE),
		filename_(NULL)
	{
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
		for (auto& it : data_)
		{
			Nrrd* nrrd = it.second.data;
			auto type = it.second.type;
			if (nrrd)
			{
				bool existInPyramid = false;
				for (int j = 0; j < pyramid_.size(); j++)
					if (pyramid_[j].data == nrrd) existInPyramid = true;

				if (!existInPyramid)
				{
					if (type == CompType::Mask && mask_undo_num_)
						nrrdNix(nrrd);
					else
						nrrdNuke(nrrd);
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

	void Texture::set_spacing(const fluo::Vector& spc)
	{
		if (!brkxml_)
		{
			spacing_ = spc;
			fluo::Transform tform;
			tform.load_identity();
			fluo::Vector nmax(res_ * spacing_);
			tform.pre_scale(nmax);
			set_transform(tform);
		}
		else
		{
			spacing_scale_ = spacing_ / base_spacing_;
			fluo::Transform tform;
			tform.load_identity();
			fluo::Vector nmax(res_* spacing_* spacing_scale_);
			tform.pre_scale(fluo::Vector(nmax));
			set_transform(tform);
		}
	}

	fluo::Vector Texture::get_spacing(int lv)
	{
		if (!brkxml_)
		{
			return spacing_;
		}
		else if (lv < 0 || lv >= pyramid_lv_num_ || pyramid_.empty())
		{
			return spacing_ * spacing_scale_;
		}
		else if (pyramid_[lv].data)
		{
			int offset = 0;
			if (pyramid_[lv].data->dim > 3) offset = 1;
			fluo::Vector spc(
				pyramid_[lv].data->axis[offset + 0].spacing,
				pyramid_[lv].data->axis[offset + 1].spacing,
				pyramid_[lv].data->axis[offset + 2].spacing);
			return spc * spacing_scale_;
		}
		return spacing_;
	}

	void Texture::set_base_spacing(const fluo::Vector& spc)
	{
		base_spacing_ = spacing_ = spc;

		fluo::Transform tform;
		tform.load_identity();
		fluo::Vector res;
		if (brkxml_)
		{
			res = fluo::Vector(
				pyramid_[0].data->axis[0].size,
				pyramid_[0].data->axis[1].size,
				pyramid_[0].data->axis[2].size);
		}
		else
		{
			res = res_;
		}
		fluo::Vector nmax = res * spacing_;
		tform.pre_scale(fluo::Vector(nmax));
		set_transform(tform);
	}

	bool Texture::build(Nrrd* nv_nrrd,
		double vmn, double vmx,
		std::vector<flvr::TextureBrick*>* brks)
	{
		if (!nv_nrrd)
			return false;

		int bytes = 0;
		switch (nv_nrrd->type)
		{
		case nrrdTypeChar:
		case nrrdTypeUChar:
			bytes = 1;
			break;
		case nrrdTypeShort:
		case nrrdTypeUShort:
			bytes = 2;
			break;
		case nrrdTypeInt:
		case nrrdTypeUInt:
			bytes = 4;
			break;
		}
		fluo::BBox bb(fluo::Point(0,0,0), fluo::Point(1,1,1));

		fluo::Transform tform;
		tform.load_identity();

		size_t dim = nv_nrrd->dim;
		fluo::Vector size;

		int offset = 0;
		if (dim > 3) offset = 1;

		for (size_t p = 0; p < dim; p++) 
			size[p] = (int)nv_nrrd->axis[p + offset].size;

		TexComp comp = { CompType::Data, bytes, nv_nrrd };
		set_nrrd(CompType::Data, comp);

		if (!brks)
		{
			//when all of them equal to old values, the result should be the same as an old one.  
			if (size != res_ ||
				bytes != nb(CompType::Data) ||
				bb.Min() != bbox()->Min() ||
				bb.Max() != bbox()->Max() ||
				vmn != vmin() ||
				vmx != vmax())
			{
				clearPyramid();
				bricks_ = &default_vec_;
				build_bricks((*bricks_), size, bytes);
				set_res(size);
				nb(CompType::Data, bytes);
			}
			brkxml_ = false;
		}
		else
		{
			bricks_ = brks;
			set_res(size);
			nb(CompType::Data, bytes);
		}

		set_bbox(bb);
		set_minmax(vmn, vmx);
		set_transform(tform);

		n_p0_ = 0;
		if (!brks)
		{
			for (unsigned int i = 0; i < (*bricks_).size(); i++)
			{
				TextureBrick *tb = (*bricks_)[i];
				tb->set_nrrd(CompType::Data, comp);
				tb->set_stride(size);
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
		spacing_ = tempb.diagonal() / res_;

		return true;
	}

	void Texture::build_bricks(std::vector<TextureBrick*> &bricks, 
		const fluo::Vector& size, int bytes)
	{
		bool force_pow2 = false;
		if (ShaderProgram::init())
			force_pow2 = !ShaderProgram::texture_non_power_of_two();

		int max_texture_size = 2048;

		if (brick_planned_size_ > 1)
			max_texture_size = brick_planned_size_;
		else
		{
			if (ShaderProgram::init())
				max_texture_size = ShaderProgram::max_texture_size();

			//further determine the max texture size
			double data_size = size.x()*size.y()*size.z()/1.04e6;
			glbin_data_manager.UpdateStreamMode(data_size);
			if (glbin_settings.m_mem_swap)
				max_texture_size = glbin_settings.m_force_brick_size;
		}

		if (max_texture_size > 1)
			build_max_tex_size_ = max_texture_size;
		else
			max_texture_size = 2048;

		// Initial brick size
		fluo::Vector bsize, off_size;
		fluo::Vector pow_size = fluo::Pow2(size);

		if (force_pow2)
		{
			if (pow_size.intx() > size.intx())
				bsize.x(std::min(pow_size.intx()/2, max_texture_size));
			if (pow_size.inty() > size.inty())
				bsize.y(std::min(pow_size.inty()/2, max_texture_size));
			if (pow_size.intz() > size.intz())
				bsize.z(std::min(pow_size.intz()/2, max_texture_size));
		}
		else
		{
			bsize.x(std::min(pow_size.intx(), max_texture_size));
			bsize.y(std::min(pow_size.inty(), max_texture_size));
			bsize.z(std::min(pow_size.intz(), max_texture_size));
		}

		brick_res_ = bsize;
		auto brickCount1D = [](int size, int bsize) {
			if (bsize > 1) {
				int span = size - 1;
				int step = bsize - 1;
				return span / step + (span % step ? 1 : 0);
			}
			return 1;
		};

		brick_num_ = fluo::Vector(
			brickCount1D(size.intx(), bsize.intx()),
			brickCount1D(size.inty(), bsize.inty()),
			brickCount1D(size.intz(), bsize.intz())
		);

		bricks.clear();

		int i, j, k;
		fluo::Vector mxyz, mxyz2;
		fluo::Point t0, t1;
		fluo::Point b0, b1;
		fluo::Point d0, d1;

		for (k = 0; k < size.intz(); k += bsize.intz())
		{
			if (k) k--;
			for (j = 0; j < size.inty(); j += bsize.inty())
			{
				if (j) j--;
				for (i = 0; i < size.intx(); i += bsize.intx())
				{
					if (i) i--;
					mxyz2 = mxyz = fluo::Min(bsize, size - fluo::Vector(i, j, k));

					if (force_pow2)
					{
						mxyz2 = fluo::Pow2(mxyz);
					}

					// Compute Texture Box.
					t0 = fluo::Point((mxyz2 - mxyz + fluo::Vector(0.5)) / mxyz2);
					if (i == 0) t0.x(0.0);
					if (j == 0) t0.y(0.0);
					if (k == 0) t0.z(0.0);

					t1 = fluo::Point(fluo::Vector(1.0) - fluo::Vector(0.5) / mxyz2);
					if (mxyz.intx() < bsize.intx() ||
						size.intx() - i == bsize.intx())
						t1.x(1.0);
					if (mxyz.inty() < bsize.inty() ||
						size.inty() - j == bsize.inty())
						t1.y(1.0);
					if (mxyz.intz() < bsize.intz() ||
						size.intz() - k == bsize.intz())
						t1.z(1.0);

					fluo::BBox tbox(t0, t1);

					// Compute BBox.
					b0 = fluo::Point((fluo::Vector(i, j, k) + fluo::Vector(0.5)) / size);
					if (i == 0)
						b0.x(0.0);
					if (j == 0)
						b0.y(0.0);
					if (k == 0)
						b0.z(0.0);
					b1 = fluo::Point(fluo::Min((fluo::Vector(i, j, k) + bsize - fluo::Vector(0.5) / size), fluo::Vector(1.0)));
					if (size.intx() - i == bsize.intx())
						b1.x(1.0);
					if (size.inty() - j == bsize.inty())
						b1.y(1.0);
					if (size.intz() - k == bsize.intz())
						b1.z(1.0);

					fluo::BBox bbox(b0, b1);

					off_size = fluo::Vector(i, j, k) - mxyz2 + mxyz;
					d0 = fluo::Point(off_size / size);
					d1 = fluo::Point((off_size + mxyz2) / size);

					fluo::BBox dbox(d0, d1);
					TextureBrick *b = new TextureBrick(nullptr,
						mxyz2, bytes,
						off_size,
						mxyz2,
						bbox,
						tbox,
						dbox,
						static_cast<unsigned int>(bricks.size()));
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
		auto c = data_.find(CompType::Mask);
		if (c != data_.end())
			return false;
		TexComp comp = { CompType::Mask, 1, nullptr };
		data_[CompType::Mask] = comp;
		for (size_t i = 0; i < (*bricks_).size(); ++i)
		{
			(*bricks_)[i]->set_nrrd(CompType::Mask, comp);
		}
		return true;
	}

	//add one more texture component as the labeling volume
	bool Texture::add_empty_label()
	{
		auto c = data_.find(CompType::Label);
		if (c != data_.end())
			return false;
		TexComp comp = { CompType::Label, 4, nullptr };
		data_[CompType::Label] = comp;
		for (size_t i = 0; i < (*bricks_).size(); ++i)
		{
			(*bricks_)[i]->set_nrrd(CompType::Label, comp);
		}
		return true;
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

		int bytes = nb(CompType::Data);

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

			TexComp comp = { CompType::Data, bytes, pyramid_[i].data };
			for (int j = 0; j < pyramid_[i].bricks.size(); j++)
			{
				pyramid_[i].bricks[j]->set_nrrd(CompType::Data, comp);
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
		build(pyramid_[pyramid_cur_lv_].data, 0, 256, &pyramid_[pyramid_cur_lv_].bricks);
		set_data_file(pyramid_[pyramid_cur_lv_].filenames, pyramid_[pyramid_cur_lv_].filetype);

		int offset = 0;
		if (pyramid_[lv].data->dim > 3) offset = 1;
		spacing_ = fluo::Vector(
			pyramid_[lv].data->axis[offset + 0].spacing,
			pyramid_[lv].data->axis[offset + 1].spacing,
			pyramid_[lv].data->axis[offset + 2].spacing);
		fluo::Transform tform;
		tform.load_identity();
		fluo::Vector nmax = res_ * spacing_ * spacing_scale_;
		tform.pre_scale(fluo::Vector(nmax));
		set_transform(tform);
		//additional information
		res_ = fluo::Vector(
			pyramid_[lv].szx,
			pyramid_[lv].szy,
			pyramid_[lv].szz);
		brick_res_ = fluo::Vector(
			pyramid_[lv].bszx,
			pyramid_[lv].bszy,
			pyramid_[lv].bszz);
		brick_num_ = fluo::Vector(
			pyramid_[lv].bnx,
			pyramid_[lv].bny,
			pyramid_[lv].bnz);
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
	void Texture::set_nrrd(CompType type, TexComp comp)
	{
		Nrrd* nrrd = comp.data;
		if (!nrrd)
			return;

		bool existInPyramid = false;
		for (int i = 0; i < pyramid_.size(); i++)
			if (pyramid_[i].data == nrrd)
				existInPyramid = true;

		auto c = data_.find(type);
		if (c != data_.end() && !existInPyramid)
		{
			if (type == CompType::Mask && mask_undo_num_)
				nrrdNix(c->second.data);
			else
				nrrdNuke(c->second.data);
		}
		data_[type] = comp;

		if (!existInPyramid)
		{
			for (int i = 0; i < (int)(*bricks_).size(); i++)
				(*bricks_)[i]->set_nrrd(type, comp);

			//add to undo list
			if (type == CompType::Mask)
				set_mask(nrrd->data);
		}
	}

	TexComp Texture::get_nrrd(CompType type)
	{
		auto c = data_.find(type);
		if (c == data_.end())
			return TexComp();
		return c->second;
	}

	//mask undo management
	bool Texture::trim_mask_undos_head()
	{
		if (mask_undo_num_==0)
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
		if (mask_undo_num_==0)
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
		if (mask_undo_num_==0)
			return false;
		if (mask_undo_pointer_ <= 0)
			return false;
		return true;
	}

	bool Texture::get_redo()
	{
		if (mask_undo_num_==0)
			return false;
		if (mask_undo_pointer_ >= mask_undos_.size()-1)
			return false;
		return true;
	}

	void Texture::set_mask(void* mask_data)
	{
		if (mask_undo_num_==0)
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
		if (mask_undo_num_==0)
			return;
		if (mask_undo_pointer_<0 ||
			mask_undo_pointer_>mask_undos_.size()-1)
			return;

		//duplicate at pointer position
		unsigned long long mem_size = (unsigned long long)res_.intx()*
			(unsigned long long)res_.inty()*(unsigned long long)res_.intz();
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
		auto c = data_.find(CompType::Mask);
		if (c == data_.end())
			return;
		Nrrd* nrrd = c->second.data;
		nrrdWrap_va(nrrd,
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)res_.intx(),
			(size_t)res_.inty(), (size_t)res_.intz());
	}

	void Texture::pop_mask()
	{
		if (mask_undo_num_ == 0)
			return;
		if (mask_undo_pointer_ <= 0 ||
			mask_undo_pointer_ > mask_undos_.size() - 1)
			return;

		delete[](unsigned char*)(mask_undos_.back());
		mask_undos_.pop_back();
		mask_undo_pointer_--;

		//update mask data
		auto c = data_.find(CompType::Mask);
		if (c == data_.end())
			return;
		Nrrd* nrrd = c->second.data;
		nrrdWrap_va(nrrd,
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)res_.intx(),
			(size_t)res_.inty(), (size_t)res_.intz());
	}

	void Texture:: mask_undos_backward()
	{
		if (mask_undo_num_==0)
			return;
		if (mask_undo_pointer_<=0 ||
			mask_undo_pointer_>mask_undos_.size()-1)
			return;

		//move pointer
		mask_undo_pointer_--;

		//update mask data
		auto c = data_.find(CompType::Mask);
		if (c == data_.end())
			return;
		Nrrd* nrrd = c->second.data;
		nrrdWrap_va(nrrd,
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)res_.intx(),
			(size_t)res_.inty(), (size_t)res_.intz());
	}

	void Texture::mask_undos_forward()
	{
		if (mask_undo_num_==0)
			return;
		if (mask_undo_pointer_<0 ||
			mask_undo_pointer_>mask_undos_.size()-2)
			return;

		//move pointer
		mask_undo_pointer_++;

		//update mask data
		auto c = data_.find(CompType::Mask);
		if (c == data_.end())
			return;
		Nrrd* nrrd = c->second.data;
		nrrdWrap_va(nrrd,
			mask_undos_[mask_undo_pointer_],
			nrrdTypeUChar, 3, (size_t)res_.intx(),
			(size_t)res_.inty(), (size_t)res_.intz());
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
		int bnx, bny, bnz;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		bnz = brick_num_.intz();
		int x = (id % (bnx * bny)) % bnx;
		if (x == 0)
			return id;
		int y = (id % (bnx * bny)) / bnx;
		int z = id / (bnx * bny);
		int r = z * bnx * bny + y * bnx + x - 1;
		if (r < 0 || r >= bnx * bny * bnz)
			return id;
		else
			return r;
	}

	unsigned int Texture::negyid(unsigned int id)
	{
		int bnx, bny, bnz;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		bnz = brick_num_.intz();
		int y = (id % (bnx * bny)) / bnx;
		if (y == 0)
			return id;
		int x = (id % (bnx * bny)) % bnx;
		int z = id / (bnx * bny);
		int r = z * bnx * bny + (y - 1) * bnx + x;
		if (r < 0 || r >= bnx * bny * bnz)
			return id;
		else
			return r;
	}

	unsigned int Texture::negzid(unsigned int id)
	{
		int bnx, bny, bnz;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		bnz = brick_num_.intz();
		int z = id / (bnx * bny);
		if (z == 0)
			return id;
		int x = (id % (bnx * bny)) % bnx;
		int y = (id % (bnx * bny)) / bnx;
		int r = (z - 1) * bnx * bny + y * bnx + x;
		if (r < 0 || r >= bnx * bny * bnz)
			return id;
		else
			return r;
	}

	unsigned int Texture::posxid(unsigned int id)
	{
		int bnx, bny, bnz;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		bnz = brick_num_.intz();
		int x = (id % (bnx * bny)) % bnx;
		if (x == bnx - 1)
			return id;
		int y = (id % (bnx * bny)) / bnx;
		int z = id / (bnx * bny);
		int r = z * bnx * bny + y * bnx + x + 1;
		if (r < 0 || r >= bnx * bny * bnz)
			return id;
		else
			return r;
	}

	unsigned int Texture::posyid(unsigned int id)
	{
		int bnx, bny, bnz;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		bnz = brick_num_.intz();
		int y = (id % (bnx * bny)) / bnx;
		if (y == bny - 1)
			return id;
		int x = (id % (bnx * bny)) % bnx;
		int z = id / (bnx * bny);
		int r = z * bnx * bny + (y + 1) * bnx + x;
		if (r < 0 || r >= bnx * bny * bnz)
			return id;
		else
			return r;
	}

	unsigned int Texture::poszid(unsigned int id)
	{
		int bnx, bny, bnz;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		bnz = brick_num_.intz();
		int z = id / (bnx * bny);
		if (z == bnz - 1)
			return id;
		int x = (id % (bnx * bny)) % bnx;
		int y = (id % (bnx * bny)) / bnx;
		int r = (z + 1) * bnx * bny + y * bnx + x;
		if (r < 0 || r >= bnx * bny * bnz)
			return id;
		else
			return r;
	}

	unsigned int Texture::get_brick_id(unsigned long long index)
	{
		int nx, ny;
		nx = res_.intx();
		ny = res_.inty();
		int bszx, bszy, bszz;
		bszx = brick_res_.intx();
		bszy = brick_res_.inty();
		bszz = brick_res_.intz();
		int bnx, bny;
		bnx = brick_num_.intx();
		bny = brick_num_.inty();
		unsigned long long x, y, z;
		z = index / (nx * ny);
		y = index % (nx * ny);
		x = y % nx;
		y = y / nx;
		//get brick indices
		x = bszx <= 1 ? 0 : x / (bszx-1);
		y = bszy <= 1 ? 0 : y / (bszy-1);
		z = bszz <= 1 ? 0 : z / (bszz-1);
		return static_cast<unsigned int>(z * bnx * bny + y * bnx + x);
	}
} // namespace flvr
