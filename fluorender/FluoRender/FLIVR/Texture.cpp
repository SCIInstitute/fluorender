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

#include <FLIVR/ShaderProgram.h>
#include <FLIVR/Texture.h>
#include <FLIVR/TextureRenderer.h>
#include <FLIVR/Utils.h>
#include <algorithm>

using namespace std;

namespace FLIVR
{
	Texture::Texture()
		: nx_(0),
		ny_(0),
		nz_(0),
		nc_(0),
		vmin_(0.0),
		vmax_(0.0),
		gmin_(0.0),
		gmax_(0.0),
		nmask_(-1),
		nlabel_(-1),
		sort_bricks_(true),
		use_priority_(false),
		n_p0_(0)
	{
		for (int i = 0; i < TEXTURE_MAX_COMPONENTS; i++)
		{
			nb_[i] = 0;
			data_[i] = 0;
			ntype_[i] = TYPE_NONE;
		}
	}

	Texture::~Texture()
	{
		for (int i=0; i<(int)bricks_.size(); i++)
		{
			if (bricks_[i])
				delete bricks_[i];
		}

		for (int i=0; i<TEXTURE_MAX_COMPONENTS; i++)
		{
			if (data_[i])
			{
				delete [] data_[i]->data;
				nrrdNix(data_[i]);
			}
		}
	}

	vector<TextureBrick*>* Texture::get_sorted_bricks(
		Ray& view, bool is_orthographic)
	{
		if (sort_bricks_)
		{
			for (unsigned int i = 0; i < bricks_.size(); i++)
			{
				Point minp(bricks_[i]->bbox().min());
				Point maxp(bricks_[i]->bbox().max());
				Vector diag(bricks_[i]->bbox().diagonal());
				minp += diag / 1000.;
				maxp -= diag / 1000.;
				Point corner[8];
				corner[0] = minp;
				corner[1] = Point(minp.x(), minp.y(), maxp.z());
				corner[2] = Point(minp.x(), maxp.y(), minp.z());
				corner[3] = Point(minp.x(), maxp.y(), maxp.z());
				corner[4] = Point(maxp.x(), minp.y(), minp.z());
				corner[5] = Point(maxp.x(), minp.y(), maxp.z());
				corner[6] = Point(maxp.x(), maxp.y(), minp.z());
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
				bricks_[i]->set_d(d);
			}
			if (TextureRenderer::get_update_order() == 0)
				std::sort(bricks_.begin(), bricks_.end(), TextureBrick::sort_asc);
			else if (TextureRenderer::get_update_order() == 1)
				std::sort(bricks_.begin(), bricks_.end(), TextureBrick::sort_dsc);

			sort_bricks_ = false;
		}
		return &bricks_;
	}

	vector<TextureBrick*>* Texture::get_closest_bricks(
		Point& center, int quota, bool skip,
		Ray& view, bool is_orthographic)
	{
		if (sort_bricks_)
		{
			quota_bricks_.clear();
			unsigned int i;
			if (quota >= bricks_.size())
				quota = int(bricks_.size());
			else
			{
				for (i=0; i<bricks_.size(); i++)
				{
					Point brick_center = bricks_[i]->bbox().center();
					double d = (brick_center - center).length();
					bricks_[i]->set_d(d);
				}
				std::sort(bricks_.begin(), bricks_.end(), TextureBrick::sort_dsc);
			}

			for (i=0; i<(unsigned int)bricks_.size(); i++)
			{
				if (skip)
				{
					if (bricks_[i]->get_priority() == 0)
						quota_bricks_.push_back(bricks_[i]);
				}
				else
					quota_bricks_.push_back(bricks_[i]);
				if (quota_bricks_.size() == quota)
					break;
			}

			for (i = 0; i < quota_bricks_.size(); i++)
			{
				Point minp(quota_bricks_[i]->bbox().min());
				Point maxp(quota_bricks_[i]->bbox().max());
				Vector diag(quota_bricks_[i]->bbox().diagonal());
				minp += diag / 1000.;
				maxp -= diag / 1000.;
				Point corner[8];
				corner[0] = minp;
				corner[1] = Point(minp.x(), minp.y(), maxp.z());
				corner[2] = Point(minp.x(), maxp.y(), minp.z());
				corner[3] = Point(minp.x(), maxp.y(), maxp.z());
				corner[4] = Point(maxp.x(), minp.y(), minp.z());
				corner[5] = Point(maxp.x(), minp.y(), maxp.z());
				corner[6] = Point(maxp.x(), maxp.y(), minp.z());
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
			if (TextureRenderer::get_update_order() == 0)
				std::sort(quota_bricks_.begin(), quota_bricks_.end(), TextureBrick::sort_asc);
			else if (TextureRenderer::get_update_order() == 1)
				std::sort(quota_bricks_.begin(), quota_bricks_.end(), TextureBrick::sort_dsc);

			sort_bricks_ = false;
		}

		return &quota_bricks_;
	}

	vector<TextureBrick*>* Texture::get_bricks()
	{
		return &bricks_;
	}

	vector<TextureBrick*>* Texture::get_quota_bricks()
	{
		return &quota_bricks_;
	}

	void Texture::set_spacings(double x, double y, double z)
	{
		spcx_ = x;
		spcy_ = y;
		spcz_ = z;
		Transform tform;
		tform.load_identity();
		Point nmax(nx_*x, ny_*y, nz_*z);
		tform.pre_scale(Vector(nmax));
		set_transform(tform);
	}

	void Texture::build(Nrrd* nv_nrrd, Nrrd* gm_nrrd,
		double vmn, double vmx,
		double gmn, double gmx)
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

		BBox bb(Point(0,0,0), Point(1,1,1)); 

		Transform tform;
		tform.load_identity();

		size_t dim = nv_nrrd->dim;
		std::vector<int> size(dim);

		int offset = 0;
		if (dim > 3) offset = 1; 

		for (size_t p = 0; p < dim; p++) 
			size[p] = (int)nv_nrrd->axis[p + offset].size;

		if (size[0] != nx() || size[1] != ny() || size[2] != nz() ||
			numc != nc() || numb[0] != nb(0) ||
			bb.min() != bbox()->min() ||
			bb.max() != bbox()->max() ||
			vmn != vmin() ||
			vmx != vmax() ||
			gmn != gmin() ||
			gmx != gmax() )
		{
			build_bricks(bricks_, size[0], size[1], size[2], numc, numb);
			set_size(size[0], size[1], size[2], numc, numb);
		}
		set_bbox(bb);
		set_minmax(vmn, vmx, gmn, gmx);
		set_transform(tform);

		n_p0_ = 0;
		for (unsigned int i = 0; i < bricks_.size(); i++)
		{
			TextureBrick *tb = bricks_[i];
			tb->set_nrrd(nv_nrrd, 0);
			tb->set_nrrd(gm_nrrd, 1);
			if (use_priority_)
			{
				tb->set_priority();
				n_p0_ += tb->get_priority()==0?1:0;
			}
		}

		BBox tempb;
		tempb.extend(transform_.project(bbox_.min()));
		tempb.extend(transform_.project(bbox_.max()));
		spcx_ = (tempb.max().x() - tempb.min().x()) / double(nx_);
		spcy_ = (tempb.max().y() - tempb.min().y()) / double(ny_);
		spcz_ = (tempb.max().z() - tempb.min().z()) / double(nz_);

		set_nrrd(nv_nrrd, 0);
		set_nrrd(gm_nrrd, 1);
	}

	void Texture::build_bricks(vector<TextureBrick*> &bricks, 
		int sz_x, int sz_y, int sz_z,
		int numc, int* numb)
	{
		bool force_pow2 = false;
		if (ShaderProgram::init())
			force_pow2 = !ShaderProgram::texture_non_power_of_two();

		int max_texture_size = 2048;
		if (ShaderProgram::init())
			max_texture_size = ShaderProgram::max_texture_size();

		//further determine the max texture size
		if (TextureRenderer::get_mem_swap())
		{
			double data_size = double(sz_x)*double(sz_y)*double(sz_z)*double(numb[0])/1.04e6;
			if (data_size > TextureRenderer::get_mem_limit() ||
				data_size > TextureRenderer::get_large_data_size())
				max_texture_size = TextureRenderer::get_force_brick_size();
		}

		// Initial brick size
		int bsize[3];

		if (force_pow2)
		{
			if (Pow2(sz_x) > (unsigned)sz_x) 
				bsize[0] = Min(int(Pow2(sz_x))/2, max_texture_size);
			if (Pow2(sz_y) > (unsigned)sz_y) 
				bsize[1] = Min(int(Pow2(sz_x))/2, max_texture_size);
			if (Pow2(sz_z) > (unsigned)sz_z) 
				bsize[2] = Min(int(Pow2(sz_x))/2, max_texture_size);
		}
		else
		{
			bsize[0] = Min(int(Pow2(sz_x)), max_texture_size);
			bsize[1] = Min(int(Pow2(sz_y)), max_texture_size);
			bsize[2] = Min(int(Pow2(sz_z)), max_texture_size);
		}

		bricks.clear();

		int i, j, k;
		int mx, my, mz, mx2, my2, mz2;
		double tx0, ty0, tz0, tx1, ty1, tz1;
		double bx1, by1, bz1;
		for (k = 0; k < sz_z; k += bsize[2])
		{
			if (k) k--;
			for (j = 0; j < sz_y; j += bsize[1])
			{
				if (j) j--;
				for (i = 0; i < sz_x; i += bsize[0])
				{
					if (i) i--;
					mx = Min(bsize[0], sz_x - i);
					my = Min(bsize[1], sz_y - j);
					mz = Min(bsize[2], sz_z - k);

					mx2 = mx;
					my2 = my;
					mz2 = mz;
					if (force_pow2)
					{
						mx2 = Pow2(mx);
						my2 = Pow2(my);
						mz2 = Pow2(mz);
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

					BBox tbox(Point(tx0, ty0, tz0), Point(tx1, ty1, tz1));

					// Compute BBox.
					bx1 = Min((i + bsize[0] - 0.5) / (double)sz_x, 1.0);
					if (sz_x - i == bsize[0]) bx1 = 1.0;

					by1 = Min((j + bsize[1] - 0.5) / (double)sz_y, 1.0);
					if (sz_y - j == bsize[1]) by1 = 1.0;

					bz1 = Min((k + bsize[2] - 0.5) / (double)sz_z, 1.0);
					if (sz_z - k == bsize[2]) bz1 = 1.0;

					BBox bbox(Point(i==0?0:(i+0.5) / (double)sz_x,
						j==0?0:(j+0.5) / (double)sz_y,
						k==0?0:(k+0.5) / (double)sz_z),
						Point(bx1, by1, bz1));

					TextureBrick *b = new TextureBrick(0, 0, mx2, my2, mz2, numc, numb, 
						i-(mx2-mx), j-(my2-my), k-(mz2-mz),
						mx2, my2, mz2, bbox, tbox);
					bricks.push_back(b);
				}
			}
		}
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
			for (i=0; i<(int)bricks_.size(); i++)
			{
				bricks_[i]->nmask(nmask_);
				bricks_[i]->nb(1, nmask_);
				bricks_[i]->ntype(TextureBrick::TYPE_MASK, nmask_);
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
			for (i=0; i<(int)bricks_.size(); i++)
			{
				bricks_[i]->nlabel(nlabel_);
				bricks_[i]->nb(4, nlabel_);
				bricks_[i]->ntype(TextureBrick::TYPE_LABEL, nlabel_);
			}
			return true;
		}
		else
			return false;
	}

	//set nrrd
	void Texture::set_nrrd(Nrrd* data, int index)
	{
		if (index>=0&&index<TEXTURE_MAX_COMPONENTS)
		{
			if (data_[index] && data)
			{
				delete [] data_[index]->data;
				nrrdNix(data_[index]);
			}

			data_[index] = data;
			for (int i=0; i<(int)bricks_.size(); i++)
			{
				bricks_[i]->set_nrrd(data, index);
			}
		}
	}

} // namespace FLIVR
