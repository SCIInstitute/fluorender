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

#ifndef SLIVR_TextureBrick_h
#define SLIVR_TextureBrick_h

#include "GL/glew.h"
#include "Ray.h"
#include "BBox.h"
#include "Plane.h"

#include <vector>
#include <nrrd.h>
#include <stdint.h>

namespace FLIVR {

	using std::vector;

	// We use no more than 2 texture units.
	// GL_MAX_TEXTURE_UNITS is the actual maximum.
	//we now added maximum to 4
	//which can include mask volumes
#define TEXTURE_MAX_COMPONENTS	4
	//these are the render modes used to determine if each mode is drawn
#define TEXTURE_RENDER_MODES	5

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
			int mx, int my, int mz, const BBox& bbox, const BBox& tbox,
			unsigned int id);
		virtual ~TextureBrick();

		inline BBox &bbox() { return bbox_; }

		inline int nx() { return nx_; }
		inline int ny() { return ny_; }
		inline int nz() { return nz_; }
		inline int nc() { return nc_; }
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

		void compute_polygons(Ray& view, double tmin, double tmax, double dt,
			vector<float>& vertex, vector<uint32_t>& index, vector<uint32_t>& size);
		void compute_polygons(Ray& view, double dt,
			vector<float>& vertex, vector<uint32_t>& index,
			vector<uint32_t>& size, bool bricks=false);
		
		//set d
		void set_d(double d) { d_ = d; }
		//sorting function
		static bool sort_asc(const TextureBrick* b1, const TextureBrick* b2)
		{ return b1->d_ > b2->d_; }
		static bool sort_dsc(const TextureBrick* b1, const TextureBrick* b2)
		{ return b2->d_ > b1->d_; }

		//current index
		inline void set_ind(size_t ind) {ind_ = ind;}
		inline size_t get_ind() {return ind_;}

		//id for analysis
		inline unsigned int get_id() { return id_; }

		//get value
		double get_data(unsigned int i, unsigned int j, unsigned int k);

		//get skip mask
		bool get_skip_mask()
		{ return skip_mask_; }
		void set_skip_mask(bool value)
		{ skip_mask_ = value; }
		void reset_skip_mask()
		{ skip_mask_ = false; }

	private:
		void compute_edge_rays(BBox &bbox);
		void compute_edge_rays_tex(BBox &bbox);
		size_t tex_type_size(GLenum t);
		GLenum tex_type_aux(Nrrd* n);

		//! bbox edges
		Ray edge_[12]; 
		//! tbox edges
		Ray tex_edge_[12]; 
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
		BBox bbox_, tbox_; 
		Vector view_vector_;
		//a value used for sorting
		//usually distance
		double d_;
		//priority level
		int priority_;//now, 0:highest
		//if it's been drawn in a full update loop
		bool drawn_[TEXTURE_RENDER_MODES];
		//current index in the queue, for reverse searching
		size_t ind_;
		//id for analysis
		unsigned int id_;
		//skip mask updating
		bool skip_mask_;
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

} // namespace FLIVR

#endif // Volume_TextureBrick_h
