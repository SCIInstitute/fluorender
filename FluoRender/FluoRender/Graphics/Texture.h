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

#ifndef Texture_h
#define Texture_h

#include <nrrd.h>
#include <BBox.h>
#include <Transform.h>
#include <Point.h>
#include <Vector.h>
#include <glm/mat4x4.hpp>
#include <vector>
#include <unordered_map>

#ifndef TextureBrick_h
#define TEXTURE_MAX_COMPONENTS	4
#endif

namespace fluo
{
	class Ray;
}
namespace flvr
{
	class TextureBrick;
	struct Pyramid_Level;
	class FileLocInfo;
	enum class CompType : int
	{
		None,
		Data,
		Gradient,
		GradientMagnitude,
		Mask,
		Label
	};
	struct TexComp
	{
		CompType type;
		int bytes;
		Nrrd* data;
	};
	class Texture 
	{
	public:
		static size_t mask_undo_num_;
		Texture();
		virtual ~Texture();

		int get_build_max_tex_size() { return build_max_tex_size_; }
		void set_brick_planned_size(int size) { brick_planned_size_ = size; }
		bool build(Nrrd* val,
			double vmn, double vmx,
			std::vector<flvr::TextureBrick*>* brks = NULL);

		fluo::Vector get_res() { return res_; }
		fluo::Vector get_brick_res() { return brick_res_; }
		fluo::Vector get_brick_num() { return brick_num_; }

		//get neighbor id
		unsigned int negxid(unsigned int id);
		unsigned int negyid(unsigned int id);
		unsigned int negzid(unsigned int id);
		unsigned int posxid(unsigned int id);
		unsigned int posyid(unsigned int id);
		unsigned int poszid(unsigned int id);
		//get brick id by voxel index
		unsigned int get_brick_id(unsigned long long index);
		TextureBrick* get_brick(unsigned int bid);

		int nc() { return data_.size(); }
		int nb(CompType type)
		{
			auto c = data_.find(type);
			if (c != data_.end())
				return c->second.bytes;
			return 0;
		}
		void nb(CompType type, int bytes)
		{
			auto c = data_.find(type);
			if (c != data_.end())
				c->second.bytes = bytes;
		}

		void set_res(const fluo::Vector& res) 
		{
			res_ = res;
		}

		//! Interface that does not expose flvr::BBox.
		void get_bounds(double& xmin, double& ymin, double& zmin,
			double& xmax, double& ymax, double& zmax) const;

		void get_bounds(fluo::BBox& b) const;

		fluo::BBox *bbox() { return &bbox_; }
		void set_bbox(const fluo::BBox& bbox) { bbox_ = bbox; }
		fluo::Transform *transform() { return &transform_; }
		void set_transform(fluo::Transform tform) { transform_ = tform; }

		// get sorted bricks
		std::vector<TextureBrick*>* get_sorted_bricks(
			fluo::Ray& view, bool is_orthographic = false);
		//get closest bricks
		std::vector<TextureBrick*>* get_closest_bricks(
			fluo::Point& center, int quota, bool skip,
			fluo::Ray& view, bool is_orthographic = false);
		//set sort bricks
		void set_sort_bricks() {sort_bricks_ = true;}
		void reset_sort_bricks() {sort_bricks_ = false;}
		bool get_sort_bricks() {return sort_bricks_;}
		// load the bricks independent of the view
		std::vector<TextureBrick*>* get_bricks();
		//get bricks sorted by id
		std::vector<TextureBrick*>* get_bricks_id();
		int get_brick_list_size() {return int((*bricks_).size());}
		//quota bricks
		std::vector<TextureBrick*>* get_quota_bricks();

		// Tests the bounding box against the current MODELVIEW and
		// PROJECTION matrices to determine if it is within the viewport.
		// Returns true if it is visible.
		void set_matrices(glm::mat4 &mv_mat2, glm::mat4 &proj_mat);
		bool test_against_view(const fluo::BBox &bbox, bool persp = false);

		int nlevels(){ return int((*bricks_).size()); }

		double vmin() const { return vmin_; }
		double vmax() const { return vmax_; }
		void set_minmax(double vmin, double vmax) {vmin_ = vmin; vmax_ = vmax;}

		void set_spacing(const fluo::Vector& spc);
		fluo::Vector get_spacing(int lv = -1);
		void set_base_spacing(const fluo::Vector& spc);
		fluo::Vector get_base_spacing() { return base_spacing_; }
		void set_spacing_scale(const fluo::Vector& scl) { spacing_scale_ = scl; }
		fluo::Vector get_spacing_scale() { return spacing_scale_; }

		// Creator of the brick owns the nrrd memory.
		void set_nrrd(CompType type, TexComp comp);
		TexComp get_nrrd(CompType type);
		bool trim_mask_undos_head();
		bool trim_mask_undos_tail();
		bool get_undo();
		bool get_redo();
		void set_mask(void* mask_data);
		void push_mask();
		void pop_mask();
		void mask_undos_forward();
		void mask_undos_backward();
		void clear_undos();

		//add one more texture component as the volume mask
		bool add_empty_mask();
		//add one more texture component as the labeling volume
		bool add_empty_label();

		//enable mask paint for all
		void deact_all_mask();
		//activate all masks
		void act_all_mask();
		//invalidate all masks
		void invalid_all_mask();
		//validate all masks
		void valid_all_mask();

		//get priority brick number
		void set_use_priority(bool value) {use_priority_ = value;}
		bool get_use_priority() {return use_priority_;}
		int get_n_p0()
		{if (use_priority_) return n_p0_; else return int((*bricks_).size());}

		//for brkxml file
		int GetCurLevel() { return pyramid_cur_lv_; }
		int GetLevelNum();
		void SetCopyableLevel(int lv) { pyramid_copy_lv_ = lv; }
		int GetCopyableLevel() { return pyramid_copy_lv_; }
		bool buildPyramid(std::vector<Pyramid_Level> &pyramid, std::vector<std::vector<std::vector<std::vector<FileLocInfo *>>>> &filenames, bool useURL = false);
		void setLevel(int lv);
		void set_data_file(std::vector<FileLocInfo *> *fname, int type);
		bool isBrxml() { return brkxml_; }
		FileLocInfo *GetFileName(int id);
		void set_FrameAndChannel(int fr, int ch);

	protected:
		void build_bricks(std::vector<TextureBrick*> &bricks,
			const fluo::Vector& size, int bytes);

		//remember the brick size, as it may change
		int build_max_tex_size_;
		//expected brick size, 0: ignored
		int brick_planned_size_;
		//! data carved up to texture memory sized chunks.
		std::vector<TextureBrick*>						*bricks_;
		//for limited number of bricks during interactions
		std::vector<TextureBrick*>						quota_bricks_;
		//sort texture brick
		bool sort_bricks_;
		//! data size
		fluo::Vector res_;
		//brick size, planned
		fluo::Vector brick_res_;
		//brick num
		fluo::Vector brick_num_;
		//! data tform
		fluo::Transform								transform_;
		double										vmin_;
		double										vmax_;
		//! data bbox
		fluo::BBox									bbox_;
		//! spacings
		fluo::Vector spacing_;
		//! base spacings (for brxml)
		fluo::Vector base_spacing_;
		//! scales of spacings (for brxml)
		fluo::Vector spacing_scale_;
		//priority
		bool use_priority_;
		int n_p0_;

		std::unordered_map<CompType, TexComp> data_;

		//undos for mask
		std::vector<void*> mask_undos_;
		int mask_undo_pointer_;

		//for brkxml
		bool brkxml_;
		int pyramid_cur_lv_;
		int pyramid_cur_fr_;
		int pyramid_cur_ch_;
		int pyramid_lv_num_;

		int pyramid_copy_lv_;
		int filetype_;

		std::vector<Pyramid_Level> pyramid_;
		std::vector<std::vector<std::vector<std::vector<FileLocInfo *>>>> filenames_;

		std::vector<FileLocInfo *> *filename_;
		void clearPyramid();

		//used when brkxml_ is not equal to false.
		std::vector<TextureBrick*> default_vec_;

		//for view testing
		fluo::Transform mv_;
		fluo::Transform pr_;
	};

} // namespace flvr

#endif // Volume_Texture_h
