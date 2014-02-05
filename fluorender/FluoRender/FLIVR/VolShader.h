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
//    File   : VolShader.h
//    Author : Milan Ikits
//    Date   : Tue Jul 13 02:27:58 2004

#ifndef VolShader_h
#define VolShader_h

#include <string>
#include <vector>

namespace FLIVR
{

	class FragmentProgram;

	class VolShader
	{
	public:
		VolShader(int channels,
				bool shading, bool fog,
				int peel, bool clip,
				bool hiqual, int mask,
				int color_mode, bool solid);
		~VolShader();

		bool create();

		inline int channels() { return channels_; }
		inline bool shading() { return shading_; }
		inline bool fog() { return fog_; }
		inline int peel() { return peel_; }
		inline bool clip() { return clip_; }
		inline bool hiqual() {return hiqual_; }
		inline int mask() {return mask_;}
		inline int color_mode() {return color_mode_;}
		inline bool solid() {return solid_;}

		inline bool match(int channels, 
						bool shading, bool fog, 
						int peel, bool clip,
						bool hiqual, int mask,
						int color_mode, bool solid)
		{ 
			return (channels_ == channels &&
				shading_ == shading && 
				fog_ == fog && 
				peel_ == peel &&
				clip_ == clip &&
				hiqual_ == hiqual &&
				mask_ == mask &&
				color_mode_ == color_mode &&
				solid_ == solid); 
		}

		inline FragmentProgram* program() { return program_; }

	protected:
		bool emit(std::string& s);

		int channels_;
		bool shading_;
		bool fog_;
		int peel_;
		bool clip_;
		bool hiqual_;
		int mask_;	//0-normal, 1-render with mask, 2-render with mask excluded
					//3-random color with label, 4-random color with label+mask
		int color_mode_;//0-normal; 1-rainbow; 2-depth
		bool solid_;//no transparency

		FragmentProgram* program_;
	};

	class VolShaderFactory
	{
	public:
		VolShaderFactory();
		~VolShaderFactory();

		FragmentProgram* shader(int channels, 
								bool shading, bool fog, 
								int peel, bool clip,
								bool hiqual, int mask,
								int color_mode, bool solid);
		//mask: 0-no mask, 1-segmentation mask, 2-labeling mask
		//color_mode: 0-normal; 1-rainbow; 2-depth

	protected:
		std::vector<VolShader*> shader_;
		int prev_shader_;
	};

} // end namespace FLIVR

#endif // VolShader_h
