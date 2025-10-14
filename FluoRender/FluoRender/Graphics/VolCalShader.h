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

#ifndef VolCalShader_h
#define VolCalShader_h

#include <ShaderProgram.h>

#define CAL_SUBSTRACTION	1	//initialize the segmentation fragment shader
#define CAL_ADDITION		2	//diffusion based grow
#define CAL_DIVISION		3	//initialize the labeling fragment shader
#define CAL_INTERSECTION	4	//minimum of two
#define CAL_APPLYMASK		5	//apply mask to volume
#define CAL_APPLYMASKINV	6	//apply the inverted mask
#define CAL_APPLYMASKINV2	7	//apply the inverted mask
#define CAL_INTERSECTION_WITH_MASK	8	//minimum of two with mask

namespace flvr
{
	struct VolCalShaderParams : public ShaderParams
	{
		int type;

		bool operator==(const VolCalShaderParams& other) const {
			return type == other.type;
		}

		size_t hash() const override {
			size_t h = 0;
			ShaderUtils::hash_combine(h, std::hash<int>{}(type));
			return h;
		}

		bool equals(const ShaderParams& other) const override {
			if (auto* o = dynamic_cast<const VolCalShaderParams*>(&other))
				return *this == *o;
			return false;
		}
	};

	class VolCalShaderFactory : public ShaderProgramFactory
	{
	public:
		ShaderProgram* shader(const ShaderParams& base) override;

	protected:
		virtual bool emit_v(const ShaderParams& params, std::string& s) override;
		virtual bool emit_g(const ShaderParams& params, std::string& s) override;
		virtual bool emit_f(const ShaderParams& params, std::string& s) override;

	private:
		std::unordered_map<VolCalShaderParams, std::unique_ptr<ShaderProgram>, ShaderParamsKeyHasher> cache_;
	};

} // end namespace flvr

#endif // VolCalShader_h
