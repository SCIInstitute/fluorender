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

#ifndef TextureBrick_h
#define TextureBrick_h

#include <Ray.h>
#include <BBox.h>
#include <vector>
#include <nrrd.h>
#include <stdint.h>
#include <map>

// We use no more than 2 texture units.
// GL_MAX_TEXTURE_UNITS is the actual maximum.
//we now added maximum to 4
//which can include mask volumes
#define TEXTURE_MAX_COMPONENTS	4
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

	class Texture;
	class TextureBrick
	{
	public:
		enum CompType
		{
			TYPE_NONE=0, TYPE_INT, TYPE_INT_GRAD, TYPE_GM, TYPE_MASK, TYPE_LABEL
		};
		// Creator of the brick owns the nrrd memory.
		TextureBrick(Nrrd* n0, Nrrd* n1,
			int nx, int ny, int nz, int nc, int* nb, int ox, int oy, int oz,
			int mx, int my, int mz, const fluo::BBox& bbox, const fluo::BBox& tbox, const fluo::BBox& dbox,
			unsigned int id, int findex = 0, long long offset = 0LL, long long fsize = 0LL);
		virtual ~TextureBrick();

		inline fluo::BBox &bbox() { return bbox_; }
		inline fluo::BBox &tbox() { return tbox_; }
		inline fluo::BBox &dbox() { return dbox_; }

		inline int nx() { return nx_<0?0:nx_; }
		inline int ny() { return ny_<0?0:ny_; }
		inline int nz() { return nz_<0?0:nz_; }
		inline int nc() { return nc_<0?0:nc_; }
		inline int nb(int c)
		{
			assert(c >= 0 && c < TEXTURE_MAX_COMPONENTS);
			return nb_[c];
		}
		void nb(int n, int c)
		{
			assert(c >= 0 && c < TEXTURE_MAX_COMPONENTS);
			nb_[c] = n;
		}
		inline void nmask(int mask) { nmask_ = mask; }
		inline int nmask() { return nmask_; }
		inline void nlabel(int label) {nlabel_ = label;}
		inline int nlabel() {return nlabel_;}
		inline void ntype(CompType type, int c)
		{
			assert(c >= 0 && c < TEXTURE_MAX_COMPONENTS);
			ntype_[c] = type;
		}
		inline CompType ntype(int c)
		{
			assert(c >= 0 && c < TEXTURE_MAX_COMPONENTS);
			return ntype_[c];
		}

		inline int mx() { return mx_; }
		inline int my() { return my_; }
		inline int mz() { return mz_; }

		inline int ox() { return ox_; }
		inline int oy() { return oy_; }
		inline int oz() { return oz_; }

		virtual int sx();
		virtual int sy();

		inline void set_drawn(int mode, bool val)
		{ if (mode>=0 && mode<TEXTURE_RENDER_MODES) drawn_[mode] = val; }
		inline void set_drawn(bool val)
		{ for (int i=0; i<TEXTURE_RENDER_MODES; i++) drawn_[i] = val; }
		//mode: 0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		inline bool drawn(int mode)
		{ if (mode>=0 && mode<TEXTURE_RENDER_MODES) return drawn_[mode]; else return false;}

		// Creator of the brick owns the nrrd memory.
		void set_nrrd(Nrrd* data, int index)
		{if (index>=0&&index<TEXTURE_MAX_COMPONENTS) data_[index] = data;}
		Nrrd* get_nrrd(int index)
		{if (index>=0&&index<TEXTURE_MAX_COMPONENTS) return data_[index]; else return 0;}

		//find out priority
		void set_priority();
		inline int get_priority() {return priority_;}

		virtual GLenum tex_type(int c);
		virtual void* tex_data(int c);
		virtual void* tex_data(int c, void* raw_data);//given external raw data, using the same address in brick
		virtual void* tex_data_brk(int c, const FileLocInfo* finfo);

		void compute_polygons(fluo::Ray& view, double tmin, double tmax, double dt,
			std::vector<float>& vertex, std::vector<uint32_t>& index, std::vector<uint32_t>& size);
		void compute_polygons(fluo::Ray& view, double dt,
			std::vector<float>& vertex, std::vector<uint32_t>& index,
			std::vector<uint32_t>& size, bool bricks=false);
		
		//set d
		void set_d(double d) { d_ = d; }
		double get_d() { return d_; }
		//sorting function
		static bool sort_asc(const TextureBrick* b1, const TextureBrick* b2)
		{ return b1->d_ > b2->d_; }
		static bool sort_dsc(const TextureBrick* b1, const TextureBrick* b2)
		{ return b2->d_ > b1->d_; }
		static bool sort_id(const TextureBrick* b1, const TextureBrick* b2)
		{ return b1->id_ < b2->id_; }

		//current index
		inline void set_ind(size_t ind) {ind_ = ind;}
		inline size_t get_ind() {return ind_;}

		//id for analysis
		inline unsigned int get_id() { return id_; }

		//get value
		double get_data(unsigned int i, unsigned int j, unsigned int k);

		void freeBrkData();
		bool read_brick(char* data, size_t size, const FileLocInfo* finfo);
		bool isLoaded() { return brkdata_ ? true : false; };
		bool isLoading() { return loading_; }
		void set_loading_state(bool val) { loading_ = val; }
		void set_id_in_loadedbrks(int id) { id_in_loadedbrks = id; };
		int get_id_in_loadedbrks() { return id_in_loadedbrks; }
		int getID() { return findex_; }

		void set_brkdata(char *brkdata) { brkdata_ = brkdata; }
		const char *getBrickData() { return brkdata_; }
		static bool read_brick_without_decomp(char* &data, size_t &readsize, FileLocInfo* finfo, void *th = NULL);

		void set_disp(bool disp) { disp_ = disp; }
		bool get_disp() { return disp_; }

		//mask validity after painting
		void valid_mask(bool val=true) { mask_valid_ = val; }
		void invalid_mask() { mask_valid_ = false; }
		bool is_mask_valid() { return mask_valid_; }
		bool is_nbmask_valid(Texture* tex);//check 6 neighbors
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
		GLenum tex_type_aux(Nrrd* n);

		bool raw_brick_reader(char* data, size_t size, const FileLocInfo* finfo);

		//! bbox edges
		fluo::Ray edge_[12];
		//! tbox edges
		fluo::Ray tex_edge_[12];
		Nrrd* data_[TEXTURE_MAX_COMPONENTS];
		//! axis sizes (pow2)
		int nx_, ny_, nz_; 
		//! number of components (< TEXTURE_MAX_COMPONENTS)
		int nc_; 
		//type of all the components
		CompType ntype_[TEXTURE_MAX_COMPONENTS];
		//the index of current mask
		int nmask_;
		//the index of current label
		int nlabel_;
		//! bytes per texel for each component.
		int nb_[TEXTURE_MAX_COMPONENTS]; 
		//! offset into volume texture
		int ox_, oy_, oz_; 
		//! data axis sizes (not necessarily pow2)
		int mx_, my_, mz_; 
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
		char *brkdata_;
		bool loading_;
		int id_in_loadedbrks;
		bool disp_;

		static std::map<std::wstring, std::wstring> cache_table_;
	};

	inline double TextureBrick::get_data(unsigned int i, unsigned int j, unsigned int k)
	{
		unsigned long long offset =
			(unsigned long long)(oz() + k) *
			(unsigned long long)(sx()) *
			(unsigned long long)(sy()) +
			(unsigned long long)(oy() + j) *
			(unsigned long long)(sx()) +
			(unsigned long long)(ox() + i);
		if (nb_[0] == 1)
		{
			unsigned char *ptr = (unsigned char*)(data_[0]->data);
			return ptr[offset] / 255.0;
		}
		else
		{
			unsigned short *ptr = (unsigned short*)(data_[0]->data);
			return ptr[offset] / 65535.0;
		}
		return 0.0;
	}

	struct Pyramid_Level {
		std::vector<FileLocInfo *> *filenames;
		int filetype;
		Nrrd* data;
		std::vector<TextureBrick *> bricks;
		//some information
		//total size
		int szx;
		int szy;
		int szz;
		//typical brick size
		int bszx;
		int bszy;
		int bszz;
		//brick num along axes
		int bnx;
		int bny;
		int bnz;
	};
} // namespace flvr

#endif // Volume_TextureBrick_h
