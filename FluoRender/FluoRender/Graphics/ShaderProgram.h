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
#include <functional>
#include <unordered_map>
#include <sstream>
#include <iostream>
#include <unordered_set>

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

	enum class RenderMode : int
	{
		Disabled,
		Standard,
		Mip,
		Slice,
		Overlay
	};

	enum class ColorMode : int
	{
		SingleColor,
		Colormap
	};

	enum class ColormapProj : int
	{
		Disabled,//0: new
		Intensity,//previously 0
		ZValue,
		YValue,
		XValue,
		TValue,
		Gradient,
		Normal,
		IntDelta,//previously 7
		Speed
	};

	struct ShaderParams
	{
		int type = 0;//img, lightfield, mesh(0:normal; 1:integer), seg, cal
		int colormap = 0;//img, vol
		int peel = 0;//mesh(0:no peeling; 1:peel positive; 2:peel both; -1:peel negative), seg, vol
		bool tex = false;//mesh
		bool fog = false;//mesh, vol
		bool shading = false;//mesh(from light), seg, vol
		bool normal = false;//mesh(0:use normal from mesh; 1:generate normal in geom shader)
		bool color = false;//mesh
		int paint_mode = 0;//seg
		int hr_mode = 0;//seg
		bool use_2d = false;//seg
		bool clip = false;//seg, vol
		bool use_dir = false;//seg
		bool poly = false;//vol
		int channels = 0;//vol
		bool grad = false;//vol
		int mask = 0;//vol(0-normal, 1-render with mask, 2-render with mask excluded)
				 //(3-random color with label, 4-random color with label+mask)
		RenderMode render_mode = RenderMode::Standard;//vol
		ColorMode color_mode = ColorMode::SingleColor;//vol(0-normal; 1-rainbow; 2-depth)
		ColormapProj colormap_proj = ColormapProj::Intensity;//vol(projection direction, 4D colormap: >=7)
		bool solid = false;//vol(no transparency)
		int vertex_type = 0;//vol
		bool depth = false;//vol, if render a depth map

		static bool ValidColormapProj(ColormapProj p)
		{
			return p != ColormapProj::Disabled;
		}

		static bool IsTimeProj(ColormapProj p)
		{
			static const std::unordered_set<ColormapProj> time_modes = {
				ColormapProj::IntDelta,
				ColormapProj::Speed
			};
			return time_modes.count(p) > 0;
		}

		static ShaderParams Img(int type, int colormap)
		{
			ShaderParams p;
			p.type = type;
			p.colormap = colormap;
			return p;
		}

		static ShaderParams LightField(int type)
		{
			ShaderParams p;
			p.type = type;
			return p;
		}

		static ShaderParams Mesh(
			int type,
			int peel,
			bool clip,
			bool tex,
			bool fog,
			bool shading,
			bool normal,
			bool color
		)
		{
			ShaderParams p;
			p.type = type;
			p.peel = peel;
			p.clip = clip;
			p.tex = tex;
			p.fog = fog;
			p.shading = shading;
			p.normal = normal;
			p.color = color;
			return p;
		}

		static ShaderParams Seg(
			int type,
			int paint_mode,
			int hr_mode,
			bool use_2d,
			bool shading,
			int peel,
			bool clip,
			bool use_dir
		)
		{
			ShaderParams p;
			p.type = type;
			p.paint_mode = paint_mode;
			p.hr_mode = hr_mode;
			p.use_2d = use_2d;
			p.shading = shading;
			p.peel = peel;
			p.clip = clip;
			p.use_dir = use_dir;
			return p;
		}

		static ShaderParams VolCal(int type)
		{
			ShaderParams p;
			p.type = type;
			return p;
		}

		static ShaderParams Volume(
			bool poly,
			int channels,
			bool shading,
			bool fog,
			int peel,
			bool clip,
			bool grad,
			int mask,
			RenderMode render_mode,
			ColorMode color_mode,
			int colormap,
			ColormapProj colormap_proj,
			bool solid,
			int vertex_type,
			bool depth
		)
		{
			ShaderParams p;
			p.poly = poly;
			p.channels = channels;
			p.shading = shading;
			p.fog = fog;
			p.peel = peel;
			p.clip = clip;
			p.grad = grad;
			p.mask = mask;
			p.render_mode = render_mode;
			p.color_mode = color_mode;
			p.colormap = colormap;
			p.colormap_proj = colormap_proj;
			p.solid = solid;
			p.vertex_type = vertex_type;
			p.depth = depth;
			return p;
		}

		std::string to_string()
		{
			std::ostringstream oss;
			oss << "ShaderParams{type=" << type
				<< ", colormap=" << colormap
				<< ", peel=" << peel
				<< ", tex=" << tex
				<< ", fog=" << fog
				<< ", shading=" << shading
				<< ", normal=" << normal
				<< ", color=" << color
				<< ", paint_mode=" << paint_mode
				<< ", hr_mode=" << hr_mode
				<< ", use_2d=" << use_2d
				<< ", clip=" << clip
				<< ", use_dir=" << use_dir
				<< ", poly=" << poly
				<< ", channels=" << channels
				<< ", grad=" << grad
				<< ", mask=" << mask
				<< ", render_mode=" << static_cast<int>(render_mode)
				<< ", color_mode=" << static_cast<int>(color_mode)
				<< ", colormap_proj=" << static_cast<int>(colormap_proj)
				<< ", solid=" << solid
				<< ", vertex_type=" << vertex_type
				<< ", depth=" << depth
				<< "}";
			return oss.str();
		}
	};

	inline bool operator==(const ShaderParams& a, const ShaderParams& b)
	{
		return a.type == b.type &&
			a.colormap == b.colormap &&
			a.peel == b.peel &&
			a.tex == b.tex &&
			a.fog == b.fog &&
			a.shading == b.shading &&
			a.normal == b.normal &&
			a.color == b.color &&
			a.paint_mode == b.paint_mode &&
			a.hr_mode == b.hr_mode &&
			a.use_2d == b.use_2d &&
			a.clip == b.clip &&
			a.use_dir == b.use_dir &&
			a.poly == b.poly &&
			a.channels == b.channels &&
			a.grad == b.grad &&
			a.mask == b.mask &&
			a.render_mode == b.render_mode &&
			a.color_mode == b.color_mode &&
			a.colormap_proj == b.colormap_proj &&
			a.solid == b.solid &&
			a.vertex_type == b.vertex_type &&
			a.depth == b.depth;
	}

	class ShaderProgramFactory
	{
	public:
		virtual ~ShaderProgramFactory() = default;

		virtual void clear() { shader_map_.clear(); }
		virtual std::shared_ptr<ShaderProgram> shader(const ShaderParams& params) = 0;

	protected:
		virtual bool emit_v(const ShaderParams& params, std::string& s) = 0;
		virtual bool emit_g(const ShaderParams& params, std::string& s)
		{
			s.clear();
			return false;
		}
		virtual bool emit_f(const ShaderParams& params, std::string& s) = 0;

	protected:
		struct ShaderParamsHasher
		{
			size_t operator()(const ShaderParams& p) const
			{
				size_t h = 0;
				auto hash_combine = [&h](auto val) {
					h ^= std::hash<decltype(val)>{}(val)+0x9e3779b9 + (h << 6) + (h >> 2);
					};

				hash_combine(p.type);
				hash_combine(p.colormap);
				hash_combine(p.peel);
				hash_combine(p.tex);
				hash_combine(p.fog);
				hash_combine(p.shading);
				hash_combine(p.normal);
				hash_combine(p.color);
				hash_combine(p.paint_mode);
				hash_combine(p.hr_mode);
				hash_combine(p.use_2d);
				hash_combine(p.clip);
				hash_combine(p.use_dir);
				hash_combine(p.poly);
				hash_combine(p.channels);
				hash_combine(p.grad);
				hash_combine(p.mask);
				hash_combine(p.render_mode);
				hash_combine(p.color_mode);
				hash_combine(p.colormap_proj);
				hash_combine(p.solid);
				hash_combine(p.vertex_type);
				hash_combine(p.depth);

				return h;
			}
		};

		std::unordered_map<ShaderParams, std::shared_ptr<ShaderProgram>, ShaderParamsHasher> shader_map_;
	};

	class ShaderProgramManager
	{
	public:
		template<typename FactoryType>
		void add_factory(const std::string& name)
		{
			factories_[name] = std::make_unique<FactoryType>();
		}

		std::shared_ptr<ShaderProgram> shader(const std::string& factory_name, const ShaderParams& params)
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
