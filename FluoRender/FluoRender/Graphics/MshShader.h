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

#ifndef MshShader_h
#define MshShader_h

#include <ShaderProgram.h>

namespace flvr
{
	struct MeshShaderParams : public ShaderParams
	{
		int type;	//0:normal; 1:integer
		int peel;	//0:no peeling; 1:peel positive; 2:peel both; -1:peel negative
		bool tex;
		bool fog;
		bool light;
		bool normal;//0:use normal from mesh; 1:generate normal in geom shader
		bool color;//vertex color

		bool operator==(const MeshShaderParams& other) const
		{
			return
				type == other.type &&
				peel == other.peel &&
				tex == other.tex &&
				fog == other.fog &&
				light == other.light &&
				normal == other.normal &&
				color == other.color;
		}

		size_t hash() const override {
			size_t h = 0;
			ShaderUtils::hash_combine(h, std::hash<int>{}(type));
			ShaderUtils::hash_combine(h, std::hash<int>{}(peel));
			ShaderUtils::hash_combine(h, std::hash<int>{}(tex));
			ShaderUtils::hash_combine(h, std::hash<int>{}(fog));
			ShaderUtils::hash_combine(h, std::hash<int>{}(light));
			ShaderUtils::hash_combine(h, std::hash<int>{}(normal));
			ShaderUtils::hash_combine(h, std::hash<int>{}(color));
			return h;
		}

		bool equals(const ShaderParams& other) const override {
			if (auto* o = dynamic_cast<const MeshShaderParams*>(&other))
				return *this == *o;
			return false;
		}
	};

	class MeshShaderFactory : public ShaderProgramFactory
	{
	public:
		ShaderProgram* shader(const ShaderParams& base) override;

	protected:
		virtual bool emit_v(const ShaderParams& params, std::string& s) override;
		virtual bool emit_g(const ShaderParams& params, std::string& s) override;
		virtual bool emit_f(const ShaderParams& params, std::string& s) override;

	private:
		std::unordered_map<MeshShaderParams, std::unique_ptr<ShaderProgram>, ShaderParamsKeyHasher> cache_;
	};

} // end namespace flvr

#endif // MshShader_h
