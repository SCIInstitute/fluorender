//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#ifndef TextureBrick_h
#define TextureBrick_h

#include <RawData.h>
#include <Ray.h>
#include <Texture.h>
#include <stdint.h>
#include <map>

//these are the render modes used to determine if each mode is drawn
#define TEXTURE_RENDER_MODES	5

#define BRICK_FILE_TYPE_NONE	0
#define BRICK_FILE_TYPE_RAW		1
#define BRICK_FILE_TYPE_JPEG	2
#define BRICK_FILE_TYPE_ZLIB	3

#ifndef __glew_h__
typedef unsigned int GLenum;
#endif

namespace flvr
{
	class FileLocInfo
	{
	public:
		FileLocInfo()
		{
			filename = L"";
			offset = 0;
			datasize = 0;
			type = 0;
			isurl = false;
			cached = false;
			cache_filename = L"";
		}
		FileLocInfo(std::wstring filename_, int offset_, int datasize_, int type_, bool isurl_)
		{
			filename = filename_;
			offset = offset_;
			datasize = datasize_;
			type = type_;
			isurl = isurl_;
			cached = false;
			cache_filename = L"";
		}
		FileLocInfo(const FileLocInfo &copy)
		{
			filename = copy.filename;
			offset = copy.offset;
			datasize = copy.datasize;
			type = copy.type;
			isurl = copy.isurl;
			cached = copy.cached;
			cache_filename = copy.cache_filename;
		}

		std::wstring filename;
		int offset;
		int datasize;
		int type; //1-raw; 2-jpeg; 3-zlib;
		bool isurl;
		bool cached;
		std::wstring cache_filename;
	};

	class TextureBrick
	{
	public:
		TextureBrick(std::shared_ptr<fluo::RawData> rd,
			const fluo::Vector& size, int byte,
			const fluo::Vector& off_size,
			const fluo::Vector& msize,
			const fluo::BBox& bbox,
			const fluo::BBox& tbox,
			const fluo::BBox& dbox,
			unsigned int id,
			int findex = 0,
			long long offset = 0LL,
			long long fsize = 0LL);
		virtual ~TextureBrick();

		inline fluo::BBox &bbox() { return bbox_; }
		inline fluo::BBox &tbox() { return tbox_; }
		inline fluo::BBox &dbox() { return dbox_; }

		inline int nc() { return static_cast<int>(data_.size()); }
		inline int nb(CompType type)
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

		inline fluo::Vector get_size() { return size_; }
		inline fluo::Vector get_msize() { return msize_; }
		inline fluo::Vector get_off_size() { return off_size_; }
		inline void set_stride(const fluo::Vector& stride)
		{
			stride_ = stride;
			stride_valid_ = true;
		}
		inline fluo::Vector get_stride()
		{
			if (stride_valid_)
				return stride_;
			else
				return size_;
		}

		inline void set_drawn(int mode, bool val)
		{ if (mode>=0 && mode<TEXTURE_RENDER_MODES) drawn_[mode] = val; }
		inline void set_drawn(bool val)
		{ for (int i=0; i<TEXTURE_RENDER_MODES; i++) drawn_[i] = val; }
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		inline bool drawn(int mode)
		{ if (mode>=0 && mode<TEXTURE_RENDER_MODES) return drawn_[mode]; else return false;}

		void set_tex_comp(CompType type, TexComp comp) { data_[type] = comp;}
		TexComp get_tex_comp(CompType type)
		{
			auto c = data_.find(type);
			if (c == data_.end())
				return TexComp();
			else
				return c->second;
		}

		//find out priority
		void set_priority();
		inline int get_priority() {return priority_;}

		GLenum tex_type(CompType type);
		std::shared_ptr<fluo::RawData> get_raw_data(CompType type);
		std::shared_ptr<fluo::RawData> get_raw_data_lod(CompType type, const std::shared_ptr<FileLocInfo>& finfo);
		//void* tex_data(CompType type, void* raw_data);//given external raw data, using the same address in brick
		//void* tex_data_brk(CompType type, const FileLocInfo* finfo);

		void compute_polygons(fluo::Ray& view, double tmin, double tmax, double dt,
			std::vector<float>& vertex, std::vector<uint32_t>& index, std::vector<uint32_t>& size);
		void compute_polygons(fluo::Ray& view, double dt,
			std::vector<float>& vertex, std::vector<uint32_t>& index,
			std::vector<uint32_t>& size, bool bricks=false);
		
		//set d
		void set_d(double d) { d_ = d; }
		double get_d() { return d_; }
		//sorting function
		static bool sort_asc(const std::shared_ptr<TextureBrick>& b1, const std::shared_ptr<TextureBrick>& b2)
		{ return b1->d_ > b2->d_; }
		static bool sort_dsc(const std::shared_ptr<TextureBrick>& b1, const std::shared_ptr<TextureBrick>& b2)
		{ return b2->d_ > b1->d_; }
		static bool sort_id(const std::shared_ptr<TextureBrick>& b1, const std::shared_ptr<TextureBrick>& b2)
		{ return b1->id_ < b2->id_; }

		//current index
		inline void set_ind(size_t ind) {ind_ = ind;}
		inline size_t get_ind() {return ind_;}

		//id for analysis
		inline unsigned int get_id() { return id_; }

		//get value
		double get_data(const fluo::Point& ijk);

		void freeBrkData();
		bool read_brick(std::shared_ptr<fluo::RawData>& data, const std::shared_ptr<FileLocInfo>& finfo);
		bool isLoaded() { return brkdata_ ? true : false; };
		bool isLoading() { return loading_; }
		void set_loading_state(bool val) { loading_ = val; }
		void set_id_in_loadedbrks(int id) { id_in_loadedbrks = id; };
		int get_id_in_loadedbrks() { return id_in_loadedbrks; }
		int getID() { return findex_; }

		void set_rawdata_lod(const std::shared_ptr<fluo::RawData>& data) { brkdata_ = data; }
		std::shared_ptr<fluo::RawData> get_raw_data_lod() { return brkdata_; }
		static bool read_brick_without_decomp(std::shared_ptr<fluo::RawData>& data, size_t &readsize, std::shared_ptr<FileLocInfo> finfo);

		void set_disp(bool disp) { disp_ = disp; }
		bool get_disp() { return disp_; }

		//mask validity after painting
		void valid_mask(bool val=true) { mask_valid_ = val; }
		void invalid_mask() { mask_valid_ = false; }
		bool is_mask_valid() { return mask_valid_; }
		bool is_nbmask_valid(const std::shared_ptr<Texture>& tex);//check 6 neighbors
		//activate/deactivate mask painting
		void act_mask(bool val = true) { mask_act_ = val; }
		void deact_mask() { mask_act_ = false; }
		bool is_mask_act() { return mask_act_; }

		void set_new_grown(bool val) { new_grown_ = val; }
		bool get_new_grown() { return new_grown_; }

	private:
		void compute_edge_rays(fluo::BBox &bbox);
		void compute_edge_rays_tex(fluo::BBox &bbox);
		size_t tex_type_size(GLenum t);

		bool raw_brick_reader(std::shared_ptr<fluo::RawData>& data, const std::shared_ptr<FileLocInfo>& finfo);

		//! bbox edges
		fluo::Ray edge_[12];
		//! tbox edges
		fluo::Ray tex_edge_[12];
		//data, same from texture
		std::unordered_map<CompType, TexComp> data_;

		//! axis sizes (pow2)
		fluo::Vector size_;
		//! offset into volume texture
		fluo::Vector off_size_;
		//! data axis sizes (not necessarily pow2)
		fluo::Vector msize_;
		//stride size
		bool stride_valid_ = false;
		fluo::Vector stride_;
		//! bounding box and texcoord box
		fluo::BBox bbox_, tbox_, dbox_;
		fluo::Vector view_vector_;
		//a value used for sorting
		//usually distance
		double d_;
		//priority level
		int priority_;//now, 0:highest
		//if it's been drawn in a full update loop
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		bool drawn_[TEXTURE_RENDER_MODES];
		//current index in the queue, for reverse searching
		size_t ind_;
		//id for analysis
		unsigned int id_;
		//mask validity after painting
		bool mask_valid_;
		//mask is active (being painted)
		bool mask_act_;
		//new label for grow ruler merge
		bool new_grown_;

		int findex_;
		long long offset_;
		long long fsize_;
		std::shared_ptr<fluo::RawData> brkdata_;
		bool loading_;
		int id_in_loadedbrks;
		bool disp_;

		static std::map<std::wstring, std::wstring> cache_table_;
	};

	inline double TextureBrick::get_data(const fluo::Point& ijk)
	{
		auto rawd = data_[CompType::Data].data;
		if (!rawd)
			return 0.0;

		const size_t x = off_size_.intx() + ijk.intx();
		const size_t y = off_size_.inty() + ijk.inty();
		const size_t z = off_size_.intz() + ijk.intz();

		return rawd->GetVoxelValue(x, y, z);
	}

	struct Pyramid_Level {
		std::vector<std::shared_ptr<FileLocInfo>> filenames;
		int filetype;
		std::shared_ptr<fluo::RawData> data;
		std::vector<std::shared_ptr<TextureBrick>> bricks;
		//information
		//total size
		fluo::Vector size;
		//int szx;
		//int szy;
		//int szz;
		//typical brick size
		fluo::Vector bsize;
		//int bszx;
		//int bszy;
		//int bszz;
		//brick num along axes
		fluo::Vector bnum;
		//int bnx;
		//int bny;
		//int bnz;
		//spacing
		fluo::Vector spacing;
	};
} // namespace flvr

#endif // Volume_TextureBrick_h
