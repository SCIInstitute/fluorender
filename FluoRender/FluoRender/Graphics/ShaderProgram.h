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

#ifndef ShaderProgram_h
#define ShaderProgram_h

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace flvr
{
	class ShaderProgram
	{
	public:
		ShaderProgram(const std::string& vert_shader,
			const std::string& frag_shader,
			const std::string& geom_shader);
		ShaderProgram(const std::string& vert_shader,
			const std::string& frag_shader);
		ShaderProgram(const std::string& frag_shader);
		~ShaderProgram();

		unsigned int id();
		bool create();
		bool valid();
		void destroy();

		void bind();
		void bind_frag_data_location(int color_num, const char* name);
		void unbind();

		//set vector uniform (4x1)
		void setLocalParam(int, double, double, double, double);
		//set matrix uniform (4x4)
		void setLocalParamMatrix(int, float*);
		//set integer
		void setLocalParamUInt(int, unsigned int);
		//set integer vector (4x1)
		void setLocalParamInt4(int, int, int, int, int);

		// Call init_shaders_supported before shaders_supported queries!
		static bool init();
		static void init_shaders_supported();
		static bool shaders_supported();
		static int max_texture_size();
		static void set_max_texture_size(int size);
		static void reset_max_texture_size();
		static bool texture_non_power_of_two();
		static void set_no_tex_upack(bool val);
		static const int MAX_SHADER_UNIFORMS = 32;
		static std::string glsl_version_;
		static std::string glsl_unroll_;
		static bool no_tex_unpack_;

	protected:
		unsigned int id_;
		std::string  vert_shader_;
		std::string  frag_shader_;
		std::string geom_shader_;
		bool use_geom_shader_;

		//validation
		bool valid_;

		//locations
		int loc_ui[MAX_SHADER_UNIFORMS];
		int loc_vec4[MAX_SHADER_UNIFORMS];
		int loc_mat4[MAX_SHADER_UNIFORMS];
		int loc_int4[MAX_SHADER_UNIFORMS];

		static bool init_;
		static bool supported_;
		static bool non_2_textures_;
		static int  max_texture_size_;
		static int v_major_;
		static int v_minor_;
	};

	struct ShaderParams
	{
		virtual ~ShaderParams() = default;
		virtual std::string to_key() const = 0;
		virtual std::unique_ptr<ShaderParams> clone() const = 0;
	};

	struct ShaderEntry
	{
		std::unique_ptr<ShaderParams> params;
		std::unique_ptr<ShaderProgram> program;
	};

	class ShaderProgramFactory
	{
	public:
		virtual ~ShaderProgramFactory() = default;

		virtual void clear() { shader_map_.clear(); }
		virtual ShaderProgram* shader(const ShaderParams& params) = 0;

	protected:
		virtual bool emit_v(const ShaderParams& params, std::string& s) = 0;
		virtual bool emit_g(const ShaderParams& params, std::string& s)
		{
			s.clear();
			return false;
		}
		virtual bool emit_f(const ShaderParams& params, std::string& s) = 0;

	protected:
		std::unordered_map<std::string, ShaderEntry> shader_map_;
	};

	class ShaderProgramManager
	{
	public:
		template<typename FactoryType>
		void add_factory(const std::string& name)
		{
			factories_[name] = std::make_unique<FactoryType>();
		}

		ShaderProgram* shader(const std::string& factory_name, const ShaderParams& params)
		{
			auto it = factories_.find(factory_name);
			if (it != factories_.end())
			{
				return it->second->shader(params);
			}
			return nullptr;
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<ShaderProgramFactory>> factories_;
	};

} // end namespace flvr

#endif // ShaderProgram_h
