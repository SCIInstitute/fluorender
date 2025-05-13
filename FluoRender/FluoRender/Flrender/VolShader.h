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

#ifndef VolShader_h
#define VolShader_h

#include <string>
#include <vector>

namespace flvr
{
	class ShaderProgram;

	class VolShader
	{
	public:
		VolShader(bool poly, int channels,
				bool shading, bool fog,
				int peel, bool clip,
				bool grad, int mask, bool mip,
				int color_mode, int colormap, int colormap_proj,
				bool solid, int vertex_shader);
		~VolShader();

		bool create();

		inline bool poly() {return poly_; }
		inline int channels() { return channels_; }
		inline bool shading() { return shading_; }
		inline bool fog() { return fog_; }
		inline int peel() { return peel_; }
		inline bool clip() { return clip_; }
		inline bool grad() {return grad_; }
		inline int mask() {return mask_;}
		inline bool mip() { return mip_; }
		inline int color_mode() {return color_mode_;}
		inline int colormap() {return colormap_;}
		inline int colormap_proj() {return colormap_proj_;}
		inline bool solid() {return solid_;}

		inline bool match(bool poly, int channels,
						bool shading, bool fog,
						int peel, bool clip,
						bool grad, int mask, bool mip,
						int color_mode, int colormap, int colormap_proj,
						bool solid, int vertex_shader)
		{ 
			return (poly_ == poly &&
				channels_ == channels &&
				shading_ == shading && 
				fog_ == fog && 
				peel_ == peel &&
				clip_ == clip &&
				grad_ == grad &&
				mask_ == mask &&
				mip_ == mip &&
				color_mode_ == color_mode &&
				colormap_ == colormap &&
				colormap_proj_ == colormap_proj &&
				solid_ == solid &&
				vertex_type_ == vertex_shader); 
		}

		inline ShaderProgram* program() { return program_; }

	protected:
		bool emit_f(std::string& s);
		bool emit_v(std::string& s);
		std::string get_colormap_code();
		std::string get_colormap_proj();

		bool poly_;
		int channels_;
		bool shading_;
		bool fog_;
		int peel_;
		bool clip_;
		bool grad_;
		int mask_;	//0-normal, 1-render with mask, 2-render with mask excluded
					//3-random color with label, 4-random color with label+mask
		bool mip_;
		int color_mode_;//0-normal; 1-rainbow; 2-depth
		int colormap_;//index
		int colormap_proj_;	//projection direction
							//4D colormap: >=7
		bool solid_;//no transparency
		int vertex_type_;

		ShaderProgram* program_;
	};

	class VolShaderFactory
	{
	public:
		VolShaderFactory();
		~VolShaderFactory();
		void clear();

		ShaderProgram* shader(bool poly, int channels,
								bool shading, bool fog,
								int peel, bool clip,
								bool grad, int mask, bool mip,
								int color_mode, int colormap, int colormap_proj,
								bool solid, int vertex_type);
		//mask: 0-no mask, 1-segmentation mask, 2-labeling mask
		//color_mode: 0-normal; 1-rainbow; 2-depth

	protected:
		std::vector<VolShader*> shader_;
		int prev_shader_;
	};

} // end namespace flvr

#endif // VolShader_h
