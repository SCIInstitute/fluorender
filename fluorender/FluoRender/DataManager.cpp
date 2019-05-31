/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#include "DataManager.h"
#include "Calculate/VolumeSampler.h"
#include "teem/Nrrd/nrrd.h"
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include <wx/stdpaths.h>
#include "utility.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>
#include <glm/gtc/matrix_transform.hpp>
#include <FLIVR/Quaternion.h>

#ifdef _WIN32
#  undef min
#  undef max
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double TreeLayer::m_sw = 0.0;

TreeLayer::TreeLayer()
{
	type = -1;
	m_gamma = Color(1.0, 1.0, 1.0);
	m_brightness = Color(1.0, 1.0, 1.0);
	m_hdr = Color(0.0, 0.0, 0.0);
	m_sync_r = m_sync_g = m_sync_b = false;
}

TreeLayer::~TreeLayer()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VolumeData::VolumeData()
{
	m_reader = 0;

	m_dup = false;
	m_dup_counter = 0;
	m_dup_data = 0;

	type = 2;//volume

	//volume renderer and texture
	m_vr = 0;
	m_tex = 0;

	//current channel index
	m_chan = -1;
	m_time = 0;

	//mdoes
	m_mode = 0;
	//stream modes
	m_stream_mode = 0;

	//mask mode
	m_mask_mode = 0;
	m_use_mask_threshold = false;

	//volume properties
	m_scalar_scale = 1.0;
	m_gm_scale = 1.0;
	m_max_value = 255.0;
	//gamma
	m_gamma3d = 1.0;
	m_gm_thresh = 0.0;
	m_offset = 1.0;
	m_lo_thresh = 0.0;
	m_hi_thresh = 1.0;
	m_color = Color(1.0, 1.0, 1.0);
	SetHSV();
	m_alpha = 1.0;
	m_sample_rate = 1.0;
	m_mat_amb = 1.0;
	m_mat_diff = 1.0;
	m_mat_spec = 1.0;
	m_mat_shine = 10;
	//noise reduction
	m_noise_rd = false;
	//shading
	m_shading = false;
	//shadow
	m_shadow = false;
	m_shadow_darkness = 0.0;

	//resolution, scaling, spacing
	m_res_x = 0;	m_res_y = 0;	m_res_z = 0;
	m_sclx = 1.0;	m_scly = 1.0;	m_sclz = 1.0;
	m_spcx = 1.0;	m_spcy = 1.0;	m_spcz = 1.0;
	m_spc_from_file = false;

	//display control
	m_disp = true;
	m_draw_bounds = false;
	m_test_wiref = false;

	//colormap mode
	m_colormap_mode = 0;
	m_colormap_disp = false;
	m_colormap_low_value = 0.0;
	m_colormap_hi_value = 1.0;
	m_colormap = 0;
	m_colormap_proj = 0;

	//blend mode
	m_blend_mode = 0;

	m_saved_mode = 0;

	m_2d_mask = 0;
	m_2d_weight1 = 0;
	m_2d_weight2 = 0;
	m_2d_dmap = 0;

	//clip distance
	m_clip_dist_x = 0;
	m_clip_dist_y = 0;
	m_clip_dist_z = 0;

	//compression
	m_compression = false;
	//resize
	m_resize = false;
	m_rnx = 0;
	m_rny = 0;
	m_rnz = 0;

	//skip brick
	m_skip_brick = false;

	//legend
	m_legend = true;
	//interpolate
	m_interpolate = true;

	//valid brick number
	m_brick_num = 0;

	//save label
	m_label_save = 0;
}

VolumeData::VolumeData(VolumeData &copy)
{
	m_reader = 0;
	//duplication
	m_dup = true;
	copy.m_dup_counter++;
	m_dup_counter = copy.m_dup_counter;
	if (copy.m_dup_data)
		m_dup_data = copy.m_dup_data;
	else
		m_dup_data = &copy;

	//layer properties
	type = 2;//volume
	SetName(copy.GetName()+wxString::Format("_%d", m_dup_counter));
	SetGamma(copy.GetGamma());
	SetBrightness(copy.GetBrightness());
	SetHdr(copy.GetHdr());
	SetSyncR(copy.GetSyncR());
	SetSyncG(copy.GetSyncG());
	SetSyncB(copy.GetSyncB());

	//path and bounds
	m_tex_path = copy.m_tex_path;
	m_bounds = copy.m_bounds;

	//volume renderer and texture
	m_vr = new VolumeRenderer(*copy.m_vr);
	m_tex = copy.m_tex;

	//current channel index
	m_chan = copy.m_chan;
	m_time = 0;

	//mdoes
	m_mode = copy.m_mode;
	//stream modes
	m_stream_mode = copy.m_stream_mode;

	//mask mode
	m_mask_mode = copy.m_mask_mode;
	m_use_mask_threshold = false;

	//volume properties
	m_scalar_scale = copy.m_scalar_scale;
	m_gm_scale = copy.m_gm_scale;
	m_max_value = copy.m_max_value;
	//gamma
	m_gamma3d = copy.m_gamma3d;
	m_gm_thresh = copy.m_gm_thresh;
	m_offset = copy.m_offset;
	m_lo_thresh = copy.m_lo_thresh;
	m_hi_thresh = copy.m_hi_thresh;
	m_color = copy.m_color;
	SetHSV();
	m_alpha = copy.m_alpha;
	m_sample_rate = copy.m_sample_rate;
	m_mat_amb = copy.m_mat_amb;
	m_mat_diff = copy.m_mat_diff;
	m_mat_spec = copy.m_mat_spec;
	m_mat_shine = copy.m_mat_shine;
	//noise reduction
	m_noise_rd = copy.m_noise_rd;
	//shading
	m_shading = copy.m_shading;
	//shadow
	m_shadow = copy.m_shadow;
	m_shadow_darkness = copy.m_shadow_darkness;

	//resolution, scaling, spacing
	m_res_x = copy.m_res_x;	m_res_y = copy.m_res_y;	m_res_z = copy.m_res_z;
	m_sclx = copy.m_sclx;	m_scly = copy.m_scly;	m_sclz = copy.m_sclz;
	m_spcx = copy.m_spcx;	m_spcy = copy.m_spcy;	m_spcz = copy.m_spcz;
	m_spc_from_file = copy.m_spc_from_file;

	//display control
	m_disp = copy.m_disp;
	m_draw_bounds = copy.m_draw_bounds;
	m_test_wiref = copy.m_test_wiref;

	//colormap mode
	m_colormap_mode = copy.m_colormap_mode;
	m_colormap_disp = copy.m_colormap_disp;
	m_colormap_low_value = copy.m_colormap_low_value;
	m_colormap_hi_value = copy.m_colormap_hi_value;
	m_colormap = copy.m_colormap;
	m_colormap_proj = copy.m_colormap_proj;

	//blend mode
	m_blend_mode = copy.m_blend_mode;

	m_saved_mode = copy.m_saved_mode;

	m_2d_mask = 0;
	m_2d_weight1 = 0;
	m_2d_weight2 = 0;
	m_2d_dmap = 0;

	//clip distance
	m_clip_dist_x = 0;
	m_clip_dist_y = 0;
	m_clip_dist_z = 0;

	//compression
	m_compression = false;
	//resize
	m_resize = false;
	m_rnx = 0;
	m_rny = 0;
	m_rnz = 0;

	//skip brick
	m_skip_brick = false;

	//legend
	m_legend = true;

	//interpolate
	m_interpolate = copy.m_interpolate;

	//valid brick number
	m_brick_num = 0;

	//save label
	m_label_save = 0;
}

VolumeData::~VolumeData()
{
	if (m_vr)
		delete m_vr;
	if (m_tex && !m_dup)
		delete m_tex;
	if (m_label_save)
		delete[] m_label_save;
}

//set viewport
void VolumeData::SetViewport(GLint vp[4])
{
	if (m_vr)
		m_vr->set_viewport(vp);
}

//set clear color
void VolumeData::SetClearColor(GLfloat clear_color[4])
{
	if (m_vr)
		m_vr->set_clear_color(clear_color);
}

//set current framebuffer
void VolumeData::SetCurFramebuffer(GLuint cur_framebuffer)
{
	if (m_vr)
		m_vr->set_cur_framebuffer(cur_framebuffer);
}

//duplication
bool VolumeData::GetDup()
{
	return m_dup;
}

//increase duplicate counter
void VolumeData::IncDupCounter()
{
	m_dup_counter++;
}

//get from
VolumeData* VolumeData::GetDupData()
{
	return m_dup_data;
}

//data related
//compression
void VolumeData::SetCompression(bool compression)
{
	m_compression = compression;
	if (m_vr)
		m_vr->set_compression(compression);
}

bool VolumeData::GetCompression()
{
	return m_compression;
}

//skip brick
void VolumeData::SetSkipBrick(bool skip)
{
	m_skip_brick = skip;
}

bool VolumeData::GetSkipBrick()
{
	return m_skip_brick;
}

int VolumeData::Load(Nrrd* data, wxString &name, wxString &path)
{
	if (!data || data->dim!=3)
		return 0;

	m_tex_path = path;
	m_name = name;

	if (m_tex)
	{
		delete m_tex;
		m_tex = NULL;
	}

	Nrrd *nv = data;
	Nrrd *gm = 0;
	m_res_x = nv->axis[0].size;
	m_res_y = nv->axis[1].size;
	m_res_z = nv->axis[2].size;

	BBox bounds;
	Point pmax(data->axis[0].max, data->axis[1].max, data->axis[2].max);
	Point pmin(data->axis[0].min, data->axis[1].min, data->axis[2].min);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;

	m_tex = new Texture();
	m_tex->set_use_priority(m_skip_brick);
	if (m_reader && m_reader->GetType()==READER_BRKXML_TYPE)
	{
		BRKXMLReader *breader = (BRKXMLReader*)m_reader;
		vector<Pyramid_Level> pyramid;
		vector<vector<vector<vector<FileLocInfo *>>>> fnames;
		int ftype = BRICK_FILE_TYPE_NONE;

		breader->build_pyramid(pyramid, fnames, 0, breader->GetCurChan());
		m_tex->SetCopyableLevel(breader->GetCopyableLevel());

		int lmnum = breader->GetLandmarkNum();
		for (int j = 0; j < lmnum; j++)
		{
			wstring name;
			VD_Landmark vlm;
			breader->GetLandmark(j, vlm.name, vlm.x, vlm.y, vlm.z, vlm.spcx, vlm.spcy, vlm.spcz);
			m_landmarks.push_back(vlm);
			breader->GetMetadataID(m_metadata_id);
		}
		if (!m_tex->buildPyramid(pyramid, fnames, breader->isURL())) return 0;
	}
	else
	{
		if (!m_tex->build(nv, gm, 0, 256, 0, 0))
			return 0;
	}

	if (m_tex)
	{
		if (m_vr)
			delete m_vr;

		vector<Plane*> planelist(0);
		Plane* plane = 0;
		//x
		plane = new Plane(Point(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
		planelist.push_back(plane);
		plane = new Plane(Point(1.0, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
		planelist.push_back(plane);
		//y
		plane = new Plane(Point(0.0, 0.0, 0.0), Vector(0.0, 1.0, 0.0));
		planelist.push_back(plane);
		plane = new Plane(Point(0.0, 1.0, 0.0), Vector(0.0, -1.0, 0.0));
		planelist.push_back(plane);
		//z
		plane = new Plane(Point(0.0, 0.0, 0.0), Vector(0.0, 0.0, 1.0));
		planelist.push_back(plane);
		plane = new Plane(Point(0.0, 0.0, 1.0), Vector(0.0, 0.0, -1.0));
		planelist.push_back(plane);

		m_vr = new VolumeRenderer(m_tex, planelist, true);
		m_vr->set_sampling_rate(m_sample_rate);
		m_vr->set_material(m_mat_amb, m_mat_diff, m_mat_spec, m_mat_shine);
		m_vr->set_shading(true);
		m_vr->set_scalar_scale(m_scalar_scale);
		m_vr->set_gm_scale(m_scalar_scale);

		SetMode(m_mode);
	}

	return 1;
}

int VolumeData::Replace(Nrrd* data, bool del_tex)
{
	if (!data || data->dim!=3)
		return 0;

	if (del_tex)
	{
		Nrrd *nv = data;
		Nrrd *gm = 0;
		m_res_x = nv->axis[0].size;
		m_res_y = nv->axis[1].size;
		m_res_z = nv->axis[2].size;

		if (m_tex)
			delete m_tex;
		m_tex = new Texture();
		m_tex->set_use_priority(m_skip_brick);
		m_tex->build(nv, gm, 0, m_max_value, 0, 0);
	}
	else
	{
		//set new
		m_tex->set_nrrd(data, 0);
	}

	//clear pool
	if (m_vr)
		m_vr->set_texture(m_tex);
	else
		return 0;

	return 1;
}

int VolumeData::Replace(VolumeData* data)
{
	if (!data ||
		m_res_x!=data->m_res_x ||
		m_res_y!=data->m_res_y ||
		m_res_z!=data->m_res_z)
		return 0;

	double spcx = 1.0, spcy = 1.0, spcz = 1.0;

	if (m_tex && m_vr)
	{
		m_tex->get_spacings(spcx, spcy, spcz);
		m_vr->clear_tex_current();
		delete m_tex;
		m_vr->reset_texture();
	}
	m_tex = data->GetTexture();
	data->SetTexture();
	SetScalarScale(data->GetScalarScale());
	SetGMScale(data->GetGMScale());
	SetMaxValue(data->GetMaxValue());
	if (m_vr)
		m_vr->set_texture(m_tex);
	else
		return 0;
	return 1;
}

//volume data
void VolumeData::AddEmptyData(int bits,
	int nx, int ny, int nz,
	double spcx, double spcy, double spcz,
	int brick_size)
{
	if (bits!=8 && bits!=16)
		return;

	if (m_vr)
		delete m_vr;
	if (m_tex)
		delete m_tex;

	Nrrd *nv = nrrdNew();
	if (bits == 8)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint8 *val8 = new (std::nothrow) uint8[mem_size];
		if (!val8)
		{
			wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		memset((void*)val8, 0, sizeof(uint8)*nx*ny*nz);
		nrrdWrap(nv, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	}
	else if (bits == 16)
	{
		unsigned long long mem_size = (unsigned long long)nx*
			(unsigned long long)ny*(unsigned long long)nz;
		uint16 *val16 = new (std::nothrow) uint16[mem_size];
		if (!val16)
		{
			wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		memset((void*)val16, 0, sizeof(uint16)*nx*ny*nz);
		nrrdWrap(nv, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	}
	nrrdAxisInfoSet(nv, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(nv, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
	nrrdAxisInfoSet(nv, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nv, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

	//resolution
	m_res_x = nv->axis[0].size;
	m_res_y = nv->axis[1].size;
	m_res_z = nv->axis[2].size;

	//bounding box
	BBox bounds;
	Point pmax(nv->axis[0].max, nv->axis[1].max, nv->axis[2].max);
	Point pmin(nv->axis[0].min, nv->axis[1].min, nv->axis[2].min);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;

	//create texture
	m_tex = new Texture();
	m_tex->set_use_priority(false);
	m_tex->set_brick_size(brick_size);
	m_tex->build(nv, 0, 0, 256, 0, 0);
	m_tex->set_spacings(spcx, spcy, spcz);

	//clipping planes
	vector<Plane*> planelist(0);
	Plane* plane = 0;
	//x
	plane = new Plane(Point(0.0, 0.0, 0.0), Vector(1.0, 0.0, 0.0));
	planelist.push_back(plane);
	plane = new Plane(Point(1.0, 0.0, 0.0), Vector(-1.0, 0.0, 0.0));
	planelist.push_back(plane);
	//y
	plane = new Plane(Point(0.0, 0.0, 0.0), Vector(0.0, 1.0, 0.0));
	planelist.push_back(plane);
	plane = new Plane(Point(0.0, 1.0, 0.0), Vector(0.0, -1.0, 0.0));
	planelist.push_back(plane);
	//z
	plane = new Plane(Point(0.0, 0.0, 0.0), Vector(0.0, 0.0, 1.0));
	planelist.push_back(plane);
	plane = new Plane(Point(0.0, 0.0, 1.0), Vector(0.0, 0.0, -1.0));
	planelist.push_back(plane);

	//create volume renderer
	m_vr = new VolumeRenderer(m_tex, planelist, true);
	m_vr->set_sampling_rate(m_sample_rate);
	m_vr->set_material(m_mat_amb, m_mat_diff, m_mat_spec, m_mat_shine);
	m_vr->set_shading(true);
	m_vr->set_scalar_scale(m_scalar_scale);
	m_vr->set_gm_scale(m_scalar_scale);

	SetMode(m_mode);
}

//volume mask
void VolumeData::LoadMask(Nrrd* mask)
{
	if (!mask || !m_tex || !m_vr)
		return;

	int nx2, ny2, nz2;
	nx2 = mask->axis[0].size;
	ny2 = mask->axis[1].size;
	nz2 = mask->axis[2].size;
	if (m_res_x != nx2 || m_res_y != ny2 || m_res_z != nz2)
	{
		double spcx, spcy, spcz;
		GetSpacings(spcx, spcy, spcz);
		FL::VolumeSampler sampler;
		sampler.SetVolume(mask);
		sampler.SetSize(m_res_x, m_res_y, m_res_z);
		sampler.SetSpacings(spcx, spcy, spcz);
		sampler.SetType(0);
		//sampler.SetFilterSize(2, 2, 0);
		sampler.Resize();
		nrrdNuke(mask);
		mask = sampler.GetResult();
	}

	//prepare the texture bricks for the mask
	m_tex->add_empty_mask();
	m_tex->set_nrrd(mask, m_tex->nmask());
}

void VolumeData::AddEmptyMask(int mode)
{
	if (!m_tex || !m_vr)
		return;

	Nrrd *nrrd_mask = 0;
	uint8 *val8 = 0;
	unsigned long long mem_size = (unsigned long long)m_res_x*
		(unsigned long long)m_res_y*(unsigned long long)m_res_z;
	//prepare the texture bricks for the mask
	bool empty = m_tex->add_empty_mask();
	if (empty)
	{
		//add the nrrd data for mask
		nrrd_mask = nrrdNew();
		val8 = new (std::nothrow) uint8[mem_size];
		if (!val8)
		{
			wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);

		m_tex->set_nrrd(nrrd_mask, m_tex->nmask());
	}
	else
	{
		nrrd_mask = m_tex->get_nrrd(m_tex->nmask());
		val8 = (uint8*)nrrd_mask->data;
	}

	if (mode == 0 || mode == 1)
	{
		if (val8)
			memset((void*)val8, mode ?
				255 : 0, mem_size*sizeof(uint8));
	}
	else if (mode == 2)
	{
		if (empty)
			memset((void*)val8, 0, mem_size*sizeof(uint8));
	}
}

//volume label
void VolumeData::LoadLabel(Nrrd* label)
{
	if (!label || !m_tex || !m_vr)
		return;

	int nx2, ny2, nz2;
	nx2 = label->axis[0].size;
	ny2 = label->axis[1].size;
	nz2 = label->axis[2].size;
	if (m_res_x != nx2 || m_res_y != ny2 || m_res_z != nz2)
	{
		double spcx, spcy, spcz;
		GetSpacings(spcx, spcy, spcz);
		FL::VolumeSampler sampler;
		sampler.SetVolume(label);
		sampler.SetSize(m_res_x, m_res_y, m_res_z);
		sampler.SetSpacings(spcx, spcy, spcz);
		sampler.Resize();
		nrrdNuke(label);
		label = sampler.GetResult();
	}

	m_tex->add_empty_label();
	m_tex->set_nrrd(label, m_tex->nlabel());
}

void VolumeData::SetOrderedID(unsigned int* val)
{
	for (int i=0; i<m_res_x; i++)
		for (int j=0; j<m_res_y; j++)
			for (int k=0; k<m_res_z; k++)
			{
				unsigned int index = m_res_y*m_res_z*i + m_res_z*j + k;
				val[index] = index+1;
			}
}

void VolumeData::SetReverseID(unsigned int* val)
{
	for (int i=0; i<m_res_x; i++)
		for (int j=0; j<m_res_y; j++)
			for (int k=0; k<m_res_z; k++)
			{
				unsigned int index = m_res_y*m_res_z*i + m_res_z*j + k;
				val[index] = m_res_x*m_res_y*m_res_z - index;
			}
}

void VolumeData::SetShuffledID(unsigned int* val)
{
	unsigned int x, y, z;
	unsigned int res;
	unsigned int len = 0;
	unsigned int r = Max(m_res_x, Max(m_res_y, m_res_z));
	while (r > 0)
	{
		r /= 2;
		len++;
	}
	for (int i=0; i<m_res_x; i++)
		for (int j=0; j<m_res_y; j++)
			for (int k=0; k<m_res_z; k++)
			{
				x = reverse_bit(i, len);
				y = reverse_bit(j, len);
				z = reverse_bit(k, len);
				res = 0;
				for (unsigned int ii=0; ii<len; ii++)
				{
					res |= (1<<ii & x)<<(2*ii);
					res |= (1<<ii & y)<<(2*ii+1);
					res |= (1<<ii & z)<<(2*ii+2);
				}
				unsigned int index = m_res_x*m_res_y*k + m_res_x*j + i;
				val[index] = m_res_x*m_res_y*m_res_z - res;
			}
}

void VolumeData::AddEmptyLabel(int mode)
{
	if (!m_tex || !m_vr)
		return;

	Nrrd *nrrd_label = 0;
	unsigned int *val32 = 0;
	//prepare the texture bricks for the labeling mask
	if (m_tex->add_empty_label())
	{
		//add the nrrd data for the labeling mask
		nrrd_label = nrrdNew();
		unsigned long long mem_size = (unsigned long long)m_res_x*
			(unsigned long long)m_res_y*(unsigned long long)m_res_z;
		val32 = new (std::nothrow) unsigned int[mem_size];
		if (!val32)
		{
			wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}

		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);

		m_tex->set_nrrd(nrrd_label, m_tex->nlabel());
	}
	else
	{
		nrrd_label = m_tex->get_nrrd(m_tex->nlabel());
		val32 = (unsigned int*)nrrd_label->data;
	}

	//apply values
	switch (mode)
	{
	case 0://zeros
		memset(val32, 0, sizeof(unsigned int)*m_res_x*m_res_y*m_res_z);
		break;
	case 1://ordered
		SetOrderedID(val32);
		break;
	case 2://shuffled
		SetShuffledID(val32);
		break;
	}

}

bool VolumeData::SearchLabel(unsigned int label)
{
	if (!m_tex)
		return false;

	Nrrd* nrrd_label = m_tex->get_nrrd(m_tex->nlabel());
	if (!nrrd_label)
		return false;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return false;

	unsigned long long for_size = (unsigned long long)m_res_x *
		(unsigned long long)m_res_y * (unsigned long long)m_res_z;
	for (unsigned long long index = 0; index < for_size; ++index)
		if (data_label[index] == label)
			return true;
	return false;
}

Nrrd* VolumeData::GetVolume(bool ret)
{
	if (m_vr && m_tex)
	{
		if (ret) m_vr->return_volume();
		return m_tex->get_nrrd(0);
	}

	return 0;
}

Nrrd* VolumeData::GetMask(bool ret)
{
	if (m_vr && m_tex && m_tex->nmask()!=-1)
	{
		if (ret) m_vr->return_mask();
		return m_tex->get_nrrd(m_tex->nmask());
	}

	return 0;
}

Nrrd* VolumeData::GetLabel(bool ret)
{
	if (m_vr && m_tex && m_tex->nlabel() != -1)
	{
		if (ret) m_vr->return_label();
		return m_tex->get_nrrd(m_tex->nlabel());
	}

	return 0;
}

double VolumeData::GetOriginalValue(int i, int j, int k, TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(0, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
	}
	else
	{
		Nrrd* data = 0;
		data = m_tex->get_nrrd(0);
		if (!data || !data->data) return 0.0;
		data_data = data->data;
		if (data->type == nrrdTypeUChar)
			bits = 8;
		else if (data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
		return 0.0;
	uint64_t ii = i, jj = j, kk = k;

	if (bits == 8)
	{
		uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
		uint8 old_value = ((uint8*)(data_data))[index];
		return double(old_value)/255.0;
	}
	else if (bits == 16)
	{
		uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
		uint16 old_value = ((uint16*)(data_data))[index];
		return double(old_value)*m_scalar_scale/65535.0;
	}

	return 0.0;
}

double VolumeData::GetTransferedValue(int i, int j, int k, TextureBrick* b)
{
	void *data_data = 0;
	int bits = 8;
	int64_t nx, ny, nz;

	if (isBrxml())
	{
		if (!b || !b->isLoaded()) return 0.0;
		FileLocInfo *finfo = m_tex->GetFileName(b->getID());
		data_data = b->tex_data_brk(0, finfo);
		if (!data_data) return 0.0;
		bits = b->nb(0) * 8;
		nx = b->nx();
		ny = b->ny();
		nz = b->nz();
	}
	else
	{
		Nrrd* data = 0;
		data = m_tex->get_nrrd(0);
		if (!data || !data->data) return 0.0;
		data_data = data->data;
		if (data->type == nrrdTypeUChar)
			bits = 8;
		else if (data->type == nrrdTypeUShort)
			bits = 16;
		nx = (int64_t)(data->axis[0].size);
		ny = (int64_t)(data->axis[1].size);
		nz = (int64_t)(data->axis[2].size);
	}

	if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
		return 0.0;
	int64_t ii = i, jj = j, kk = k;

	if (bits == 8)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint8 old_value = ((uint8*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)/255.0;
		if (m_vr->get_inversion())
			new_value = 1.0-new_value;
		if (i>0 && i<nx-1 &&
			j>0 && j<ny-1 &&
			k>0 && k<nz-1)
		{
			double v1 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii-1];
			double v2 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii+1];
			double v3 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj-1) + ii];
			double v4 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj+1) + ii];
			double v5 = ((uint8*)(data_data))[nx*ny*(kk-1) + nx*jj + ii];
			double v6 = ((uint8*)(data_data))[nx*ny*(kk+1) + nx*jj + ii];
			double normal_x, normal_y, normal_z;
			normal_x = (v2 - v1) / 255.0;
			normal_y = (v4 - v3) / 255.0;
			normal_z = (v6 - v5) / 255.0;
			gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<m_lo_thresh-m_sw ||
			new_value>m_hi_thresh+m_sw)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / m_gamma3d;
			new_value = (new_value<m_lo_thresh?
				(m_sw-m_lo_thresh+new_value)/m_sw:
			(new_value>m_hi_thresh?
				(m_sw-new_value+m_hi_thresh)/m_sw:1.0))
				*new_value;
			new_value *= (m_gm_thresh > 0.0 ?
				Clamp(gm / m_gm_thresh, 0.0,
					1.0 + m_gm_thresh*10.0) : 1.0);
			new_value = pow(Clamp(new_value/m_offset,
				gamma<1.0?-(gamma-1.0)*0.00001:0.0,
				gamma>1.0?0.9999:1.0), gamma);
			new_value *= m_alpha;
		}
		return new_value;
	}
	else if (bits == 16)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint16 old_value = ((uint16*)(data_data))[index];
		double gm = 0.0;
		double new_value = double(old_value)*m_scalar_scale/65535.0;
		if (m_vr->get_inversion())
			new_value = 1.0-new_value;
		if (ii>0 && ii<nx-1 &&
			jj>0 && jj<ny-1 &&
			kk>0 && kk<nz-1)
		{
			double v1 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii-1];
			double v2 = ((uint8*)(data_data))[nx*ny*kk + nx*jj + ii+1];
			double v3 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj-1) + ii];
			double v4 = ((uint8*)(data_data))[nx*ny*kk + nx*(jj+1) + ii];
			double v5 = ((uint8*)(data_data))[nx*ny*(kk-1) + nx*jj + ii];
			double v6 = ((uint8*)(data_data))[nx*ny*(kk+1) + nx*jj + ii];
			double normal_x, normal_y, normal_z;
			normal_x = (v2 - v1)*m_scalar_scale/65535.0;
			normal_y = (v4 - v3)*m_scalar_scale/65535.0;
			normal_z = (v6 - v5)*m_scalar_scale/65535.0;
			gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<m_lo_thresh-m_sw ||
			new_value>m_hi_thresh+m_sw)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / m_gamma3d;
			new_value = (new_value<m_lo_thresh?
				(m_sw-m_lo_thresh+new_value)/m_sw:
			(new_value>m_hi_thresh?
				(m_sw-new_value+m_hi_thresh)/m_sw:1.0))
				*new_value;
			new_value *= (m_gm_thresh > 0.0 ?
				Clamp(gm / m_gm_thresh, 0.0,
					1.0 + m_gm_thresh*10.0) : 1.0);
			new_value = pow(Clamp(new_value/m_offset,
				gamma<1.0?-(gamma-1.0)*0.00001:0.0,
				gamma>1.0?0.9999:1.0), gamma);
			new_value *= m_alpha;
		}
		return new_value;
	}

	return 0.0;
}

void VolumeData::SetResize(int resize, int nx, int ny, int nz)
{
	if (resize > -1)
		m_resize = resize > 0;
	if (nx > -1)
		m_rnx = nx;
	if (ny > -1)
		m_rny = ny;
	if (nz > -1)
		m_rnz = nz;
}

//save
void VolumeData::Save(wxString &filename, int mode, bool bake, bool compress)
{
	if (!m_vr || !m_tex)
		return;

	Nrrd* data = 0;
	bool delete_data = false;

	BaseWriter *writer = 0;
	switch (mode)
	{
	case 0://multi-page tiff
		writer = new TIFWriter();
		break;
	case 1://single-page tiff sequence
		writer = new TIFWriter();
		break;
	case 2://nrrd
		writer = new NRRDWriter();
		break;
	}

	double spcx, spcy, spcz;
	GetSpacings(spcx, spcy, spcz);

	//save data
	data = m_tex->get_nrrd(0);
	if (data)
	{
		if (bake)
		{
			wxProgressDialog *prg_diag = new wxProgressDialog(
				"FluoRender: Baking volume data...",
				"Baking volume data. Please wait.",
				100, 0, wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);

			//process the data
			int bits = data->type==nrrdTypeUShort?16:8;
			int nx = int(data->axis[0].size);
			int ny = int(data->axis[1].size);
			int nz = int(data->axis[2].size);

			//clipping planes
			vector<Plane*> *planes = m_vr->get_planes();

			Nrrd* baked_data = nrrdNew();
			if (bits == 8)
			{
				unsigned long long mem_size = (unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz;
				uint8 *val8 = new (std::nothrow) uint8[mem_size];
				if (!val8)
				{
					wxMessageBox("Not enough memory. Please save project and restart.");
					return;
				}
				//transfer function
				for (int i=0; i<nx; i++)
				{
					prg_diag->Update(95*(i+1)/nx);
					for (int j=0; j<ny; j++)
						for (int k=0; k<nz; k++)
						{
							int index = nx*ny*k + nx*j + i;
							bool clipped = false;
							Point p(double(i) / double(nx),
								double(j) / double(ny),
								double(k) / double(nz));
							for (int pi = 0; pi < 6; ++pi)
							{
								if ((*planes)[pi] &&
									(*planes)[pi]->eval_point(p) < 0.0)
								{
									val8[index] = 0;
									clipped = true;
								}
							}
							if (!clipped)
							{
								double new_value = GetTransferedValue(i, j, k);
								val8[index] = uint8(new_value*255.0);
							}
						}
				}
				nrrdWrap(baked_data, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
			}
			else if (bits == 16)
			{
				unsigned long long mem_size = (unsigned long long)nx*
					(unsigned long long)ny*(unsigned long long)nz;
				uint16 *val16 = new (std::nothrow) uint16[mem_size];
				if (!val16)
				{
					wxMessageBox("Not enough memory. Please save project and restart.");
					return;
				}
				//transfer function
				for (int i=0; i<nx; i++)
				{
					prg_diag->Update(95*(i+1)/nx);
					for (int j=0; j<ny; j++)
						for (int k=0; k<nz; k++)
						{
							int index = nx*ny*k + nx*j + i;
							bool clipped = false;
							Point p(double(i) / double(nx),
								double(j) / double(ny),
								double(k) / double(nz));
							for (int pi = 0; pi < 6; ++pi)
							{
								if ((*planes)[pi] &&
									(*planes)[pi]->eval_point(p) < 0.0)
								{
									val16[index] = 0;
									clipped = true;
								}
							}
							if (!clipped)
							{
								double new_value = GetTransferedValue(i, j, k);
								val16[index] = uint16(new_value*65535.0);
							}
						}
				}
				nrrdWrap(baked_data, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
			}
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoSpacing, spcx, spcy, spcz);
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
			nrrdAxisInfoSet(baked_data, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

			data = baked_data;
			delete_data = true;

			prg_diag->Update(100);
			delete prg_diag;
		}

		if (m_resize)
		{
			FL::VolumeSampler sampler;
			sampler.SetVolume(data);
			sampler.SetSize(m_rnx, m_rny, m_rnz);
			sampler.SetSpacings(spcx, spcy, spcz);
			sampler.SetType(0);
			//sampler.SetFilterSize(2, 2, 0);
			sampler.Resize();
			Nrrd* temp = data;
			data = sampler.GetResult();
			if (data)
			{
				spcx = data->axis[0].spacing;
				spcy = data->axis[1].spacing;
				spcz = data->axis[2].spacing;
			}
			if (delete_data)
				nrrdNuke(temp);
		}

		writer->SetData(data);
		writer->SetSpacings(spcx, spcy, spcz);
		writer->SetCompression(compress);
		writer->Save(filename.ToStdWstring(), mode);

		if (delete_data)
			nrrdNuke(data);
	}
	delete writer;

	m_tex_path = filename;
	SaveMask(false, 0, 0);
	SaveLabel(false, 0, 0);
}

void VolumeData::SaveMask(bool use_reader, int t, int c)
{
	if (!m_vr || !m_tex)
		return;

	Nrrd* data = 0;
	bool delete_data = false;
	double spcx, spcy, spcz;
	GetSpacings(spcx, spcy, spcz);

	//save mask
	if (m_tex->nmask() != -1)
	{
		m_vr->return_mask();
		data = m_tex->get_nrrd(m_tex->nmask());
		if (data)
		{
			if (m_resize)
			{
				FL::VolumeSampler sampler;
				sampler.SetVolume(data);
				sampler.SetSize(m_rnx, m_rny, m_rnz);
				sampler.SetSpacings(spcx, spcy, spcz);
				sampler.SetType(0);
				//sampler.SetFilterSize(2, 2, 0);
				sampler.Resize();
				data = sampler.GetResult();
				if (data)
				{
					spcx = data->axis[0].spacing;
					spcy = data->axis[1].spacing;
					spcz = data->axis[2].spacing;
					delete_data = true;
				}
			}

			MSKWriter msk_writer;
			msk_writer.SetData(data);
			msk_writer.SetSpacings(spcx, spcy, spcz);
			wstring filename;
			if (use_reader && m_reader)
				filename = m_reader->GetCurMaskName(t, c);
			else
				filename = m_tex_path.substr(0, m_tex_path.find_last_of('.')) + ".msk";
			msk_writer.Save(filename, 0);
			if (delete_data)
				nrrdNuke(data);
		}
	}
}

void VolumeData::SaveLabel(bool use_reader, int t, int c)
{
	if (!m_vr || !m_tex)
		return;

	Nrrd* data = 0;
	bool delete_data = false;
	double spcx, spcy, spcz;
	GetSpacings(spcx, spcy, spcz);

	//save label
	if (m_tex->nlabel() != -1)
	{
		data = m_tex->get_nrrd(m_tex->nlabel());
		if (data)
		{
			if (m_resize)
			{
				FL::VolumeSampler sampler;
				sampler.SetVolume(data);
				sampler.SetSize(m_rnx, m_rny, m_rnz);
				sampler.SetSpacings(spcx, spcy, spcz);
				sampler.Resize();
				data = sampler.GetResult();
				if (data)
				{
					spcx = data->axis[0].spacing;
					spcy = data->axis[1].spacing;
					spcz = data->axis[2].spacing;
					delete_data = true;
				}
			}

			MSKWriter msk_writer;
			msk_writer.SetData(data);
			msk_writer.SetSpacings(spcx, spcy, spcz);
			wstring filename;
			if (use_reader && m_reader)
				filename = m_reader->GetCurLabelName(t, c);
			else
				filename = m_tex_path.substr(0, m_tex_path.find_last_of('.')) + ".lbl";
			msk_writer.Save(filename, 1);
			if (delete_data)
				nrrdNuke(data);
		}
	}
}

//bounding box
BBox VolumeData::GetBounds()
{
	return m_bounds;
}

BBox VolumeData::GetClippedBounds()
{
	vector<Plane*> *planes = m_vr->get_planes();
	if (planes->size() != 6)
		return m_bounds;

	//calculating planes
	//get six planes
	Plane* px1 = (*planes)[0];
	Plane* px2 = (*planes)[1];
	Plane* py1 = (*planes)[2];
	Plane* py2 = (*planes)[3];
	Plane* pz1 = (*planes)[4];
	Plane* pz2 = (*planes)[5];

	Vector diff = m_bounds.max() - m_bounds.min();
	Point min = Point(m_bounds.min().x() - diff.x()*px1->d(),
		m_bounds.min().y() - diff.y()*py1->d(),
		m_bounds.min().z() - diff.z()*pz1->d());
	Point max = Point(m_bounds.min().x() + diff.x()*px2->d(),
		m_bounds.min().y() + diff.y()*py2->d(),
		m_bounds.min().z() + diff.z()*pz2->d());

	return BBox(min, max);
}

//path
void VolumeData::SetPath(wxString path)
{
	m_tex_path = path;
}

wxString VolumeData::GetPath()
{
	return m_tex_path;
}

//multi-channel
void VolumeData::SetCurChannel(int chan)
{
	m_chan = chan;
}

int VolumeData::GetCurChannel()
{
	return m_chan;
}

//time sequence
void VolumeData::SetCurTime(int time)
{
	m_time = time;
}

int VolumeData::GetCurTime()
{
	return m_time;
}

//MIP & normal modes
void VolumeData::SetMode(int mode)
{
	if (!m_vr)
		return;

	m_saved_mode = m_mode;
	m_mode = mode;

	switch (mode)
	{
	case 0://normal
		m_vr->set_mode(TextureRenderer::MODE_OVER);

		m_vr->set_color(m_color);
		m_vr->set_alpha(m_alpha);
		m_vr->set_lo_thresh(m_lo_thresh);
		m_vr->set_hi_thresh(m_hi_thresh);
		m_vr->set_gm_thresh(m_gm_thresh);
		m_vr->set_gamma3d(m_gamma3d);
		m_vr->set_offset(m_offset);
		break;
	case 1://MIP
		m_vr->set_mode(TextureRenderer::MODE_MIP);
		{
			double h, s, v;
			GetHSV(h, s, v);
			HSVColor hsv(h, s, 1.0);
			Color rgb = Color(hsv);
			m_vr->set_color(rgb);
		}
		m_vr->set_alpha(1.0);
		m_vr->set_lo_thresh(0.0);
		m_vr->set_hi_thresh(1.0);
		m_vr->set_gm_thresh(0.0);
		m_vr->set_gamma3d(m_gamma3d);
		m_vr->set_offset(m_offset);
		break;
	case 2://white shading
		m_vr->set_mode(TextureRenderer::MODE_OVER);
		m_vr->set_colormap_mode(0);

		m_vr->set_color(Color(1.0, 1.0, 1.0));
		m_vr->set_alpha(1.0);
		m_vr->set_lo_thresh(m_lo_thresh);
		m_vr->set_hi_thresh(m_hi_thresh);
		m_vr->set_gm_thresh(m_gm_thresh);
		m_vr->set_gamma3d(m_gamma3d);
		m_vr->set_offset(m_offset);
		break;
	case 3://white mip
		m_vr->set_mode(TextureRenderer::MODE_MIP);
		m_vr->set_colormap_mode(0);

		m_vr->set_color(Color(1.0, 1.0, 1.0));
		m_vr->set_alpha(1.0);
		m_vr->set_lo_thresh(0.0);
		m_vr->set_hi_thresh(1.0);
		m_vr->set_gm_thresh(0.0);
		m_vr->set_gamma3d(1.0);
		m_vr->set_offset(1.0);
		break;
	}
}

int VolumeData::GetMode()
{
	return m_mode;
}

void VolumeData::RestoreMode()
{
	SetMode(m_saved_mode);
}

//inversion
void VolumeData::SetInvert(bool mode)
{
	if (m_vr)
		m_vr->set_inversion(mode);
}

bool VolumeData::GetInvert()
{
	if (m_vr)
		return m_vr->get_inversion();
	else
		return false;
}

//mask mode
void VolumeData::SetMaskMode(int mode)
{
	m_mask_mode = mode;
	if (m_vr)
		m_vr->set_ml_mode(mode);
}

int VolumeData::GetMaskMode()
{
	return m_mask_mode;
}

//noise reduction
void VolumeData::SetNR(bool val)
{
	m_noise_rd = val;
	if (m_vr)
		m_vr->SetNoiseRed(val);
}

bool VolumeData::GetNR()
{
	return m_noise_rd;
}

//volumerenderer
VolumeRenderer *VolumeData::GetVR()
{
	return m_vr;
}

//texture
Texture* VolumeData::GetTexture()
{
	return m_tex;
}

void VolumeData::SetTexture()
{
	if (m_vr)
		m_vr->reset_texture();
	m_tex = 0;
}

void VolumeData::SetMatrices(glm::mat4 &mv_mat,
	glm::mat4 &proj_mat, glm::mat4 &tex_mat)
{
	glm::mat4 scale_mv = glm::scale(mv_mat, glm::vec3(m_sclx, m_scly, m_sclz));
	if (m_vr)
		m_vr->set_matrices(scale_mv, proj_mat, tex_mat);
}

//draw volume
void VolumeData::Draw(bool ortho, bool adaptive, bool interactive, double zoom)
{
	if (m_vr)
	{
		m_vr->draw(m_test_wiref, adaptive, interactive, ortho, zoom, m_stream_mode);
	}
	if (m_draw_bounds)
		DrawBounds();
}

void VolumeData::DrawBounds()
{
}

//draw mask (create the mask)
//type: 0-initial; 1-diffusion-based growing
//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear, 7-all
//			  11-posterize
//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
void VolumeData::DrawMask(int type, int paint_mode, int hr_mode,
	double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
	double w2d, double bins, int order, bool ortho, bool estimate)
{
	if (m_vr)
	{
		m_vr->set_2d_mask(m_2d_mask);
		m_vr->set_2d_weight(m_2d_weight1, m_2d_weight2);
		m_vr->draw_mask(type, paint_mode, hr_mode, ini_thresh, gm_falloff, scl_falloff, scl_translate, w2d, bins, ortho, estimate);
		if (GetAllBrickNum()>1 &&
			(order == 1 || order == 2))
		{
			//invalidate mask
			m_vr->return_mask(order);
			m_vr->clear_tex_mask();
		}
		if (estimate)
			m_est_thresh = m_vr->get_estimated_thresh();
	}
}

//draw label (create the label)
//type: 0-initialize; 1-maximum intensity filtering
//mode: 0-normal; 1-posterized; 2-copy values; 3-poster, copy
void VolumeData::DrawLabel(int type, int mode, double thresh, double gm_falloff)
{
	if (m_vr)
		m_vr->draw_label(type, mode, thresh, gm_falloff);
}

//calculation
void VolumeData::Calculate(int type, VolumeData *vd_a, VolumeData *vd_b)
{
	if (m_vr)
	{
		if (type==6 || type==7)
			m_vr->set_hi_thresh(vd_a->GetRightThresh());
		m_vr->calculate(type, vd_a?vd_a->GetVR():0, vd_b?vd_b->GetVR():0);
		m_vr->return_volume();
	}
}

//set 2d mask for segmentation
void VolumeData::Set2dMask(GLuint mask)
{
	m_2d_mask = mask;
}

//set 2d weight map for segmentation
void VolumeData::Set2DWeight(GLuint weight1, GLuint weight2)
{
	m_2d_weight1 = weight1;
	m_2d_weight2 = weight2;
}

//set 2d depth map for rendering shadows
void VolumeData::Set2dDmap(GLuint dmap)
{
	m_2d_dmap = dmap;
	if (m_vr)
		m_vr->set_2d_dmap(m_2d_dmap);
}

//properties
//transfer function
void VolumeData::Set3DGamma(double dVal)
{
	m_gamma3d = dVal;
	if (m_vr)
		m_vr->set_gamma3d(m_gamma3d);
}

double VolumeData::Get3DGamma()
{
	return m_gamma3d;
}

void VolumeData::SetBoundary(double dVal)
{
	m_gm_thresh = dVal;
	if (m_vr)
		m_vr->set_gm_thresh(m_gm_thresh);
}

double VolumeData::GetBoundary()
{
	return m_gm_thresh;
}

void VolumeData::SetOffset(double dVal)
{
	m_offset = dVal;
	if (m_vr)
		m_vr->set_offset(m_offset);
}

double VolumeData::GetOffset()
{
	return m_offset;
}

void VolumeData::SetLeftThresh(double dVal)
{
	m_lo_thresh = dVal;
	if (m_vr)
		m_vr->set_lo_thresh(m_lo_thresh);
}

double VolumeData::GetLeftThresh()
{
	return m_lo_thresh;
}

void VolumeData::SetRightThresh(double dVal)
{
	m_hi_thresh = dVal;
	if (m_vr)
		m_vr->set_hi_thresh(m_hi_thresh);
}

double VolumeData::GetRightThresh()
{
	return m_hi_thresh;
}

void VolumeData::SetColor(Color &color, bool update_hsv)
{
	m_color = color;
	if (update_hsv)
		SetHSV();
	if (m_vr)
		m_vr->set_color(color);
}

Color VolumeData::GetColor()
{
	return m_color;
}

void VolumeData::SetMaskColor(Color &color, bool set)
{
	if (m_vr)
		m_vr->set_mask_color(color, set);
}

Color VolumeData::GetMaskColor()
{
	Color result;
	if (m_vr)
		result = m_vr->get_mask_color();
	return result;
}

bool VolumeData::GetMaskColorSet()
{
	if (m_vr)
		return m_vr->get_mask_color_set();
	else
		return false;
}

void VolumeData::ResetMaskColorSet()
{
	if (m_vr)
		m_vr->reset_mask_color_set();
}

Color VolumeData::SetLuminance(double dVal)
{
	double h, s, v;
	GetHSV(h, s, v);
	HSVColor hsv(h, s, dVal);
	m_color = Color(hsv);
	if (m_vr)
		m_vr->set_color(m_color);
	return m_color;
}

double VolumeData::GetLuminance()
{
	HSVColor hsv(m_color);
	return hsv.val();
}

void VolumeData::SetAlpha(double alpha)
{
	m_alpha = alpha;
	if (m_vr)
		m_vr->set_alpha(m_alpha);
}

double VolumeData::GetAlpha()
{
	return m_alpha;
}

void VolumeData::SetEnableAlpha(bool mode)
{
	if (m_vr)
	{
		m_vr->set_solid(!mode);
		if (mode)
			m_vr->set_alpha(m_alpha);
		else
			m_vr->set_alpha(1.0);
	}
}

bool VolumeData::GetEnableAlpha()
{
	if (m_vr)
		return !m_vr->get_solid();
	else
		return true;
}

void VolumeData::SetHSV(double hue, double sat, double val)
{
	if (hue < 0 || sat < 0 || val < 0)
	{
		m_hsv = HSVColor(m_color);
	}
	else
	{
		m_hsv = HSVColor(hue, sat, val);
	}
}

void VolumeData::GetHSV(double &hue, double &sat, double &val)
{
	hue = m_hsv.hue();
	sat = m_hsv.sat();
	val = m_hsv.val();
}

//mask threshold
void VolumeData::SetMaskThreshold(double thresh)
{
	if (m_use_mask_threshold && m_vr)
		m_vr->set_mask_thresh(thresh);
}

void VolumeData::SetUseMaskThreshold(bool mode)
{
	m_use_mask_threshold = mode;
	if (m_vr && !m_use_mask_threshold)
		m_vr->set_mask_thresh(0.0);
}

//shading
void VolumeData::SetShading(bool bVal)
{
	m_shading = bVal;
	if (m_vr)
		m_vr->set_shading(bVal);
}

bool VolumeData::GetShading()
{
	return m_shading;
}

//shadow
void VolumeData::SetShadow(bool bVal)
{
	m_shadow = bVal;
}

bool VolumeData::GetShadow()
{
	return m_shadow;
}

void VolumeData::SetShadowParams(double val)
{
	m_shadow_darkness = val;
}

void VolumeData::GetShadowParams(double &val)
{
	val = m_shadow_darkness;
}

void VolumeData::SetMaterial(double amb, double diff, double spec, double shine)
{
	m_mat_amb = amb;
	m_mat_diff = diff;
	m_mat_spec = spec;
	m_mat_shine = shine;
	if (m_vr)
		m_vr->set_material(m_mat_amb, m_mat_diff, m_mat_spec, m_mat_shine);
}

void VolumeData::GetMaterial(double &amb, double &diff, double &spec, double &shine)
{
	amb = m_mat_amb;
	diff = m_mat_diff;
	spec = m_mat_spec;
	shine = m_mat_shine;
}

void VolumeData::SetLowShading(double dVal)
{
	double amb, diff, spec, shine;
	GetMaterial(amb, diff, spec, shine);
	SetMaterial(dVal, diff, spec, shine);
}

void VolumeData::SetHiShading(double dVal)
{
	double amb, diff, spec, shine;
	GetMaterial(amb, diff, spec, shine);
	SetMaterial(amb, diff, spec, dVal);
}

//sample rate
void VolumeData::SetSampleRate(double rate)
{
	m_sample_rate = rate;
	if (m_vr)
		m_vr->set_sampling_rate(m_sample_rate);
}

double VolumeData::GetSampleRate()
{
	return m_sample_rate;
}

//colormap mode
void VolumeData::SetColormapMode(int mode)
{
	m_colormap_mode = mode;
	if (m_vr)
	{
		m_vr->set_colormap_mode(m_colormap_mode);
		m_vr->set_color(m_color);
	}
}

int VolumeData::GetColormapMode()
{
	return m_colormap_mode;
}

void VolumeData::SetColormapDisp(bool disp)
{
	m_colormap_disp = disp;
}

bool VolumeData::GetColormapDisp()
{
	return m_colormap_disp;
}

void VolumeData::SetColormapValues(double low, double high)
{
	m_colormap_low_value = low;
	m_colormap_hi_value = high;
	if (m_vr)
		m_vr->set_colormap_values(
		m_colormap_low_value, m_colormap_hi_value);
}

void VolumeData::GetColormapValues(double &low, double &high)
{
	low = m_colormap_low_value;
	high = m_colormap_hi_value;
}

void VolumeData::SetColormap(int value)
{
	m_colormap = value;
	if (m_vr)
		m_vr->set_colormap(m_colormap);
}

void VolumeData::SetColormapProj(int value)
{
	m_colormap_proj = value;
	if (m_vr)
		m_vr->set_colormap_proj(m_colormap_proj);
}

int VolumeData::GetColormap()
{
	return m_colormap;
}

int VolumeData::GetColormapProj()
{
	return m_colormap_proj;
}

Color VolumeData::GetColorFromColormap(double value)
{
	Color color;
	double v = (value - m_colormap_low_value) /
		(m_colormap_hi_value - m_colormap_low_value);
	switch (m_colormap)
	{
	case 0:
	default:
		color.r(Clamp(4.0*v - 2.0, 0.0, 1.0));
		color.g(Clamp(v<0.5 ? 4.0*v : -4.0*v + 4.0, 0.0, 1.0));
		color.b(Clamp(-4.0*v + 2.0, 0.0, 1.0));
		break;
	case 1:
		color.r(Clamp(-4.0*v + 2.0, 0.0, 1.0));
		color.g(Clamp(v<0.5 ? 4.0*v : -4.0*v + 4.0, 0.0, 1.0));
		color.b(Clamp(4.0*v - 2.0, 0.0, 1.0));
		break;
	case 2:
		color.r(Clamp(2.0*v, 0.0, 1.0));
		color.g(Clamp(4.0*v - 2.0, 0.0, 1.0));
		color.b(Clamp(4.0*v - 3.0, 0.0, 1.0));
		break;
	case 3:
		color.r(Clamp(v, 0.0, 1.0));
		color.g(Clamp(1.0 - v, 0.0, 1.0));
		color.b(1.0);
		break;
	case 4:
		color.r(Clamp(v<0.5 ? v*0.9 + 0.25 : 0.7, 0.0, 1.0));
		color.g(Clamp(v<0.5 ? v*0.8 + 0.3 : (1.0 - v)*1.4, 0.0, 1.0));
		color.b(Clamp(v<0.5 ? v*(-0.1) + 0.75 : (1.0 - v)*1.1 + 0.15, 0.0, 1.0));
		break;
	case 5:
		color.r(Clamp(v, 0.0, 1.0));
		color.g(Clamp(v, 0.0, 1.0));
		color.b(Clamp(v, 0.0, 1.0));
		break;
	case 6:
		color.r(1.0 - Clamp(v, 0.0, 1.0));
		color.g(1.0 - Clamp(v, 0.0, 1.0));
		color.b(1.0 - Clamp(v, 0.0, 1.0));
		break;
	}
	return color;
}

//resolution  scaling and spacing
void VolumeData::GetResolution(int &res_x, int &res_y, int &res_z, int lv)
{
	if (lv >= 0 && isBrxml() && m_tex)
	{
		res_x = m_tex->nx();
		res_y = m_tex->ny();
		res_z = m_tex->nz();
	}
	else
	{
		res_x = m_res_x;
		res_y = m_res_y;
		res_z = m_res_z;
	}
}

void VolumeData::SetScalings(double sclx, double scly, double sclz)
{
	m_sclx = sclx; m_scly = scly; m_sclz = sclz;
}

void VolumeData::GetScalings(double &sclx, double &scly, double &sclz)
{
	sclx = m_sclx; scly = m_scly; sclz = m_sclz;
}

void VolumeData::SetSpacings(double spcx, double spcy, double spcz)
{
	if (GetTexture())
	{
		GetTexture()->set_spacings(spcx, spcy, spcz);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

void VolumeData::GetSpacings(double &spcx, double &spcy, double & spcz, int lv)
{
	if (GetTexture())
		GetTexture()->get_spacings(spcx, spcy, spcz, lv);
}

void VolumeData::GetFileSpacings(double &spcx, double &spcy, double &spcz)
{
	spcx = m_spcx; spcy = m_spcy; spcz = m_spcz;
}

//brkxml
void VolumeData::SetBaseSpacings(double spcx, double spcy, double spcz)
{
	if (GetTexture())
	{
		GetTexture()->set_base_spacings(spcx, spcy, spcz);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

void VolumeData::GetBaseSpacings(double &spcx, double &spcy, double & spcz)
{
	if (GetTexture())
		GetTexture()->get_base_spacings(spcx, spcy, spcz);
}

void VolumeData::SetSpacingScales(double s_spcx, double s_spcy, double s_spcz)
{
	if (GetTexture())
	{
		GetTexture()->set_spacing_scales(s_spcx, s_spcy, s_spcz);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

void VolumeData::GetSpacingScales(double &s_spcx, double &s_spcy, double &s_spcz)
{
	if (GetTexture())
		GetTexture()->get_spacing_scales(s_spcx, s_spcy, s_spcz);
}

void VolumeData::SetLevel(int lv)
{
	if (GetTexture() && isBrxml())
	{
		GetTexture()->setLevel(lv);
		m_bounds.reset();
		GetTexture()->get_bounds(m_bounds);
	}
}

int VolumeData::GetLevel()
{
	if (GetTexture() && isBrxml())
		return GetTexture()->GetCurLevel();
	else
		return -1;
}

int VolumeData::GetLevelNum()
{
	if (GetTexture() && isBrxml())
		return GetTexture()->GetLevelNum();
	else
		return -1;
}

//bits
int VolumeData::GetBits()
{
	if (!m_tex)
		return 0;
	Nrrd* nrrd_data = m_tex->get_nrrd(0);
	if (!nrrd_data)
		return 0;
	if (nrrd_data->type == nrrdTypeUChar)
		return 8;
	else if (nrrd_data->type == nrrdTypeUShort)
		return 16;
	return 0;
}

//display controls
void VolumeData::SetDisp(bool disp)
{
	m_disp = disp;
	GetTexture()->set_sort_bricks();
}

bool VolumeData::GetDisp()
{
	return m_disp;
}

void VolumeData::ToggleDisp()
{
	m_disp = !m_disp;
	GetTexture()->set_sort_bricks();
}

//bounding box
void VolumeData::SetDrawBounds(bool draw)
{
	m_draw_bounds = draw;
}

bool VolumeData::GetDrawBounds()
{
	return m_draw_bounds;
}

void VolumeData::ToggleDrawBounds()
{
	m_draw_bounds = !m_draw_bounds;
}

//wireframe
void VolumeData::SetWireframe(bool val)
{
	m_test_wiref = val;
}

//blend mode
void VolumeData::SetBlendMode(int mode)
{
	m_blend_mode = mode;
}

int VolumeData::GetBlendMode()
{
	return m_blend_mode;
}

//clip distance
void VolumeData::SetClipDistance(int distx, int disty, int distz)
{
	m_clip_dist_x = distx;
	m_clip_dist_y = disty;
	m_clip_dist_z = distz;
}

void VolumeData::GetClipDistance(int &distx, int &disty, int &distz)
{
	distx = m_clip_dist_x;
	disty = m_clip_dist_y;
	distz = m_clip_dist_z;
}

//randomize color
void VolumeData::RandomizeColor()
{
	double hue = (double)rand()/(RAND_MAX) * 360.0;
	Color color(HSVColor(hue, 1.0, 1.0));
	SetColor(color);
}

//shown in legend
void VolumeData::SetLegend(bool val)
{
	m_legend = val;
}

bool VolumeData::GetLegend()
{
	return m_legend;
}

//interpolate
void VolumeData::SetInterpolate(bool val)
{
	if (m_vr)
		m_vr->set_interpolate(val);
	m_interpolate = val;
}

bool VolumeData::GetInterpolate()
{
	return m_interpolate;
}

void VolumeData::SetFog(bool use_fog,
	double fog_intensity, double fog_start, double fog_end)
{
	if (m_vr)
		m_vr->set_fog(use_fog, fog_intensity, fog_start, fog_end);
}

int VolumeData::GetAllBrickNum()
{
	if (!m_tex)
		return 0;
	return m_tex->get_brick_num();
}

bool VolumeData::isBrxml()
{
	if (!m_tex) return false;

	return m_tex->isBrxml();
}

//save label
void VolumeData::PushLabel(bool ret)
{
	if (ret && m_vr)
		m_vr->return_label();
	if (!m_tex)
		return;
	if (m_tex->nlabel() == -1)
		return;

	Nrrd* data = m_tex->get_nrrd(m_tex->nlabel());
	if (!data || ! data->data)
		return;
	int nx, ny, nz;
	GetResolution(nx, ny, nz);
	unsigned long long size = (unsigned long long)nx * ny * nz;
	if (!m_label_save)
		m_label_save = new unsigned int[size];
	memcpy(m_label_save, data->data, size * sizeof(unsigned int));
}

void VolumeData::PopLabel()
{
	delete[] m_label_save;
	m_label_save = 0;
}

void VolumeData::LoadLabel2()
{
	if (m_label_save)
	{
		Nrrd* data = m_tex->get_nrrd(m_tex->nlabel());
		if (!data || !data->data)
			return;
		int nx, ny, nz;
		GetResolution(nx, ny, nz);
		unsigned long long size = (unsigned long long)nx * ny * nz;
		memcpy(data->data, m_label_save, size * sizeof(unsigned int));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeshData::MeshData() :
m_data(0),
	m_mr(0),
	m_center(0.0, 0.0, 0.0),
	m_disp(true),
	m_draw_bounds(false),
	m_light(true),
	m_mat_amb(0.3, 0.3, 0.3),
	m_mat_diff(1.0, 0.0, 0.0),
	m_mat_spec(0.2, 0.2, 0.2),
	m_mat_shine(30.0),
	m_mat_alpha(1.0),
	m_shadow(true),
	m_shadow_darkness(0.6),
	m_enable_limit(false),
	m_limit(50)
{
	type = 3;//mesh

	m_trans[0] = 0.0;
	m_trans[1] = 0.0;
	m_trans[2] = 0.0;
	m_rot[0] = 0.0;
	m_rot[1] = 0.0;
	m_rot[2] = 0.0;
	m_scale[0] = 1.0;
	m_scale[1] = 1.0;
	m_scale[2] = 1.0;

	double hue, sat, val;
	hue = double(rand()%360);
	sat = 1.0;
	val = 1.0;
	Color color(HSVColor(hue, sat, val));
	m_mat_diff = color;

	m_legend = true;
}

MeshData::~MeshData()
{
	if (m_mr)
		delete m_mr;
	if (m_data)
		glmDelete(m_data);
}

//set viewport
void MeshData::SetViewport(GLint vp[4])
{
	if (m_mr)
		m_mr->set_viewport(vp);
}

int MeshData::Load(GLMmodel* mesh)
{
	if (!mesh) return 0;

	m_data_path = "";
	m_name = "New Mesh";

	if (m_data)
		delete m_data;
	m_data = mesh;

	if (!m_data->normals)
	{
		if (!m_data->facetnorms)
			glmFacetNormals(m_data);
		glmVertexNormals(m_data, 89.0);
	}

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	m_data->materials[0].name = NULL;
	m_data->materials[0].ambient[0] = m_mat_amb.r();
	m_data->materials[0].ambient[1] = m_mat_amb.g();
	m_data->materials[0].ambient[2] = m_mat_amb.b();
	m_data->materials[0].ambient[3] = m_mat_alpha;
	m_data->materials[0].diffuse[0] = m_mat_diff.r();
	m_data->materials[0].diffuse[1] = m_mat_diff.g();
	m_data->materials[0].diffuse[2] = m_mat_diff.b();
	m_data->materials[0].diffuse[3] = m_mat_alpha;
	m_data->materials[0].specular[0] = m_mat_spec.r();
	m_data->materials[0].specular[1] = m_mat_spec.g();
	m_data->materials[0].specular[2] = m_mat_spec.b();
	m_data->materials[0].specular[3] = m_mat_alpha;
	m_data->materials[0].shininess = m_mat_shine;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = GL_FALSE;
	m_data->materials[0].textureID = 0;

	//bounds
	GLfloat fbounds[6];
	glmBoundingBox(m_data, fbounds);
	BBox bounds;
	Point pmin(fbounds[0], fbounds[2], fbounds[4]);
	Point pmax(fbounds[1], fbounds[3], fbounds[5]);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;
	m_center = Point((m_bounds.min().x()+m_bounds.max().x())*0.5,
		(m_bounds.min().y()+m_bounds.max().y())*0.5,
		(m_bounds.min().z()+m_bounds.max().z())*0.5);

	if (m_mr)
		delete m_mr;
	m_mr = new FLIVR::MeshRenderer(m_data);

	return 1;
}

int MeshData::Load(wxString &filename)
{
	m_data_path = filename;
	m_name = wxFileNameFromPath(filename);

	if (m_data)
		delete m_data;

	string str_fn = filename.ToStdString();
	bool no_fail = true;
	m_data = glmReadOBJ(str_fn.c_str(),&no_fail);
	//while (!no_fail) {
	//	wxMessageDialog *dial = new wxMessageDialog(NULL, 
	//		wxT("A part of the OBJ file failed to load. Would you like to try re-loading?"), 
	//		wxT("OBJ Load Failure"), 
	//		wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);
	//	if (dial->ShowModal() == wxID_YES) {
	//		if (m_data)
	//			delete m_data;
	//		m_data = glmReadOBJ(str_fn.c_str(),&no_fail);
	//	} else break;
	//}

	if (!m_data)
		return 0;

	if (!m_data->normals && m_data->numtriangles)
	{
		if (!m_data->facetnorms)
			glmFacetNormals(m_data);
		glmVertexNormals(m_data, 89.0);
	}

	if (!m_data->materials)
	{
		m_data->materials = new GLMmaterial;
		m_data->nummaterials = 1;
	}

	/* set the default material */
	m_data->materials[0].name = NULL;
	m_data->materials[0].ambient[0] = m_mat_amb.r();
	m_data->materials[0].ambient[1] = m_mat_amb.g();
	m_data->materials[0].ambient[2] = m_mat_amb.b();
	m_data->materials[0].ambient[3] = m_mat_alpha;
	m_data->materials[0].diffuse[0] = m_mat_diff.r();
	m_data->materials[0].diffuse[1] = m_mat_diff.g();
	m_data->materials[0].diffuse[2] = m_mat_diff.b();
	m_data->materials[0].diffuse[3] = m_mat_alpha;
	m_data->materials[0].specular[0] = m_mat_spec.r();
	m_data->materials[0].specular[1] = m_mat_spec.g();
	m_data->materials[0].specular[2] = m_mat_spec.b();
	m_data->materials[0].specular[3] = m_mat_alpha;
	m_data->materials[0].shininess = m_mat_shine;
	m_data->materials[0].emmissive[0] = 0.0;
	m_data->materials[0].emmissive[1] = 0.0;
	m_data->materials[0].emmissive[2] = 0.0;
	m_data->materials[0].emmissive[3] = 0.0;
	m_data->materials[0].havetexture = GL_FALSE;
	m_data->materials[0].textureID = 0;

	//bounds
	GLfloat fbounds[6];
	glmBoundingBox(m_data, fbounds);
	BBox bounds;
	Point pmin(fbounds[0], fbounds[2], fbounds[4]);
	Point pmax(fbounds[1], fbounds[3], fbounds[5]);
	bounds.extend(pmin);
	bounds.extend(pmax);
	m_bounds = bounds;
	m_center = Point((m_bounds.min().x()+m_bounds.max().x())*0.5,
		(m_bounds.min().y()+m_bounds.max().y())*0.5,
		(m_bounds.min().z()+m_bounds.max().z())*0.5);

	if (m_mr)
		delete m_mr;
	m_mr = new FLIVR::MeshRenderer(m_data);

	return 1;
}

void MeshData::Save(wxString& filename)
{
	if (m_data)
	{
		char* str = new char[filename.length()+1];
		for (int i=0; i<(int)filename.length(); i++)
			str[i] = (char)filename[i];
		str[filename.length()] = 0;
		glmWriteOBJ(m_data, str, GLM_SMOOTH);
		delete []str;
		m_data_path = filename;
	}
}

//MR
MeshRenderer* MeshData::GetMR()
{
	return m_mr;
}

void MeshData::SetMatrices(glm::mat4 &mv_mat, glm::mat4 &proj_mat)
{
	if (m_mr)
	{
		glm::mat4 mv_temp;
		mv_temp = glm::translate(
			mv_mat, glm::vec3(
		m_trans[0]+m_center.x(),
		m_trans[1]+m_center.y(),
		m_trans[2]+m_center.z()));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(m_rot[0])),
			glm::vec3(1.0, 0.0, 0.0));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(m_rot[1])),
			glm::vec3(0.0, 1.0, 0.0));
		mv_temp = glm::rotate(
			mv_temp, float(glm::radians(m_rot[2])),
			glm::vec3(0.0, 0.0, 1.0));
		mv_temp = glm::scale(mv_temp,
			glm::vec3(float(m_scale[0]), float(m_scale[1]), float(m_scale[2])));
		mv_temp = glm::translate(mv_temp,
			glm::vec3(-m_center.x(), -m_center.y(), -m_center.z()));

		m_mr->SetMatrices(mv_temp, proj_mat);
	}
}

void MeshData::Draw(int peel)
{
	if (!m_mr)
		return;

	glDisable(GL_CULL_FACE);
	m_mr->set_depth_peel(peel);
	m_mr->draw();
	if (m_draw_bounds && (peel==4 || peel==5))
		DrawBounds();
	glEnable(GL_CULL_FACE);
}

void MeshData::DrawBounds()
{
	if (!m_mr)
		return;

	m_mr->draw_wireframe();
}

void MeshData::DrawInt(unsigned int name)
{
	if (!m_mr)
		return;

	m_mr->draw_integer(name);
}

//lighting
void MeshData::SetLighting(bool bVal)
{
	m_light = bVal;
	if (m_mr) m_mr->set_lighting(m_light);
}

bool MeshData::GetLighting()
{
	return m_light;
}

//fog
void MeshData::SetFog(bool bVal,
	double fog_intensity, double fog_start, double fog_end)
{
	m_fog = bVal;
	if (m_mr) m_mr->set_fog(m_fog, fog_intensity, fog_start, fog_end);
}

bool MeshData::GetFog()
{
	return m_fog;
}

void MeshData::SetMaterial(Color& amb, Color& diff, Color& spec, 
	double shine, double alpha)
{
	m_mat_amb = amb;
	m_mat_diff = diff;
	m_mat_spec = spec;
	m_mat_shine = shine;
	m_mat_alpha = alpha;

	if (m_data && m_data->materials)
	{
		for (int i=0; i<(int)m_data->nummaterials; i++)
		{
			if (i==0)
			{
				m_data->materials[i].ambient[0] = m_mat_amb.r();
				m_data->materials[i].ambient[1] = m_mat_amb.g();
				m_data->materials[i].ambient[2] = m_mat_amb.b();
				m_data->materials[i].diffuse[0] = m_mat_diff.r();
				m_data->materials[i].diffuse[1] = m_mat_diff.g();
				m_data->materials[i].diffuse[2] = m_mat_diff.b();
				m_data->materials[i].specular[0] = m_mat_spec.r();
				m_data->materials[i].specular[1] = m_mat_spec.g();
				m_data->materials[i].specular[2] = m_mat_spec.b();
				m_data->materials[i].shininess = m_mat_shine;
			}
			m_data->materials[i].specular[3] = m_mat_alpha;
			m_data->materials[i].ambient[3] = m_mat_alpha;
			m_data->materials[i].diffuse[3] = m_mat_alpha;
		}
	}
}

void MeshData::SetColor(Color &color, int type)
{
	switch (type)
	{
	case MESH_COLOR_AMB:
		m_mat_amb = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].ambient[0] = m_mat_amb.r();
			m_data->materials[0].ambient[1] = m_mat_amb.g();
			m_data->materials[0].ambient[2] = m_mat_amb.b();
		}
		break;
	case MESH_COLOR_DIFF:
		m_mat_diff = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].diffuse[0] = m_mat_diff.r();
			m_data->materials[0].diffuse[1] = m_mat_diff.g();
			m_data->materials[0].diffuse[2] = m_mat_diff.b();
		}
		break;
	case MESH_COLOR_SPEC:
		m_mat_spec = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].specular[0] = m_mat_spec.r();
			m_data->materials[0].specular[1] = m_mat_spec.g();
			m_data->materials[0].specular[2] = m_mat_spec.b();
		}
		break;
	}
}

void MeshData::SetFloat(double &value, int type)
{
	switch (type)
	{
	case MESH_FLOAT_SHN:
		m_mat_shine = value;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].shininess = m_mat_shine;
		}
		break;
	case MESH_FLOAT_ALPHA:
		m_mat_alpha = value;
		if (m_data && m_data->materials)
		{
			for (int i=0; i<(int)m_data->nummaterials; i++)
			{
				m_data->materials[i].ambient[3] = m_mat_alpha;
				m_data->materials[i].diffuse[3] = m_mat_alpha;
				m_data->materials[i].specular[3] = m_mat_alpha;
			}
		}
		if (m_mr) m_mr->set_alpha(value);
		break;
	}

}

void MeshData::GetMaterial(Color& amb, Color& diff, Color& spec,
	double& shine, double& alpha)
{
	amb = m_mat_amb;
	diff = m_mat_diff;
	spec = m_mat_spec;
	shine = m_mat_shine;
	alpha = m_mat_alpha;
}

bool MeshData::IsTransp()
{
	if (m_mat_alpha>=1.0)
		return false;
	else
		return true;
}

//shadow
void MeshData::SetShadow(bool bVal)
{
	m_shadow= bVal;
}

bool MeshData::GetShadow()
{
	return m_shadow;
}

void MeshData::SetShadowParams(double val)
{
	m_shadow_darkness = val;
}

void MeshData::GetShadowParams(double &val)
{
	val = m_shadow_darkness;
}

wxString MeshData::GetPath()
{
	return m_data_path;
}

BBox MeshData::GetBounds()
{
	BBox bounds;
	Point p[8];
	p[0] = Point(m_bounds.min().x(), m_bounds.min().y(), m_bounds.min().z());
	p[1] = Point(m_bounds.min().x(), m_bounds.min().y(), m_bounds.max().z());
	p[2] = Point(m_bounds.min().x(), m_bounds.max().y(), m_bounds.min().z());
	p[3] = Point(m_bounds.min().x(), m_bounds.max().y(), m_bounds.max().z());
	p[4] = Point(m_bounds.max().x(), m_bounds.min().y(), m_bounds.min().z());
	p[5] = Point(m_bounds.max().x(), m_bounds.min().y(), m_bounds.max().z());
	p[6] = Point(m_bounds.max().x(), m_bounds.max().y(), m_bounds.min().z());
	p[7] = Point(m_bounds.max().x(), m_bounds.max().y(), m_bounds.max().z());
	double s, c;
	Point temp;
	for (int i=0 ; i<8 ; i++)
	{
		p[i] = Point(p[i].x()*m_scale[0], p[i].y()*m_scale[1], p[i].z()*m_scale[2]);
		s = sin(d2r(m_rot[2]));
		c = cos(d2r(m_rot[2]));
		temp = Point(c*p[i].x()-s*p[i].y(), s*p[i].x()+c*p[i].y(), p[i].z());
		p[i] = temp;
		s = sin(d2r(m_rot[1]));
		c = cos(d2r(m_rot[1]));
		temp = Point(c*p[i].x()+s*p[i].z(), p[i].y(), -s*p[i].x()+c*p[i].z());
		p[i] = temp;
		s = sin(d2r(m_rot[0]));
		c = cos(d2r(m_rot[0]));
		temp = Point(p[i].x(), c*p[i].y()-s*p[i].z(), s*p[i].y()+c*p[i].z());
		p[i] = Point(temp.x()+m_trans[0], temp.y()+m_trans[1], temp.z()+m_trans[2]);
		bounds.extend(p[i]);
	}
	return bounds;
}

GLMmodel* MeshData::GetMesh()
{
	return m_data;
}

void MeshData::SetDisp(bool disp)
{
	m_disp = disp;
}

void MeshData::ToggleDisp()
{
	m_disp = !m_disp;
}

bool MeshData::GetDisp()
{
	return m_disp;
}

void MeshData::SetDrawBounds(bool draw)
{
	m_draw_bounds = draw;
}

void MeshData::ToggleDrawBounds()
{
	m_draw_bounds = !m_draw_bounds;
}

bool MeshData::GetDrawBounds()
{
	return m_draw_bounds;
}

void MeshData::SetTranslation(double x, double y, double z)
{
	m_trans[0] = x;
	m_trans[1] = y;
	m_trans[2] = z;
}

void MeshData::GetTranslation(double &x, double &y, double &z)
{
	x = m_trans[0];
	y = m_trans[1];
	z = m_trans[2];
}

void MeshData::SetRotation(double x, double y, double z)
{
	m_rot[0] = x;
	m_rot[1] = y;
	m_rot[2] = z;
}

void MeshData::GetRotation(double &x, double &y, double &z)
{
	x = m_rot[0];
	y = m_rot[1];
	z = m_rot[2];
}

void MeshData::SetScaling(double x, double y, double z)
{
	m_scale[0] = x;
	m_scale[1] = y;
	m_scale[2] = z;
}

void MeshData::GetScaling(double &x, double &y, double &z)
{
	x = m_scale[0];
	y = m_scale[1];
	z = m_scale[2];
}

//randomize color
void MeshData::RandomizeColor()
{
	double hue = (double)rand()/(RAND_MAX) * 360.0;
	Color color(HSVColor(hue, 1.0, 1.0));
	SetColor(color, MESH_COLOR_DIFF);
    Color amb = color * 0.3;
	SetColor(amb, MESH_COLOR_AMB);
}

//shown in legend
void MeshData::SetLegend(bool val)
{
	m_legend = val;
}

bool MeshData::GetLegend()
{
	return m_legend;
}

//size limiter
void MeshData::SetLimit(bool bVal)
{
	m_enable_limit = bVal;
	if (m_enable_limit)
		m_mr->set_limit(m_limit);
	else
		m_mr->set_limit(-1);
//	m_mr->update();
}

bool MeshData::GetLimit()
{
	return m_enable_limit;
}

void MeshData::SetLimitNumer(int val)
{
	m_limit = val;
	if (m_enable_limit)
	{
		m_mr->set_limit(val);
//		m_mr->update();
	}
}

int MeshData::GetLimitNumber()
{
	return m_limit;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
AText::AText()
{
}

AText::AText(const string &str, const Point &pos)
{
	m_txt = str;
	m_pos = pos;
}

AText::~AText()
{
}

string AText::GetText()
{
	return m_txt;
}

Point AText::GetPos()
{
	return m_pos;
}

void AText::SetText(string str)
{
	m_txt = str;
}

void AText::SetPos(Point pos)
{
	m_pos = pos;
}

void AText::SetInfo(string str)
{
	m_info = str;
}

int Annotations::m_num = 0;

Annotations::Annotations()
{
	type = 4;//annotations
	m_num++;
	m_name = wxString::Format("Antn_%d", m_num);
	m_tform = 0;
	m_vd = 0;
	m_disp = true;
	m_memo_ro = false;
}

Annotations::~Annotations()
{
	Clear();
}

int Annotations::GetTextNum()
{
	return (int)m_alist.size();
}

string Annotations::GetTextText(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		AText* atext = m_alist[index];
		if (atext)
			return atext->m_txt;
	}
	return "";
}

Point Annotations::GetTextPos(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		AText* atext = m_alist[index];
		if (atext)
			return atext->m_pos;
	}
	return Point(Vector(0.0));
}

Point Annotations::GetTextTransformedPos(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		AText* atext = m_alist[index];
		if (atext && m_tform)
			return m_tform->transform(atext->m_pos);
	}
	return Point(Vector(0.0));
}

string Annotations::GetTextInfo(int index)
{
	if (index>=0 && index<(int)m_alist.size())
	{
		AText* atext = m_alist[index];
		if(atext)
			return atext->m_info;
	}
	return "";
}

void Annotations::AddText(std::string str, Point pos, std::string info)
{
	AText* atext = new AText(str, pos);
	atext->SetInfo(info);
	m_alist.push_back(atext);
}

void Annotations::SetTransform(Transform *tform)
{
	m_tform = tform;
}

void Annotations::SetVolume(VolumeData *vd)
{
	m_vd = vd;
	if (m_vd)
		m_name += "_FROM_" + m_vd->GetName();
}

VolumeData* Annotations::GetVolume()
{
	return m_vd;
}

void Annotations::Clear()
{
	for (int i=0; i<(int)m_alist.size(); i++)
	{
		AText* atext = m_alist[i];
		if (atext)
			delete atext;
	}
	m_alist.clear();
}

//memo
void Annotations::SetMemo(string &memo)
{
	m_memo = memo;
}

string& Annotations::GetMemo()
{
	return m_memo;
}

void Annotations::SetMemoRO(bool ro)
{
	m_memo_ro = ro;
}

bool Annotations::GetMemoRO()
{
	return m_memo_ro;
}

//save/load
wxString Annotations::GetPath()
{
	return m_data_path;
}

int Annotations::Load(wxString &filename, DataManager* mgr)
{
	wxFileInputStream fis(filename);
	if (!fis.Ok())
		return 0;

	wxTextInputStream tis(fis);
	wxString str;

	while (!fis.Eof())
	{
		wxString sline = tis.ReadLine();

		if (sline.SubString(0, 5) == "Name: ")
		{
			m_name = sline.SubString(6, sline.Length()-1);
		}
		else if (sline.SubString(0, 8) == "Display: ")
		{
			str = sline.SubString(9, 9);
			if (str == "0")
				m_disp = false;
			else
				m_disp = true;
		}
		else if (sline.SubString(0, 4) == "Memo:")
		{
			str = tis.ReadLine();
			while (str.SubString(0, 12) != "Memo Update: " &&
				!fis.Eof())
			{
				m_memo += str + "\n";
				str = tis.ReadLine();
			}
			if (str.SubString(13, 13) == "0")
				m_memo_ro = false;
			else
				m_memo_ro = true;
		}
		else if (sline.SubString(0, 7) == "Volume: ")
		{
			str = sline.SubString(8, sline.Length()-1);
			VolumeData* vd = mgr->GetVolumeData(str);
			if (vd)
			{
				m_vd = vd;
				m_tform = vd->GetTexture()->transform();
			}
		}
		else if (sline.SubString(0, 9) == "Transform:")
		{
			str = tis.ReadLine();
			str = tis.ReadLine();
			str = tis.ReadLine();
			str = tis.ReadLine();
		}
		else if (sline.SubString(0, 10) == "Components:")
		{
			str = tis.ReadLine();
			int tab_counter = 0;
			for (int i=0; i<(int)str.Length(); i++)
			{
				if (str[i] == '\t')
					tab_counter++;
				if (tab_counter == 4)
				{
					m_info_meaning = str.SubString(i+1, str.Length()-1);
					break;
				}
			}

			str = tis.ReadLine();
			while (!fis.Eof())
			{
				if (AText* atext = GetAText(str))
					m_alist.push_back(atext);
				str = tis.ReadLine();
			}
		}
	}

	m_data_path = filename;
	return 1;
}

void Annotations::Save(wxString &filename)
{
	wxFileOutputStream fos(filename);
	if (!fos.Ok())
		return;

	wxTextOutputStream tos(fos);

	int resx = 1;
	int resy = 1;
	int resz = 1;
	if (m_vd)
		m_vd->GetResolution(resx, resy, resz);

	tos << "Name: " << m_name << "\n";
	tos << "Display: " << m_disp << "\n";
	tos << "Memo:\n" << m_memo << "\n";
	tos << "Memo Update: " << m_memo_ro << "\n";
	if (m_vd)
	{
		tos << "Volume: " << m_vd->GetName() << "\n";
		tos << "Voxel size (X Y Z):\n";
		double spcx, spcy, spcz;
		m_vd->GetSpacings(spcx, spcy, spcz);
		tos << spcx << "\t" << spcy << "\t" << spcz << "\n";
	}


	tos << "\nComponents:\n";
	tos << "ID\tX\tY\tZ\t" << m_info_meaning << "\n\n";
	for (int i=0; i<(int)m_alist.size(); i++)
	{
		AText* atext = m_alist[i];
		if (atext)
		{
			tos << atext->m_txt << "\t";
			tos << int(atext->m_pos.x()*resx+1.0) << "\t";
			tos << int(atext->m_pos.y()*resy+1.0) << "\t";
			tos << int(atext->m_pos.z()*resz+1.0) << "\t";
			tos << atext->m_info << "\n";
		}
	}

	m_data_path = filename;
}

wxString Annotations::GetInfoMeaning()
{
	return m_info_meaning;
}

void Annotations::SetInfoMeaning(wxString &str)
{
	m_info_meaning = str;
}

//test if point is inside the clipping planes
bool Annotations::InsideClippingPlanes(Point &pos)
{
	if (!m_vd)
		return true;

	vector<Plane*> *planes = m_vd->GetVR()->get_planes();
	if (!planes)
		return true;
	if (planes->size() != 6)
		return true;

	Plane* plane = 0;
	for (int i=0; i<6; i++)
	{
		plane = (*planes)[i];
		if (!plane)
			continue;
		if (plane->eval_point(pos) < 0)
			return false;
	}

	return true;
}

AText* Annotations::GetAText(wxString str)
{
	AText *atext = 0;
	wxString sID;
	wxString sX;
	wxString sY;
	wxString sZ;
	wxString sInfo;
	int tab_counter = 0;

	for (int i=0; i<(int)str.Length(); i++)
	{
		if (str[i] == '\t')
			tab_counter++;
		else
		{
			if (tab_counter == 0)
				sID += str[i];
			else if (tab_counter == 1)
				sX += str[i];
			else if (tab_counter == 2)
				sY += str[i];
			else if (tab_counter == 3)
				sZ += str[i];
			else if (tab_counter == 4)
			{
				sInfo = str.SubString(i, str.Length()-1);
				break;
			}
		}
	}
	if (tab_counter == 4)
	{
		double x, y, z;
		sX.ToDouble(&x);
		sY.ToDouble(&y);
		sZ.ToDouble(&z);
		int resx = 1;
		int resy = 1;
		int resz = 1;
		if (m_vd)
			m_vd->GetResolution(resx, resy, resz);
		x /= resx?resx:1;
		y /= resy?resy:1;
		z /= resz?resz:1;
		Point pos(x, y, z);
		atext = new AText(sID.ToStdString(), pos);
		atext->SetInfo(sInfo.ToStdString());
	}

	return atext;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Ruler::m_num = 0;
Ruler::Ruler()
{
	type = 7;//ruler
	m_id = m_num;
	m_num++;
	m_name = wxString::Format("Ruler %d", m_num);
	m_disp = true;
	m_tform = 0;
	m_ruler_type = 0;
	m_finished = false;
	m_use_color = false;

	//time-dependent
	m_time_dep = false;
	m_time = 0;

	//brush size
	m_brush_size = 0.0;
}

Ruler::~Ruler()
{
}

//data
int Ruler::GetNumPoint()
{
	return (int)m_ruler.size();
}

Point *Ruler::GetPoint(int index)
{
	if (index>=0 && (size_t)index<m_ruler.size())
		return &(m_ruler[index]);
	else
		return 0;
}

int Ruler::GetRulerType()
{
	return m_ruler_type;
}

void Ruler::SetRulerType(int type)
{
	m_ruler_type = type;
}

bool Ruler::GetFinished()
{
	return m_finished;
}

void Ruler::SetFinished()
{
	m_finished = true;
}

double Ruler::GetLength()
{
	double length = 0.0;
	Point p1, p2;

	for (unsigned int i=1; i<m_ruler.size(); ++i)
	{
		p1 = m_ruler[i-1];
		p2 = m_ruler[i];
		length += (p2-p1).length();
	}

	return length;
}

double Ruler::GetLengthObject(double spcx, double spcy, double spcz)
{
	double length = 0.0;
	Point p1, p2;

	for (unsigned int i=1; i<m_ruler.size(); ++i)
	{
		p1 = m_ruler[i-1];
		p2 = m_ruler[i];
		p1 = Point(p1.x()/spcx, p1.y()/spcy, p1.z()/spcz);
		p2 = Point(p2.x()/spcx, p2.y()/spcy, p2.z()/spcz);
		length += (p2-p1).length();
	}

	return length;
}

double Ruler::GetAngle()
{
	double angle = 0.0;

	if (m_ruler_type == 0 ||
		m_ruler_type == 3)
	{
		if (m_ruler.size() >= 2)
		{
			Vector v = m_ruler[1] - m_ruler[0];
			v.normalize();
			angle = atan2(-v.y(), (v.x()>0.0?1.0:-1.0)*sqrt(v.x()*v.x() + v.z()*v.z()));
			angle = r2d(angle);
			angle = angle<0.0?angle+180.0:angle;
		}
	}
	else if (m_ruler_type == 4)
	{
		if (m_ruler.size() >=3)
		{
			Vector v1, v2;
			v1 = m_ruler[0] - m_ruler[1];
			v1.normalize();
			v2 = m_ruler[2] - m_ruler[1];
			v2.normalize();
			angle = acos(Dot(v1, v2));
			angle = r2d(angle);
		}
	}

	return angle;
}

void Ruler::Scale(double spcx, double spcy, double spcz)
{
	for (size_t i = 0; i < m_ruler.size(); ++i)
		m_ruler[i].scale(spcx, spcy, spcz);
}

bool Ruler::AddPoint(Point &point)
{
	if (m_ruler_type == 2 &&
		m_ruler.size() == 1)
		return false;
	else if ((m_ruler_type == 0 ||
		m_ruler_type == 3) &&
		m_ruler.size() == 2)
		return false;
	else if (m_ruler_type == 4 &&
		m_ruler.size() == 3)
		return false;
	else
	{
		m_ruler.push_back(point);
		if (m_ruler_type == 2 &&
			m_ruler.size() == 1)
			m_finished = true;
		else if ((m_ruler_type == 0 ||
			m_ruler_type == 3) &&
			m_ruler.size() == 2)
			m_finished = true;
		else if (m_ruler_type == 4 &&
			m_ruler.size() == 3)
			m_finished = true;
		return true;
	}
}

void Ruler::SetTransform(Transform *tform)
{
	m_tform = tform;
}

void Ruler::Clear()
{
	m_ruler.clear();
}

wxString Ruler::GetDelInfoValues(wxString del)
{
	wxString output;

	for (size_t i=0; i<m_info_values.length(); i++)
	{
		if (m_info_values[i] == '\t')
			output += del;
		else
			output += m_info_values[i];
	}

	return output;
}

wxString Ruler::GetPosValues()
{
	wxString output;

	//x string
	output += "x\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		output += std::to_string(m_ruler[i].x());
		if (i == m_ruler.size() - 1)
			output += "\n";
		else
			output += "\t";
	}
	//y string
	output += "y\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		output += std::to_string(m_ruler[i].y());
		if (i == m_ruler.size() - 1)
			output += "\n";
		else
			output += "\t";
	}
	//z string
	output += "z\t";
	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		output += std::to_string(m_ruler[i].z());
		if (i == m_ruler.size() - 1)
			output += "\n";
		else
			output += "\t";
	}

	return output;
}

wxString Ruler::GetPosNames()
{
	wxString output;

	output += "Coords\t";

	for (size_t i = 0; i < m_ruler.size(); ++i)
	{
		output += "Point" + std::to_string(i+1);
		if (i == m_ruler.size() - 1)
			output += "\n";
		else
			output += "\t";
	}

	return output;
}

void Ruler::SaveProfile(wxString &filename)
{
}

void Ruler::FinishEllipse(Vector view)
{
	if (m_ruler_type != 5 || m_ruler.size() != 2)
		return;

	Point p0 = m_ruler[0];
	Point p1 = m_ruler[1];
	Vector p01 = p0 - p1;
	Vector axis = Cross(p01, view);
	axis.normalize();
	axis = Cross(p01, axis);
	axis.normalize();
	Point p2, p3, pc;
	pc = Point((p0 + p1) / 2.0);
	Vector halfd = p0 - pc;
	Quaternion q0(halfd);
	Quaternion q(90.0, axis);
	q.Normalize();
	Quaternion q2 = (-q) * q0 * q;
	p2 = Point(q2.x, q2.y, q2.z);
	p3 = -p2;
	p2 = Point(pc + p2);
	p3 = Point(pc + p3);
	AddPoint(p2);
	AddPoint(p3);

	m_finished = true;
}

Point Ruler::GetCenter()
{
	Point result;
	if (m_ruler.empty())
		return result;
	for (auto it = m_ruler.begin();
		it != m_ruler.end(); ++it)
	{
		result += *it;
	}
	result /= m_ruler.size();
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TraceGroup::m_num = 0;
TraceGroup::TraceGroup()
{
	type = 8;//traces
	m_num++;
	m_name = wxString::Format("Traces %d", m_num);
	m_cur_time = -1;
	m_prv_time = -1;
	m_ghost_num = 10;
	m_draw_tail = true;
	m_draw_lead = false;
	m_cell_size = 20;
	m_uncertain_low = 0;
	m_uncertain_high = 20;
	m_track_map = FL::pTrackMap(new FL::TrackMap());
}

TraceGroup::~TraceGroup()
{
}

void TraceGroup::SetCurTime(int time)
{
	m_cur_time = time;
	TextureRenderer::vertex_array_manager_.set_dirty(VA_Traces);
}

int TraceGroup::GetCurTime()
{
	return m_cur_time;
}
void TraceGroup::SetPrvTime(int time)
{
	m_prv_time = time;
}

int TraceGroup::GetPrvTime()
{
	return m_prv_time;
}

//get information
void TraceGroup::GetLinkLists(size_t frame,
	FL::VertexList &in_orphan_list,
	FL::VertexList &out_orphan_list,
	FL::VertexList &in_multi_list,
	FL::VertexList &out_multi_list)
{
	if (in_orphan_list.size())
		in_orphan_list.clear();
	if (out_orphan_list.size())
		out_orphan_list.clear();
	if (in_multi_list.size())
		in_multi_list.clear();
	if (out_multi_list.size())
		out_multi_list.clear();

	FL::TrackMapProcessor tm_processor(m_track_map);
	tm_processor.SetSizeThresh(m_cell_size);
	tm_processor.SetUncertainLow(m_uncertain_low);
	tm_processor.SetUncertainHigh(m_uncertain_high);
	tm_processor.GetLinkLists(frame,
		in_orphan_list, out_orphan_list,
		in_multi_list, out_multi_list);
}

void TraceGroup::ClearCellList()
{
	m_cell_list.clear();
	TextureRenderer::vertex_array_manager_.set_dirty(VA_Traces);
}

//cur_sel_list: ids from previous time point
//m_prv_time: previous time value
//m_id_map: ids of current time point that are linked to previous
//m_cur_time: current time value
//time values are check with frame ids in the frame list
void TraceGroup::UpdateCellList(FL::CellList &cur_sel_list)
{
	ClearCellList();
	FL::CellListIter cell_iter;

	//why does not the time change?
	//because I just want to find out the current selection
	if (m_prv_time == m_cur_time)
	{
		//copy cur_sel_list to m_cell_list
		for (cell_iter = cur_sel_list.begin();
			cell_iter != cur_sel_list.end();
			++cell_iter)
		{
			if (cell_iter->second->GetSizeUi() >
				(unsigned int)m_cell_size)
				m_cell_list.insert(pair<unsigned int, FL::pCell>
					(cell_iter->second->Id(), cell_iter->second));
		}
		return;
	}

	//get mapped cells
	//cur_sel_list -> m_cell_list
	FL::TrackMapProcessor tm_processor(m_track_map);
	tm_processor.GetMappedCells(
		cur_sel_list, m_cell_list,
		(unsigned int)m_prv_time,
		(unsigned int)m_cur_time);

	TextureRenderer::vertex_array_manager_.set_dirty(VA_Traces);
}

FL::CellList &TraceGroup::GetCellList()
{
	return m_cell_list;
}

bool TraceGroup::FindCell(unsigned int id)
{
	return m_cell_list.find(id) != m_cell_list.end();
}

//modifications
bool TraceGroup::AddCell(FL::pCell &cell, size_t frame)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.AddCellDup(cell, frame);
}

bool TraceGroup::LinkCells(FL::CellList &list1, FL::CellList &list2,
	size_t frame1, size_t frame2, bool exclusive)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.LinkCells(list1, list2,
		frame1, frame2, exclusive);
}

bool TraceGroup::IsolateCells(FL::CellList &list, size_t frame)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.IsolateCells(list, frame);
}

bool TraceGroup::UnlinkCells(FL::CellList &list1, FL::CellList &list2,
	size_t frame1, size_t frame2)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.UnlinkCells(list1, list2, frame1, frame2);
}

bool TraceGroup::CombineCells(FL::pCell &cell, FL::CellList &list,
	size_t frame)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.CombineCells(cell, list, frame);
}

bool TraceGroup::DivideCells(FL::CellList &list, size_t frame)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.DivideCells(list, frame);
}

bool TraceGroup::ReplaceCellID(unsigned int old_id, unsigned int new_id, size_t frame)
{
	FL::TrackMapProcessor tm_processor(m_track_map);
	return tm_processor.ReplaceCellID(old_id, new_id, frame);
}

bool TraceGroup::GetMappedRulers(RulerList &rulers)
{
	size_t frame_num = m_track_map->GetFrameNum();
	if (m_ghost_num <= 0 ||
		m_cur_time < 0 ||
		m_cur_time >= frame_num)
		return false;

	//estimate verts size
	size_t remain_num = frame_num - m_cur_time - 1;
	size_t ghost_lead, ghost_tail;
	ghost_lead = m_draw_lead ?
		(remain_num>m_ghost_num ?
			m_ghost_num : remain_num) : 0;
	ghost_tail = m_draw_tail ?
		(m_cur_time >= m_ghost_num ?
			m_ghost_num : m_cur_time) : 0;

	FL::CellList temp_sel_list1, temp_sel_list2;

	if (m_draw_lead)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
		i < m_cur_time + ghost_lead; ++i)
		{
			GetMappedRulers(
				temp_sel_list1, temp_sel_list2,
				rulers, i, i + 1);
			//swap
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	//clear ruler id
	for (RulerListIter iter = rulers.begin();
	iter != rulers.end(); ++iter)
		(*iter)->Id(0);

	if (m_draw_tail)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
		i > m_cur_time - ghost_tail; --i)
		{
			GetMappedRulers(
				temp_sel_list1, temp_sel_list2,
				rulers, i, i - 1);
			//sawp
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	return true;
}

unsigned int TraceGroup::GetMappedEdges(
	FL::CellList & sel_list1, FL::CellList & sel_list2,
	std::vector<float>& verts,
	size_t frame1, size_t frame2)
{
	unsigned int result = 0;

	size_t frame_num = m_track_map->GetFrameNum();
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return result;

	FL::CellList &cell_list1 = m_track_map->GetCellList(frame1);
	FL::InterGraph &inter_graph = m_track_map->GetInterGraph(
		frame1 > frame2 ? frame2 : frame1);
	FL::CellListIter sel_iter, cell_iter;
	FL::pVertex vertex1, vertex2;
	FL::pCell cell;
	FL::InterVert v1, v2;
	std::pair<FL::InterAdjIter, FL::InterAdjIter> adj_verts;
	FL::InterAdjIter inter_iter;
	FL::CellBinIter pwcell_iter;
	FLIVR::Color c;
	std::pair<FL::InterEdge, bool> inter_edge;

	for (sel_iter = sel_list1.begin();
		sel_iter != sel_list1.end();
		++sel_iter)
	{
		cell_iter = cell_list1.find(sel_iter->second->Id());
		if (cell_iter == cell_list1.end())
			continue;
		vertex1 = cell_iter->second->GetVertex().lock();
		if (!vertex1)
			continue;
		v1 = vertex1->GetInterVert(inter_graph);
		if (v1 == FL::InterGraph::null_vertex())
			continue;
		adj_verts = boost::adjacent_vertices(v1, inter_graph);
		//for each adjacent vertex
		for (inter_iter = adj_verts.first;
			inter_iter != adj_verts.second;
			++inter_iter)
		{
			v2 = *inter_iter;
			//get edge
			inter_edge = boost::edge(v1, v2, inter_graph);
			if (!inter_edge.second)
				continue;
			else if (!inter_graph[inter_edge.first].link)
				continue;
			vertex2 = inter_graph[v2].vertex.lock();
			if (!vertex2)
				continue;
			//store all cells in sel_list2
			for (pwcell_iter = vertex2->GetCellsBegin();
				pwcell_iter != vertex2->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				sel_list2.insert(std::pair<unsigned int, FL::pCell>
					(cell->Id(), cell));
				//save to verts
				c = FLIVR::HSVColor(cell->Id() % 360, 1.0, 0.9);
				verts.push_back(vertex1->GetCenter().x());
				verts.push_back(vertex1->GetCenter().y());
				verts.push_back(vertex1->GetCenter().z());
				verts.push_back(c.r());
				verts.push_back(c.g());
				verts.push_back(c.b());
				verts.push_back(vertex2->GetCenter().x());
				verts.push_back(vertex2->GetCenter().y());
				verts.push_back(vertex2->GetCenter().z());
				verts.push_back(c.r());
				verts.push_back(c.g());
				verts.push_back(c.b());
				result += 2;
			}
		}
	}

	return result;
}

bool TraceGroup::GetMappedRulers(
	FL::CellList& sel_list1, FL::CellList &sel_list2,
	RulerList& rulers,
	size_t frame1, size_t frame2)
{
	size_t frame_num = m_track_map->GetFrameNum();
	if (frame1 >= frame_num ||
		frame2 >= frame_num ||
		frame1 == frame2)
		return false;

	FL::CellList &cell_list1 = m_track_map->GetCellList(frame1);
	FL::InterGraph &inter_graph = m_track_map->GetInterGraph(
		frame1 > frame2 ? frame2 : frame1);
	FL::CellListIter sel_iter, cell_iter;
	FL::pVertex vertex1, vertex2;
	FL::pCell cell;
	FL::InterVert v1, v2;
	std::pair<FL::InterAdjIter, FL::InterAdjIter> adj_verts;
	FL::InterAdjIter inter_iter;
	FL::CellBinIter pwcell_iter;
	FLIVR::Color c;
	std::pair<FL::InterEdge, bool> inter_edge;
	RulerListIter ruler_iter;

	for (sel_iter = sel_list1.begin();
		sel_iter != sel_list1.end();
		++sel_iter)
	{
		cell_iter = cell_list1.find(sel_iter->second->Id());
		if (cell_iter == cell_list1.end())
			continue;
		vertex1 = cell_iter->second->GetVertex().lock();
		if (!vertex1)
			continue;
		v1 = vertex1->GetInterVert(inter_graph);
		if (v1 == FL::InterGraph::null_vertex())
			continue;
		adj_verts = boost::adjacent_vertices(v1, inter_graph);
		//for each adjacent vertex
		for (inter_iter = adj_verts.first;
			inter_iter != adj_verts.second;
			++inter_iter)
		{
			v2 = *inter_iter;
			//get edge
			inter_edge = boost::edge(v1, v2, inter_graph);
			if (!inter_edge.second)
				continue;
			else if (!inter_graph[inter_edge.first].link)
				continue;
			vertex2 = inter_graph[v2].vertex.lock();
			if (!vertex2)
				continue;
			//store all cells in sel_list2
			for (pwcell_iter = vertex2->GetCellsBegin();
				pwcell_iter != vertex2->GetCellsEnd();
				++pwcell_iter)
			{
				cell = pwcell_iter->lock();
				if (!cell)
					continue;
				sel_list2.insert(std::pair<unsigned int, FL::pCell>
					(cell->Id(), cell));
				//save to rulers
				ruler_iter = FindRulerFromList(vertex1->Id(), rulers);
				if (ruler_iter == rulers.end())
				{
					Ruler* ruler = new Ruler();
					ruler->SetRulerType(1);//multi-point
					ruler->AddPoint(vertex1->GetCenter());
					ruler->AddPoint(vertex2->GetCenter());
					ruler->SetTimeDep(false);
					ruler->Id(vertex2->Id());
					rulers.push_back(ruler);
				}
				else
				{
					Ruler* ruler = *ruler_iter;
					ruler->AddPoint(vertex2->GetCenter());
					ruler->Id(vertex2->Id());
				}
			}
		}
	}

	return true;
}

RulerListIter TraceGroup::FindRulerFromList(unsigned int id, RulerList &list)
{
	RulerListIter iter = list.begin();
	while (iter != list.end())
	{
		if ((*iter)->Id() == id)
			return iter;
		++iter;
	}
	return iter;
}

bool TraceGroup::Load(wxString &filename)
{
	m_data_path = filename;
	FL::TrackMapProcessor tm_processor(m_track_map);
	std::string str = ws2s(m_data_path.ToStdWstring());
	return tm_processor.Import(str);
}

bool TraceGroup::Save(wxString &filename)
{
	m_data_path = filename;
	FL::TrackMapProcessor tm_processor(m_track_map);
	std::string str = ws2s(m_data_path.ToStdWstring());
	return tm_processor.Export(str);
}

unsigned int TraceGroup::Draw(vector<float> &verts)
{
	unsigned int result = 0;
	size_t frame_num = m_track_map->GetFrameNum();
	if (m_ghost_num <= 0 ||
		m_cur_time < 0 ||
		m_cur_time >= frame_num ||
		m_cell_list.empty())
		return result;

	//estimate verts size
	size_t remain_num = frame_num - m_cur_time - 1;
	size_t ghost_lead, ghost_tail;
	ghost_lead = m_draw_lead ?
		(remain_num>m_ghost_num ?
		m_ghost_num : remain_num) : 0;
	ghost_tail = m_draw_tail ?
		(m_cur_time>=m_ghost_num ?
		m_ghost_num : m_cur_time) : 0;
	verts.reserve((ghost_lead + ghost_tail) *
		m_cell_list.size() * 3 * 6 * 3);//1.5 branches each

	FL::CellList temp_sel_list1, temp_sel_list2;

	if (m_draw_lead)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
			i < m_cur_time + ghost_lead; ++i)
		{
			result += GetMappedEdges(
				temp_sel_list1, temp_sel_list2,
				verts, i, i + 1);
			//swap
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	if (m_draw_tail)
	{
		temp_sel_list1 = m_cell_list;
		for (size_t i = m_cur_time;
			i > m_cur_time - ghost_tail; --i)
		{
			result += GetMappedEdges(
				temp_sel_list1, temp_sel_list2,
				verts, i, i - 1);
			//sawp
			temp_sel_list1 = temp_sel_list2;
			temp_sel_list2.clear();
		}
	}

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int DataGroup::m_num = 0;
DataGroup::DataGroup()
{
	type = 5;//group
	m_num++;
	m_name = wxString::Format("Group %d", m_num);
	m_disp = true;
	m_sync_volume_prop = false;
}

DataGroup::~DataGroup()
{
}

int DataGroup::GetBlendMode()
{
	if (!m_vd_list.empty())
		return m_vd_list[0]->GetBlendMode();
	else
		return 0;
}

//set gamma to all
void DataGroup::SetGammaAll(Color &gamma)
{
	SetGamma(gamma);
	for (int i=0; i<(int)m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd)
			vd->SetGamma(gamma);
	}
}

//set brightness to all
void DataGroup::SetBrightnessAll(Color &brightness)
{
	SetBrightness(brightness);
	for (int i=0; i<(int)m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd)
			vd->SetBrightness(brightness);
	}
}

//set Hdr to all
void DataGroup::SetHdrAll(Color &hdr)
{
	SetHdr(hdr);
	for (int i=0; i<(int)m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd)
			vd->SetHdr(hdr);
	}
}

//set sync red to all
void DataGroup::SetSyncRAll(bool sync_r)
{
	SetSyncR(sync_r);
	for (int i=0; i<(int)m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd)
			vd->SetSyncR(sync_r);
	}
}

//set sync green to all
void DataGroup::SetSyncGAll(bool sync_g)
{
	SetSyncG(sync_g);
	for (int i=0; i<(int)m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd)
			vd->SetSyncG(sync_g);
	}
}

//set sync blue to all
void DataGroup::SetSyncBAll(bool sync_b)
{
	SetSyncB(sync_b);
	for (int i=0; i<(int)m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd)
			vd->SetSyncB(sync_b);
	}
}

void DataGroup::ResetSync()
{
	int i;
	int cnt = 0;
	bool r_v = false;
	bool g_v = false;
	bool b_v = false;

	for (i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
		{
			Color c = vd->GetColor();
			bool r, g, b;
			r = g = b = false;
			cnt = 0;
			if (c.r()>0) {cnt++; r=true;}
			if (c.g()>0) {cnt++; g=true;}
			if (c.b()>0) {cnt++; b=true;}

			if (cnt > 1)
			{
				r_v = r_v||r;
				g_v = g_v||g;
				b_v = b_v||b;
			}
		}
	}

	SetSyncRAll(r_v);
	SetSyncGAll(g_v);
	SetSyncBAll(b_v);
}

//volume properties
void DataGroup::SetEnableAlpha(bool mode)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetEnableAlpha(mode);
	}
}

void DataGroup::SetAlpha(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetAlpha(dVal);
	}
}

void DataGroup::SetSampleRate(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetSampleRate(dVal);
	}
}

void DataGroup::SetBoundary(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetBoundary(dVal);
	}
}

void DataGroup::Set3DGamma(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->Set3DGamma(dVal);
	}
}

void DataGroup::SetOffset(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetOffset(dVal);
	}
}

void DataGroup::SetLeftThresh(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetLeftThresh(dVal);
	}
}

void DataGroup::SetRightThresh(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetRightThresh(dVal);
	}
}

void DataGroup::SetLowShading(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetLowShading(dVal);
	}
}

void DataGroup::SetHiShading(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetHiShading(dVal);
	}
}

void DataGroup::SetLuminance(double dVal)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetLuminance(dVal);
	}
}

void DataGroup::SetColormapMode(int mode)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetColormapMode(mode);
	}
}

void DataGroup::SetColormapDisp(bool disp)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetColormapDisp(disp);
	}
}

void DataGroup::SetColormapValues(double low, double high)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
		{
			double l, h;
			vd->GetColormapValues(l, h);
			vd->SetColormapValues(low<0?l:low, high<0?h:high);
		}
	}
}

void DataGroup::SetColormap(int value)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetColormap(value);
	}
}

void DataGroup::SetColormapProj(int value)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetColormapProj(value);
	}
}

void DataGroup::SetShading(bool shading)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetShading(shading);
	}
}

void DataGroup::SetShadow(bool shadow)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetShadow(shadow);
	}
}

void DataGroup::SetShadowParams(double val)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetShadowParams(val);
	}
}

void DataGroup::SetMode(int mode)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetMode(mode);
	}
}

void DataGroup::SetNR(bool val)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetNR(val);
	}
}
//inversion
void DataGroup::SetInterpolate(bool mode)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetInterpolate(mode);
	}
}

//inversion
void DataGroup::SetInvert(bool mode)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetInvert(mode);
	}
}

//blend mode
void DataGroup::SetBlendMode(int mode)
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
			vd->SetBlendMode(mode);
	}
}

//randomize color
void DataGroup::RandomizeColor()
{
	for (int i=0; i<GetVolumeNum(); i++)
	{
		VolumeData* vd = GetVolumeData(i);
		if (vd)
		{
			double hue = (double)rand()/(RAND_MAX) * 360.0;
			Color color(HSVColor(hue, 1.0, 1.0));
			vd->SetColor(color);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int MeshGroup::m_num = 0;
MeshGroup::MeshGroup()
{
	type = 6;//mesh group
	m_num++;
	m_name = wxString::Format("MGroup %d", m_num);
	m_disp = true;
	m_sync_mesh_prop = false;
}

MeshGroup::~MeshGroup()
{
}

//randomize color
void MeshGroup::RandomizeColor()
{
	for (int i=0; i<GetMeshNum(); i++)
	{
		MeshData* md = GetMeshData(i);
		if (md)
		{
			double hue = (double)rand()/(RAND_MAX) * 360.0;
			Color color(HSVColor(hue, 1.0, 1.0));
			md->SetColor(color, MESH_COLOR_DIFF);
            Color amb = color * 0.3;
			md->SetColor(amb, MESH_COLOR_AMB);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DataManager::DataManager() :
m_vol_exb(0.0),
	m_vol_gam(1.0),
	m_vol_of1(1.0),
	m_vol_of2(1.0),
	m_vol_lth(0.0),
	m_vol_hth(1.0),
	m_vol_lsh(0.5),
	m_vol_hsh(10.0),
	m_vol_alf(0.5),
	m_vol_spr(1.5),
	m_vol_xsp(1.0),
	m_vol_ysp(1.0),
	m_vol_zsp(2.5),
	m_vol_lum(1.0),
	m_vol_cmm(0),
	m_vol_cmp(0),
	m_vol_cmj(0),
	m_vol_lcm(0.0),
	m_vol_hcm(1.0),
	m_vol_eap(true),
	m_vol_esh(true),
	m_vol_interp(true),
	m_vol_inv(false),
	m_vol_mip(false),
	m_vol_nrd(false),
	m_vol_shw(false),
	m_vol_swi(0.0),
	m_vol_test_wiref(false),
	m_use_defaults(true),
	m_override_vox(true)
{
	wxString expath = wxStandardPaths::Get().GetExecutablePath();
	expath = wxPathOnly(expath);
	wxString dft = expath + "/default_volume_settings.dft";
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	double val;
	int ival;
	if (fconfig.Read("extract_boundary", &val))
		m_vol_exb = val;
	if (fconfig.Read("gamma", &val))
		m_vol_gam = val;
	if (fconfig.Read("low_offset", &val))
		m_vol_of1 = val;
	if (fconfig.Read("high_offset", &val))
		m_vol_of2 = val;
	if (fconfig.Read("low_thresholding", &val))
		m_vol_lth = val;
	if (fconfig.Read("high_thresholding", &val))
		m_vol_hth = val;
	if (fconfig.Read("low_shading", &val))
		m_vol_lsh = val;
	if (fconfig.Read("high_shading", &val))
		m_vol_hsh = val;
	if (fconfig.Read("alpha", &val))
		m_vol_alf = val;
	if (fconfig.Read("sample_rate", &val))
		m_vol_spr = val;
	if (fconfig.Read("x_spacing", &val))
		m_vol_xsp = val;
	if (fconfig.Read("y_spacing", &val))
		m_vol_ysp = val;
	if (fconfig.Read("z_spacing", &val))
		m_vol_zsp = val;
	if (fconfig.Read("luminance", &val))
		m_vol_lum = val;
	if (fconfig.Read("colormap_mode", &ival))
		m_vol_cmm = ival;
	if (fconfig.Read("colormap", &ival))
		m_vol_cmp = ival;
	if (fconfig.Read("colormap_proj", &ival))
		m_vol_cmj = ival;

	if (fconfig.Read("colormap_low", &val))
		m_vol_lcm = val;
	if (fconfig.Read("colormap_hi", &val))
		m_vol_hcm = val;

	bool bval;
	if (fconfig.Read("enable_alpha", &bval))
		m_vol_eap = bval;
	if (fconfig.Read("enable_shading", &bval))
		m_vol_esh = bval;
	if (fconfig.Read("enable_interp", &bval))
		m_vol_interp = bval;
	if (fconfig.Read("enable_inv", &bval))
		m_vol_inv = bval;
	if (fconfig.Read("enable_mip", &bval))
		m_vol_mip = bval;
	if (fconfig.Read("noise_rd", &bval))
		m_vol_nrd = bval;

	//shadow
	if (fconfig.Read("enable_shadow", &bval))
		m_vol_shw = bval;
	if (fconfig.Read("shadow_intensity", &val))
		m_vol_swi = val;

	//wavelength to color table
	m_vol_wav[0] = Color(1.0, 1.0, 1.0);
	m_vol_wav[1] = Color(1.0, 1.0, 1.0);
	m_vol_wav[2] = Color(1.0, 1.0, 1.0);
	m_vol_wav[3] = Color(1.0, 1.0, 1.0);

	//slice sequence
	m_sliceSequence = false;
	//compression
	m_compression = false;
	//skip brick
	m_skip_brick = false;
	//time sequence identifier
	m_timeId = "_T";
	//load mask
	m_load_mask = true;
}

DataManager::~DataManager()
{
	for (int i=0 ; i<(int)m_vd_list.size() ; i++)
		if (m_vd_list[i])
			delete m_vd_list[i];
	for (int i=0 ; i<(int)m_md_list.size() ; i++)
		if (m_md_list[i])
			delete m_md_list[i];
	for (int i=0; i<(int)m_reader_list.size(); i++)
		if (m_reader_list[i])
			delete m_reader_list[i];
	for (int i=0; i<(int)m_annotation_list.size(); i++)
		if (m_annotation_list[i])
			delete m_annotation_list[i];
}

void DataManager::ClearAll()
{
	for (int i=0 ; i<(int)m_vd_list.size() ; i++)
		if (m_vd_list[i])
			delete m_vd_list[i];
	for (int i=0 ; i<(int)m_md_list.size() ; i++)
		if (m_md_list[i])
			delete m_md_list[i];
	for (int i=0; i<(int)m_reader_list.size(); i++)
		if (m_reader_list[i])
			delete m_reader_list[i];
	for (int i=0; i<(int)m_annotation_list.size(); i++)
		if (m_annotation_list[i])
			delete m_annotation_list[i];
	m_vd_list.clear();
	m_md_list.clear();
	m_reader_list.clear();
	m_annotation_list.clear();
}

void DataManager::SetVolumeDefault(VolumeData* vd)
{
	if (m_use_defaults)
	{
		vd->SetWireframe(m_vol_test_wiref);
		vd->Set3DGamma(m_vol_gam);
		vd->SetBoundary(m_vol_exb);
		vd->SetOffset(m_vol_of1);
		vd->SetLeftThresh(m_vol_lth);
		vd->SetRightThresh(m_vol_hth);
		vd->SetAlpha(m_vol_alf);
		vd->SetSampleRate(m_vol_spr);
		double amb, diff, spec, shine;
		vd->GetMaterial(amb, diff, spec, shine);
		vd->SetMaterial(m_vol_lsh, diff, spec, m_vol_hsh);
		if (!vd->GetSpcFromFile())
			vd->SetBaseSpacings(m_vol_xsp, m_vol_ysp, m_vol_zsp);
		vd->SetColormapMode(m_vol_cmm);
		vd->SetColormap(m_vol_cmp);
		vd->SetColormapProj(m_vol_cmj);
		vd->SetColormapValues(m_vol_lcm, m_vol_hcm);

		vd->SetEnableAlpha(m_vol_eap);
		int resx, resy, resz;
		vd->GetResolution(resx, resy, resz);
		if (resz > 1)
			vd->SetShading(m_vol_esh);
		else
			vd->SetShading(false);
		vd->SetMode(m_vol_mip?1:0);
		vd->SetNR(m_vol_nrd);
		//inversion
		vd->SetInterpolate(m_vol_interp);
		//inversion
		vd->SetInvert(m_vol_inv);

		//shadow
		vd->SetShadow(m_vol_shw);
		vd->SetShadowParams(m_vol_swi);
	}
}

//set project path
//when data and project are moved, use project file's path
//if data's directory doesn't exist
void DataManager::SetProjectPath(wxString path)
{
	m_prj_path.Clear();
	m_prj_path = wxPathOnly(path);
}

wxString DataManager::SearchProjectPath(wxString &filename)
{
	int i;

	wxString pathname = filename;

	if (m_prj_path == "")
		return "";
	wxString search_str;
	for (i = pathname.Length() - 1; i >= 0; i--)
	{
		if (pathname[i] == '\\' || pathname[i] == '/')
		{
			search_str.Prepend(GETSLASH());
			wxString name_temp = m_prj_path + search_str;
			if (wxFileExists(name_temp))
				return name_temp;
		}
		else
			search_str.Prepend(pathname[i]);
	}
	return "";
}

int DataManager::LoadVolumeData(wxString &filename, int type, bool withImageJ, int ch_num, int t_num)
{
	bool isURL = false;
	bool downloaded = false;
	wxString downloaded_filepath;
	bool downloaded_metadata = false;
	wxString downloaded_metadatafilepath;

	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	int i;
	int result = 0;
	BaseReader* reader = 0;

	for (i=0; i<(int)m_reader_list.size(); i++)
	{
		wstring wstr = pathname.ToStdWstring();
		if (m_reader_list[i]->Match(wstr))
		{
			reader = m_reader_list[i];
			break;
		}
	}

	int reader_return = -1;
	if (reader)
	{
		bool preprocess = false;
		if (reader->GetSliceSeq() != m_sliceSequence)
		{
			reader->SetSliceSeq(m_sliceSequence);
			preprocess = true;
		}
		if (reader->GetTimeId() != m_timeId.ToStdWstring())
		{
			wstring str_w = m_timeId.ToStdWstring();
			reader->SetTimeId(str_w);
			preprocess = true;
		}
		if (preprocess)
			reader_return = reader->Preprocess();
	}
	else
	{
		//RGB tiff
		//TODO: Loading with imageJ irrespective of the file type.
		if (withImageJ == true)
			reader = new ImageJReader();
		else {
			if (type == LOAD_TYPE_TIFF)
				reader = new TIFReader();
			else if (type == LOAD_TYPE_NRRD)
				reader = new NRRDReader();
			else if (type == LOAD_TYPE_OIB)
				reader = new OIBReader();
			else if (type == LOAD_TYPE_OIF)
				reader = new OIFReader();
			else if (type == LOAD_TYPE_LSM)
				reader = new LSMReader();
			else if (type == LOAD_TYPE_PVXML)
			{
				reader = new PVXMLReader();
				((PVXMLReader*)reader)->SetFlipX(m_pvxml_flip_x);
				((PVXMLReader*)reader)->SetFlipY(m_pvxml_flip_y);
			}
			else if (type == LOAD_TYPE_BRKXML)
				reader = new BRKXMLReader();
		}
		
		
		m_reader_list.push_back(reader);
		wstring str_w = pathname.ToStdWstring();
		reader->SetFile(str_w);
		reader->SetSliceSeq(m_sliceSequence);
		str_w = m_timeId.ToStdWstring();
		reader->SetTimeId(str_w);
		reader_return = reader->Preprocess();
	}

	if (reader_return > 0)
	{
		string err_str = BaseReader::GetError(reader_return);
		wxMessageBox(err_str);
		int i = (int)m_reader_list.size() - 1;		
		if (m_reader_list[i]) {
			delete m_reader_list[i];
			m_reader_list.erase(m_reader_list.begin() + (int)m_reader_list.size() - 1);
		}
		return result;
	}

	//align data for compression if vtc is not supported
	if (!GLEW_NV_texture_compression_vtc && m_compression)
	{
		reader->SetResize(1);
		reader->SetAlignment(4);
	}

	int chan = reader->GetChanNum();
	for (i=(ch_num>=0?ch_num:0);
		i<(ch_num>=0?ch_num+1:chan); i++)
	{
		VolumeData *vd = new VolumeData();
		if (!vd)
			continue;

		vd->SetSkipBrick(m_skip_brick);
		Nrrd* data = reader->Convert(t_num>=0?t_num:reader->GetCurTime(), i, true);
		if (!data)
			continue;

		wxString name;
		if (type != LOAD_TYPE_BRKXML)
		{
			name = wxString(reader->GetDataName());
			if (chan > 1)
				name += wxString::Format("_Ch%d", i + 1);
		}
		else
		{
			BRKXMLReader* breader = (BRKXMLReader*)reader;
			name = reader->GetDataName();
			name = name.Mid(0, name.find_last_of(wxT('.')));
			if (ch_num > 1) name = wxT("_Ch") + wxString::Format("%i", i);
			pathname = filename;
			breader->SetCurChan(i);
			breader->SetCurTime(0);
		}

		vd->SetReader(reader);
		vd->SetCompression(m_compression);

		bool valid_spc = reader->IsSpcInfoValid();
		if (vd->Load(data, name, pathname))
		{
			if (m_load_mask)
			{
				//mask
				MSKReader msk_reader;
				std::wstring str = reader->GetCurMaskName(t_num >= 0 ? t_num : reader->GetCurTime(), i);
				msk_reader.SetFile(str);
				Nrrd* mask = msk_reader.Convert(0, 0, true);
				if (mask)
					vd->LoadMask(mask);
				//label mask
				LBLReader lbl_reader;
				str = reader->GetCurLabelName(t_num >= 0 ? t_num : reader->GetCurTime(), i);
				lbl_reader.SetFile(str);
				Nrrd* label = lbl_reader.Convert(0, 0, true);
				if (label)
					vd->LoadLabel(label);
			}
			if (type == LOAD_TYPE_BRKXML) ((BRKXMLReader*)reader)->SetLevel(0);
			//for 2D data
			int xres, yres, zres;
			vd->GetResolution(xres, yres, zres);
			double zspcfac = (double)Max(xres, yres) / 256.0;
			if (zspcfac < 1.0) zspcfac = 1.0;
			double tester = reader->GetXSpc();
			if (zres == 1) vd->SetBaseSpacings(reader->GetXSpc(), reader->GetYSpc(), reader->GetXSpc()*zspcfac);
			else vd->SetBaseSpacings(reader->GetXSpc(), reader->GetYSpc(), reader->GetZSpc());
			vd->SetSpcFromFile(valid_spc);
			vd->SetScalarScale(reader->GetScalarScale());
			vd->SetMaxValue(reader->GetMaxValue());
			vd->SetCurTime(reader->GetCurTime());
			vd->SetCurChannel(i);
			//++
			result++;
		}
		else
		{
			delete vd;
			continue;
		}

		SetVolumeDefault(vd);
		AddVolumeData(vd);

		//get excitation wavelength
		double wavelength = reader->GetExcitationWavelength(i);
		if (wavelength > 0.0) {
			FLIVR::Color col = GetWavelengthColor(wavelength);
			vd->SetColor(col);
		}
		else if (wavelength < 0.) {
			FLIVR::Color white = Color(1.0, 1.0, 1.0);
			vd->SetColor(white);
		}
		else
		{
			FLIVR::Color white = Color(1.0, 1.0, 1.0);
			FLIVR::Color red   = Color(1.0, 0.0, 0.0);
			FLIVR::Color green = Color(0.0, 1.0, 0.0);
			FLIVR::Color blue  = Color(0.0, 0.0, 1.0);
			if (chan == 1) {
				vd->SetColor(white);
			}
			else
			{
				if (i == 0)
					vd->SetColor(red);
				else if (i == 1)
					vd->SetColor(green);
				else if (i == 2)
					vd->SetColor(blue);
				else
					vd->SetColor(white);
			}
		}

	}

	return result;
}

int DataManager::LoadMeshData(wxString &filename)
{
	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	MeshData *md = new MeshData();
	md->Load(pathname);

	wxString name = md->GetName();
	wxString new_name = name;
	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);
	if (i>1)
		md->SetName(new_name);
	m_md_list.push_back(md);

	return 1;
}

int DataManager::LoadMeshData(GLMmodel* mesh)
{
	if (!mesh) return 0;

	MeshData *md = new MeshData();
	md->Load(mesh);

	wxString name = md->GetName();
	wxString new_name = name;
	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);
	if (i>1)
		md->SetName(new_name);
	m_md_list.push_back(md);

	return 1;
}

VolumeData* DataManager::GetVolumeData(int index)
{
	if (index>=0 && index<(int)m_vd_list.size())
		return m_vd_list[index];
	else
		return 0;
}

MeshData* DataManager::GetMeshData(int index)
{
	if (index>=0 && index<(int)m_md_list.size())
		return m_md_list[index];
	else
		return 0;
}

VolumeData* DataManager::GetVolumeData(wxString &name)
{
	for (int i=0 ; i<(int)m_vd_list.size() ; i++)
	{
		if (name == m_vd_list[i]->GetName())
		{
			return m_vd_list[i];
		}
	}
	return 0;
}

MeshData* DataManager::GetMeshData(wxString &name)
{
	for (int i=0 ; i<(int)m_md_list.size() ; i++)
	{
		if (name == m_md_list[i]->GetName())
		{
			return m_md_list[i];
		}
	}
	return 0;
}

int DataManager::GetVolumeIndex(wxString &name)
{
	for (int i=0 ; i<(int)m_vd_list.size() ; i++)
	{
		if (!m_vd_list[i])
			continue;
		if (name == m_vd_list[i]->GetName())
		{
			return i;
		}
	}
	return -1;
}

int DataManager::GetMeshIndex(wxString &name)
{
	for (int i=0 ; i<(int)m_md_list.size() ; i++)
	{
		if (name == m_md_list[i]->GetName())
		{
			return i;
		}
	}
	return -1;
}

void DataManager::RemoveVolumeData(int index)
{
	VolumeData* data = m_vd_list[index];
	if (!data)
		return;
	
	for (auto iter = m_vd_list.begin();
		iter != m_vd_list.end();)
	{
		VolumeData* vd = *iter;
		bool del = false;
		if (vd)
		{
			if (vd == data)
				del = true;
			if (vd->GetDup())
			{
				if (vd->GetDupData() == data)
					del = true;
			}
		}
		if (del)
		{
			iter = m_vd_list.erase(iter);
			delete vd;
		}
		else
			++iter;
	}	
}

void DataManager::RemoveVolumeData(const wxString &name)
{
	for (int i = 0; i<(int)m_vd_list.size(); i++)
	{
		if (name == m_vd_list[i]->GetName())
		{
			RemoveVolumeData(i);
		}
	}
}

void DataManager::RemoveMeshData(int index)
{
	MeshData* data = m_md_list[index];
	if (data)
	{
		m_md_list.erase(m_md_list.begin()+index);
		delete data;
		data = 0;
	}
}

int DataManager::GetVolumeNum()
{
	return m_vd_list.size();
}

int DataManager::GetMeshNum()
{
	return m_md_list.size();
}

void DataManager::AddVolumeData(VolumeData* vd)
{
	if (!vd)
		return;

	wxString name = vd->GetName();
	wxString new_name = name;

	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);

	if (i>1)
		vd->SetName(new_name);

	if (m_override_vox)
	{
		if (m_vd_list.size() > 0)
		{
			double spcx, spcy, spcz;
			m_vd_list[0]->GetBaseSpacings(spcx, spcy, spcz);
			vd->SetSpacings(spcx, spcy, spcz);
			vd->SetBaseSpacings(spcx, spcy, spcz);
			//vd->SetSpcFromFile(true);
		}
	}
	m_vd_list.push_back(vd);
}

VolumeData* DataManager::DuplicateVolumeData(VolumeData* vd)
{
	VolumeData* vd_new = 0;

	if (vd)
	{
		vd_new = new VolumeData(*vd);
		AddVolumeData(vd_new);
	}

	return vd_new;
}

int DataManager::LoadAnnotations(wxString &filename)
{
	wxString pathname = filename;
	if (!wxFileExists(pathname))
	{
		pathname = SearchProjectPath(filename);
		if (!wxFileExists(pathname))
			return 0;
	}

	Annotations* ann = new Annotations();
	ann->Load(pathname, this);

	wxString name = ann->GetName();
	wxString new_name = name;
	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);
	if (i>1)
		ann->SetName(new_name);
	m_annotation_list.push_back(ann);

	return 1;
}

void DataManager::AddAnnotations(Annotations* ann)
{
	if (!ann)
		return;

	wxString name = ann->GetName();
	wxString new_name = name;

	int i;
	for (i=1; CheckNames(new_name); i++)
		new_name = name+wxString::Format("_%d", i);

	if (i>1)
		ann->SetName(new_name);

	m_annotation_list.push_back(ann);
}

void DataManager::RemoveAnnotations(int index)
{
	Annotations* ann = m_annotation_list[index];
	if (ann)
	{
		m_annotation_list.erase(m_annotation_list.begin()+index);
		delete ann;
		ann = 0;
	}
}

int DataManager::GetAnnotationNum()
{
	return m_annotation_list.size();
}

Annotations* DataManager::GetAnnotations(int index)
{
	if (index>=0 && index<(int)m_annotation_list.size())
		return m_annotation_list[index];
	else
		return 0;
}

Annotations* DataManager::GetAnnotations(wxString &name)
{
	for (int i=0; i<(int)m_annotation_list.size(); i++)
	{
		if (name == m_annotation_list[i]->GetName())
			return m_annotation_list[i];
	}
	return 0;
}

int DataManager::GetAnnotationIndex(wxString &name)
{
	for (int i=0; i<(int)m_annotation_list.size(); i++)
	{
		if (!m_annotation_list[i])
			continue;
		if (name == m_annotation_list[i]->GetName())
			return i;
	}
	return -1;
}

bool DataManager::CheckNames(wxString &str)
{
	bool result = false;
	for (unsigned int i=0; i<m_vd_list.size(); i++)
	{
		VolumeData* vd = m_vd_list[i];
		if (vd && vd->GetName()==str)
		{
			result = true;
			break;
		}
	}
	if (!result)
	{
		for (unsigned int i=0; i<m_md_list.size(); i++)
		{
			MeshData* md = m_md_list[i];
			if (md && md->GetName()==str)
			{
				result = true;
				break;
			}
		}
	}
	if (!result)
	{
		for (unsigned int i=0; i<m_annotation_list.size(); i++)
		{
			Annotations* ann = m_annotation_list[i];
			if (ann && ann->GetName()==str)
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

Color DataManager::GetColor(int c)
{
	Color result(1.0, 1.0, 1.0);
	switch (c)
	{
	case 1://red
		result = Color(1.0, 0.0, 0.0);
		break;
	case 2://green
		result = Color(0.0, 1.0, 0.0);
		break;
	case 3://blue
		result = Color(0.0, 0.0, 1.0);
		break;
	case 4://cyan
		result = Color(0.0, 1.0, 1.0);
		break;
	case 5://magenta
		result = Color(1.0, 0.0, 1.0);
		break;
	case 6://yellow
		result = Color(1.0, 1.0, 0.0);
		break;
	case 7://orange
		result = Color(1.0, 0.5, 0.0);
		break;
	case 8://white
		result = Color(1.0, 1.0, 1.0);
		break;
	}
	return result;
}

void DataManager::SetWavelengthColor(int c1, int c2, int c3, int c4)
{
	m_vol_wav[0] = GetColor(c1);
	m_vol_wav[1] = GetColor(c2);
	m_vol_wav[2] = GetColor(c3);
	m_vol_wav[3] = GetColor(c4);
}

Color DataManager::GetWavelengthColor(double wavelength)
{
	if (wavelength < 340.0)
		return Color(1.0, 1.0, 1.0);
	else if (wavelength < 440.0)
		return m_vol_wav[0];
	else if (wavelength < 500.0)
		return m_vol_wav[1];
	else if (wavelength < 600.0)
		return m_vol_wav[2];
	else if (wavelength < 750.0)
		return m_vol_wav[3];
	else
		return Color(1.0, 1.0, 1.0);
}
