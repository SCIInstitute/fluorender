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

#include <ShaderProgram.h>

namespace flvr
{
	struct VolShaderParams : public ShaderParams
	{
		bool poly;
		int channels;
		bool shading;
		bool fog;
		int peel;
		bool clip;
		bool grad;
		int mask;	//0-normal, 1-render with mask, 2-render with mask excluded
					//3-random color with label, 4-random color with label+mask
		bool mip;
		int color_mode;//0-normal; 1-rainbow; 2-depth
		int colormap;//index
		int colormap_proj;	//projection direction
							//4D colormap: >=7
		bool solid;//no transparency
		int vertex_type;

		bool operator==(const VolShaderParams& other) const
		{
			return
				poly == other.poly &&
				channels == other.channels &&
				shading == other.shading &&
				fog == other.fog &&
				peel == other.peel &&
				clip == other.clip &&
				grad == other.grad &&
				mask == other.mask &&
				mip == other.mip &&
				color_mode == other.color_mode &&
				colormap == other.colormap &&
				colormap_proj == other.colormap_proj &&
				solid == other.solid &&
				vertex_type == other.vertex_type;
		}

		size_t hash() const override {
			size_t h = 0;
			ShaderUtils::hash_combine(h, std::hash<int>{}(poly));
			ShaderUtils::hash_combine(h, std::hash<int>{}(channels));
			ShaderUtils::hash_combine(h, std::hash<int>{}(shading));
			ShaderUtils::hash_combine(h, std::hash<int>{}(fog));
			ShaderUtils::hash_combine(h, std::hash<int>{}(peel));
			ShaderUtils::hash_combine(h, std::hash<int>{}(clip));
			ShaderUtils::hash_combine(h, std::hash<int>{}(grad));
			ShaderUtils::hash_combine(h, std::hash<int>{}(mask));
			ShaderUtils::hash_combine(h, std::hash<int>{}(mip));
			ShaderUtils::hash_combine(h, std::hash<int>{}(color_mode));
			ShaderUtils::hash_combine(h, std::hash<int>{}(colormap));
			ShaderUtils::hash_combine(h, std::hash<int>{}(colormap_proj));
			ShaderUtils::hash_combine(h, std::hash<int>{}(solid));
			ShaderUtils::hash_combine(h, std::hash<int>{}(vertex_type));
			return h;
		}

		bool equals(const ShaderParams& other) const override {
			if (auto* o = dynamic_cast<const VolShaderParams*>(&other))
				return *this == *o;
			return false;
		}
	};

	class VolShaderFactory : public ShaderProgramFactory
	{
	public:
		ShaderProgram* shader(const ShaderParams& base) override;

	protected:
		virtual bool emit_v(const ShaderParams& params, std::string& s) override;
		virtual bool emit_g(const ShaderParams& params, std::string& s) override;
		virtual bool emit_f(const ShaderParams& params, std::string& s) override;

		std::string get_colormap_code(int colormap, int colormap_proj);
		std::string get_colormap_proj(int colormap_proj);

	private:
		std::unordered_map<VolShaderParams, std::unique_ptr<ShaderProgram>, ShaderParamsKeyHasher> cache_;
	};

} // end namespace flvr

#endif // VolShader_h
