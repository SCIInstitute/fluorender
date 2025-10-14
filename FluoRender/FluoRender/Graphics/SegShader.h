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

#ifndef SegShader_h
#define SegShader_h

#include <ShaderProgram.h>

//type definitions
#define SEG_SHDR_INITIALIZE	1	//initialize the segmentation fragment shader
#define SEG_SHDR_DB_GROW	2	//diffusion based grow

namespace flvr
{
	struct SegShaderParams : public ShaderParams
	{
		int type;
		int paint_mode;
		int hr_mode;
		bool use_2d;
		bool shading;
		int peel;
		bool clip;
		bool use_dir;

		bool operator==(const SegShaderParams& other) const
		{
			return
				type == other.type &&
				paint_mode == other.paint_mode &&
				hr_mode == other.hr_mode &&
				use_2d == other.use_2d &&
				shading == other.shading &&
				peel == other.peel &&
				clip == other.clip &&
				use_dir == other.use_dir;
		}

		size_t hash() const override {
			size_t h = 0;
			ShaderUtils::hash_combine(h, std::hash<int>{}(type));
			ShaderUtils::hash_combine(h, std::hash<int>{}(paint_mode));
			ShaderUtils::hash_combine(h, std::hash<int>{}(hr_mode));
			ShaderUtils::hash_combine(h, std::hash<int>{}(use_2d));
			ShaderUtils::hash_combine(h, std::hash<int>{}(shading));
			ShaderUtils::hash_combine(h, std::hash<int>{}(peel));
			ShaderUtils::hash_combine(h, std::hash<int>{}(clip));
			ShaderUtils::hash_combine(h, std::hash<int>{}(use_dir));
			return h;
		}

		bool equals(const ShaderParams& other) const override {
			if (auto* o = dynamic_cast<const SegShaderParams*>(&other))
				return *this == *o;
			return false;
		}
	};

	class SegShaderFactory : public ShaderProgramFactory
	{
	public:
		ShaderProgram* shader(const ShaderParams& base) override;

	protected:
		virtual bool emit_v(const ShaderParams& params, std::string& s) override;
		virtual bool emit_g(const ShaderParams& params, std::string& s) override;
		virtual bool emit_f(const ShaderParams& params, std::string& s) override;

	private:
		std::unordered_map<SegShaderParams, std::unique_ptr<ShaderProgram>, ShaderParamsKeyHasher> cache_;
	};

} // end namespace flvr

#endif // SegShader_h
