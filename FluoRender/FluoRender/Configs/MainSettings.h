/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#ifndef _MAINSETTINGS_H_
#define _MAINSETTINGS_H_

#define TOOL_PAINT_BRUSH	1
#define TOOL_MEASUREMENT	2
#define TOOL_TRACKING		3
#define TOOL_NOISE_REDUCTION	4
#define TOOL_VOLUME_SIZE	5
#define TOOL_COLOCALIZATION	6
#define TOOL_CONVERT		7
#define TOOL_OPENCL			8
#define TOOL_COMPONENT		9
#define TOOL_CALCULATIONS	10
#define TOOL_MACHINE_LEARNING	11

#include <BrushDefault.h>
#include <ComponentDefault.h>
#include <OutAdjDefault.h>
#include <ViewDefault.h>
#include <VolumeDefault.h>
#include <wx/string.h>
#include <string>

class MainSettings
{
public:
	MainSettings();
	~MainSettings();

	void Read();
	void Save();

	std::vector<std::string> GetJvmArgs()
	{
		std::vector<std::string> args;
		args.push_back(m_jvm_path.ToStdString());
		args.push_back(m_ij_path.ToStdString());
		args.push_back(m_bioformats_path.ToStdString());
		return args;
	}
	BrushDefault m_brush_def;
	ComponentDefault m_comp_def;
	OutAdjDefault m_outadj_def;
	ViewDefault m_view_def;
	VolumeDataDefault m_vol_def;

public:
	//default values

	bool m_prj_save;		//save project automatically
	bool m_prj_save_inc;	//save project incrementally
	wxString m_time_id;		//identfier for time sequence
	bool m_save_compress;	//save tif compressed
	bool m_override_vox;
	int m_last_tool;		//last tool

	bool m_slice_sequence;	//if slices are sequence
	bool m_chann_sequence;	//read channels
	int m_digit_order;		//digit order
	int m_ser_num;			//series number
	bool m_skip_brick;		//brick skipping
	bool m_load_mask;		//load volume mask
	bool m_save_crop;		//save crop
	int m_save_filter;		//filter
	bool m_vrp_embed;		//embed files in project
	bool m_save_alpha;		//save alpha channel in captured images
	bool m_save_float;		//save float values in captured images
	double m_dpi;			//dpi number of captured image
	bool m_realtime_compress;//real time compress
	double m_mov_bitrate;	//bitrate for mov export (Mbits)
	wxString m_mov_filename;//file name for mov export

	bool m_run_script;		//script
	bool m_script_break;	//allow script break
	wxString m_script_file;

	bool m_inverse_slider;	//invert vertical sliders
	int m_mulfunc;			//multifunction button use
	double m_time_span;		//time span to find sliders undo together

	bool m_test_speed;		//test fps
	bool m_test_param;		//using parameter test window
	bool m_test_wiref;		//draw wireframe of volumes

	int m_peeling_layers;	//peeling layer number
	bool m_micro_blend;		//blending slice in depth mode
	bool m_grad_bg;
	bool m_mouse_int;		//enable lower sample rate for mouse interactions
	double m_pin_threshold;	//rot center anchor thresh
	double m_soft_threshold;
	double m_line_width;	//line width
	int m_plane_mode;		//clipping plane display mode

	bool m_shadow_dir;		//enable directional shaow
	double m_shadow_dir_x;	//x comp of shadow direction
	double m_shadow_dir_y;	//y comp of shadow direction

	int m_wav_color1;		//wavelength to color
	int m_wav_color2;		//1-red; 2-green; 3-blue; 4-purple; 5-white
	int m_wav_color3;
	int m_wav_color4;

	int m_disp_id;			//display
	bool m_stereo;			//stereo
	bool m_sbs;
	double m_eye_dist;
	bool m_stay_top;		//full screen
	bool m_show_cursor;

	wxString m_font_file;	//font lib file in the Fonts folder
	int m_text_size;		//text size in viewport
	int m_text_color;		//text color: 0- contrast to bg; 1-same as bg; 2-volume sec color

	bool m_ruler_use_transf;//ruler use transfer function
	double m_ruler_relax_f1;//ruler relax
	double m_ruler_infr;	//ruler influence range
	int m_ruler_relax_iter;
	bool m_ruler_auto_relax;
	int m_ruler_relax_type;
	bool m_ruler_df_f;		//ruler exports df/f
	int m_ruler_size_thresh;//grow ruler size thresh
	double m_pencil_dist;	//distance between two points for pencil tool
	int m_point_volume_mode;//point volume mode

	bool m_mem_swap;		//enable memory swap
	double m_graphics_mem;	//in MB
							//it's the user setting
							//final value is determined by both reading from the card and this value
	double m_large_data_size;//data size considered as large and needs forced bricking
	int m_force_brick_size;	//in pixels
							//it's the user setting
							//final value is determined by both reading from the card and this value
	int m_up_time;			//response time in ms
	int m_update_order;		//0:back-to-front; 1:front-to-back
	bool m_invalidate_tex;	//invalidate texture in every loop
	int m_detail_level_offset;//an offset value to current level of detail (for multiresolution data only)

	int m_bg_type;			//background parameters: 0-mean; 1-minmax; 2-median
	int m_kx, m_ky;			//windows size
	double m_varth, m_gauth;//thresholds

	bool m_pvxml_flip_x;	//flip pvxml frame
	bool m_pvxml_flip_y;
	int m_pvxml_seq_type;

	int m_api_type;			//pixel format//0-default; 1-amd; 2-nv
	int m_red_bit;
	int m_green_bit;
	int m_blue_bit;
	int m_alpha_bit;
	int m_depth_bit;
	int m_samples;
	int m_gl_major_ver;		//context attrib
	int m_gl_minor_ver;
	int m_gl_profile_mask;
	bool m_use_max_texture_size;//max texture size
	int m_max_texture_size;
	bool m_no_tex_pack;		//no tex pack
	int m_cl_platform_id;	//cl device
	int m_cl_device_id;

	int m_track_iter;		//tracking settings
	double m_component_size;
	bool m_consistent_color;
	bool m_try_merge;
	bool m_try_split;
	double m_contact_factor;
	double m_similarity;

	wxString m_jvm_path;	// java settings strings.
	wxString m_ij_path;
	wxString m_bioformats_path;
	int m_ij_mode;//0: imagej; 1: fiji

	std::string m_cg_table;	//machine learning settings
	std::string m_vp_table;
	bool m_ml_auto_start_all;
	bool m_cg_auto_start;
	bool m_vp_auto_start;
	bool m_vp_auto_apply;

	//python settings
	int m_python_ver;//minor version no of python3
};
#endif
