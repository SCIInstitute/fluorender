#include "DataManager.h"
#include "teem/Nrrd/nrrd.h"
#include <wx/msgdlg.h>
#include <wx/progdlg.h>
#include <wx/wfstream.h>
#include <wx/txtstrm.h>
#include "utility.h"
#include <sstream>
#include <fstream>
#include <algorithm>
#include <set>

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

	//legend
	m_legend = true;

	//valid brick number
	m_brick_num = 0;
}

VolumeData::VolumeData(VolumeData &copy)
{
	m_reader = 0;
	//duplication
	m_dup = true;
	copy.m_dup_counter++;
	m_dup_counter = copy.m_dup_counter;

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

	//skip brick
	m_skip_brick = false;

	//legend
	m_legend = true;

	//valid brick number
	m_brick_num = 0;
}

VolumeData::~VolumeData()
{
	if (m_tex && !m_dup)
		delete m_tex;
	if (m_vr)
		delete m_vr;
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
	m_tex->build(nv, gm, 0, 256, 0, 0);

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

	if (m_tex)
	{
		m_tex->get_spacings(spcx, spcy, spcz);
		delete m_tex;
	}
	m_tex = new Texture();
	m_tex->set_use_priority(m_skip_brick);
	m_tex->build(data->GetTexture()->get_nrrd(0), 0,
		0, data->GetMaxValue(), 0, 0);
	m_tex->set_spacings(spcx, spcy, spcz);
	data->GetTexture()->set_nrrd(0, 0);
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
							  double spcx, double spcy, double spcz)
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
		uint8 *val8 = new (std::nothrow) uint8[nx*ny*nz];
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
		uint16 *val16 = new (std::nothrow) uint16[nx*ny*nz];
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

	//prepare the texture bricks for the mask
	m_tex->add_empty_mask();
	m_tex->set_nrrd(mask, m_tex->nmask());
}

void VolumeData::AddEmptyMask()
{
	if (!m_tex || !m_vr)
		return;

	//prepare the texture bricks for the mask
	if (m_tex->add_empty_mask())
	{
		//add the nrrd data for mask
		Nrrd *nrrd_mask = nrrdNew();
		uint8 *val8 = new (std::nothrow) uint8[m_res_x*m_res_y*m_res_z];
		if (!val8)
		{
			wxMessageBox("Not enough memory. Please save project and restart.");
			return;
		}
		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		memset((void*)val8, 0, sizeof(uint8)*m_res_x*m_res_y*m_res_z);
		nrrdWrap(nrrd_mask, val8, nrrdTypeUChar, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_mask, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);

		m_tex->set_nrrd(nrrd_mask, m_tex->nmask());
	}
}

//volume label
void VolumeData::LoadLabel(Nrrd* label)
{
	if (!m_tex || !m_vr)
		return;

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

	//prepare the texture bricks for the labeling mask
	if (m_tex->add_empty_label())
	{
		//add the nrrd data for the labeling mask
		Nrrd *nrrd_label = nrrdNew();
		unsigned int *val32 = new (std::nothrow) unsigned int[m_res_x*m_res_y*m_res_z];
		if (!val32)
		{
			wxMessageBox("Not enough memory. Please save project and restart.");
			return;
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

		double spcx, spcy, spcz;
		m_tex->get_spacings(spcx, spcy, spcz);
		nrrdWrap(nrrd_label, val32, nrrdTypeUInt, 3, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSpacing, spcx, spcy, spcz);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMax, spcx*m_res_x, spcy*m_res_y, spcz*m_res_z);
		nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSize, (size_t)m_res_x, (size_t)m_res_y, (size_t)m_res_z);

		m_tex->set_nrrd(nrrd_label, m_tex->nlabel());
	}
}

Nrrd* VolumeData::GetMask()
{
	if (m_vr && m_tex && m_tex->nmask()!=-1)
	{
		m_vr->return_mask();
		return m_tex->get_nrrd(m_tex->nmask());
	}

	return 0;
}

double VolumeData::GetOriginalValue(int i, int j, int k)
{
	Nrrd* data = m_tex->get_nrrd(0);
    if (!data) return 0.0;
    
    int bits = data->type;
    uint64_t nx = (uint64_t)(data->axis[0].size);
    uint64_t ny = (uint64_t)(data->axis[1].size);
    uint64_t nz = (uint64_t)(data->axis[2].size);
    
    if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
        return 0.0;
    uint64_t ii = i, jj = j, kk = k;
    
    if (bits == nrrdTypeUChar)
    {
        uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
        uint8 old_value = ((uint8*)(data->data))[index];
        return double(old_value)/255.0;
    }
    else if (bits == nrrdTypeUShort)
    {
        uint64_t index = (nx)*(ny)*(kk) + (nx)*(jj) + (ii);
        uint16 old_value = ((uint16*)(data->data))[index];
        return double(old_value)*m_scalar_scale/65535.0;
    }
    
    return 0.0;
}

double VolumeData::GetTransferedValue(int i, int j, int k)
{
	Nrrd* data = m_tex->get_nrrd(0);
	if (!data) return 0.0;

    int bits = data->type;
    uint64_t nx = (uint64_t)(data->axis[0].size);
    uint64_t ny = (uint64_t)(data->axis[1].size);
    uint64_t nz = (uint64_t)(data->axis[2].size);
	if (i<0 || i>=nx || j<0 || j>=ny || k<0 || k>=nz)
		return 0.0;
    uint64_t ii = i, jj = j, kk = k;

	if (bits == nrrdTypeUChar)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
		uint8 old_value = ((uint8*)(data->data))[index];
		double gm = 0.0;
		double new_value = double(old_value)/255.0;
		if (m_vr->get_inversion())
			new_value = 1.0-new_value;
		if (i>0 && i<nx-1 &&
			j>0 && j<ny-1 &&
			k>0 && k<nz-1)
		{
			double v1 = ((uint8*)(data->data))[nx*ny*kk + nx*jj + ii-1];
            double v2 = ((uint8*)(data->data))[nx*ny*kk + nx*jj + ii+1];
            double v3 = ((uint8*)(data->data))[nx*ny*kk + nx*(jj-1) + ii];
            double v4 = ((uint8*)(data->data))[nx*ny*kk + nx*(jj+1) + ii];
            double v5 = ((uint8*)(data->data))[nx*ny*(kk-1) + nx*jj + ii];
            double v6 = ((uint8*)(data->data))[nx*ny*(kk+1) + nx*jj + ii];
            double normal_x, normal_y, normal_z;
            normal_x = (v2 - v1) / 255.0;
            normal_y = (v4 - v3) / 255.0;
            normal_z = (v6 - v5) / 255.0;
            gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
		}
		if (new_value<m_lo_thresh-m_sw ||
			new_value>m_hi_thresh+m_sw ||
			gm<m_gm_thresh)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / m_gamma3d;
			new_value = (new_value<m_lo_thresh?
				(m_sw-m_lo_thresh+new_value)/m_sw:
				(new_value>m_hi_thresh?
				(m_sw-new_value+m_hi_thresh)/m_sw:1.0))
				*new_value;
			new_value = pow(Clamp(new_value/m_offset,
				gamma<1.0?-(gamma-1.0)*0.00001:0.0,
				gamma>1.0?0.9999:1.0), gamma);
			new_value *= m_alpha;
		}
		return new_value;
	}
	else if (bits == nrrdTypeUShort)
	{
		uint64_t index = nx*ny*kk + nx*jj + ii;
        uint16 old_value = ((uint16*)(data->data))[index];
        double gm = 0.0;
        double new_value = double(old_value)*m_scalar_scale/65535.0;
        if (m_vr->get_inversion())
            new_value = 1.0-new_value;
        if (ii>0 && ii<nx-1 &&
            jj>0 && jj<ny-1 &&
            kk>0 && kk<nz-1)
        {
            double v1 = ((uint8*)(data->data))[nx*ny*kk + nx*jj + ii-1];
            double v2 = ((uint8*)(data->data))[nx*ny*kk + nx*jj + ii+1];
            double v3 = ((uint8*)(data->data))[nx*ny*kk + nx*(jj-1) + ii];
            double v4 = ((uint8*)(data->data))[nx*ny*kk + nx*(jj+1) + ii];
            double v5 = ((uint8*)(data->data))[nx*ny*(kk-1) + nx*jj + ii];
            double v6 = ((uint8*)(data->data))[nx*ny*(kk+1) + nx*jj + ii];
            double normal_x, normal_y, normal_z;
            normal_x = (v2 - v1)*m_scalar_scale/65535.0;
            normal_y = (v4 - v3)*m_scalar_scale/65535.0;
            normal_z = (v6 - v5)*m_scalar_scale/65535.0;
            gm = sqrt(normal_x*normal_x + normal_y*normal_y + normal_z*normal_z)*0.53;
        }
		if (new_value<m_lo_thresh-m_sw ||
			new_value>m_hi_thresh+m_sw ||
			gm<m_gm_thresh)
			new_value = 0.0;
		else
		{
			double gamma = 1.0 / m_gamma3d;
			new_value = (new_value<m_lo_thresh?
				(m_sw-m_lo_thresh+new_value)/m_sw:
				(new_value>m_hi_thresh?
				(m_sw-new_value+m_hi_thresh)/m_sw:1.0))
				*new_value;
			new_value = pow(Clamp(new_value/m_offset,
				gamma<1.0?-(gamma-1.0)*0.00001:0.0,
				gamma>1.0?0.9999:1.0), gamma);
			new_value *= m_alpha;
		}
		return new_value;
	}

	return 0.0;
}

//save
void VolumeData::Save(wxString &filename, int mode, bool bake, bool compress)
{
	if (m_vr && m_tex)
	{
		Nrrd* data = 0;

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

				Nrrd* baked_data = nrrdNew();
				if (bits == 8)
				{
					uint8 *val8 = new (std::nothrow) uint8[nx*ny*nz];
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
							double new_value = GetTransferedValue(i, j, k);
							val8[index] = uint8(new_value*255.0);
						}
					}
					nrrdWrap(baked_data, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
				}
				else if (bits == 16)
				{
					uint16 *val16 = new (std::nothrow) uint16[nx*ny*nz];
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
							double new_value = GetTransferedValue(i, j, k);
							val16[index] = uint16(new_value*65535.0);
						}
					}
					nrrdWrap(baked_data, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
				}
				nrrdAxisInfoSet(baked_data, nrrdAxisInfoSpacing, spcx, spcy, spcz);
				nrrdAxisInfoSet(baked_data, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);
				nrrdAxisInfoSet(baked_data, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
				nrrdAxisInfoSet(baked_data, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);

				writer->SetData(baked_data);
				writer->SetSpacings(spcx, spcy, spcz);
				writer->SetCompression(compress);
				writer->Save(filename.ToStdWstring(), mode);

				//free memory
				nrrdNix(baked_data);

				prg_diag->Update(100);
				delete prg_diag;
			}
			else
			{
				writer->SetData(data);
				writer->SetSpacings(spcx, spcy, spcz);
				writer->SetCompression(compress);
				writer->Save(filename.ToStdWstring(), mode);
			}
		}
		delete writer;

		//save mask
		if (m_tex->nmask() != -1)
		{
			m_vr->return_mask();
			data = m_tex->get_nrrd(m_tex->nmask());
			if (data)
			{
				MSKWriter msk_writer;
				msk_writer.SetData(data);
				msk_writer.SetSpacings(spcx, spcy, spcz);
				msk_writer.Save(filename.ToStdWstring(), 0);
			}
		}

		//save label
		if (m_tex->nlabel() != -1)
		{
			data = m_tex->get_nrrd(m_tex->nlabel());
			if (data)
			{
				MSKWriter msk_writer;
				msk_writer.SetData(data);
				msk_writer.SetSpacings(spcx, spcy, spcz);
				msk_writer.Save(filename.ToStdWstring(), 1);
			}
		}

		m_tex_path = filename;
	}
}

//bounding box
BBox VolumeData::GetBounds()
{
	return m_bounds;
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

//draw volume
void VolumeData::Draw(bool ortho, bool interactive, double zoom, bool intp)
{
	glPushMatrix();
	glScalef(m_sclx, m_scly, m_sclz);
	if (m_vr)
	{
		m_vr->draw(m_test_wiref, interactive, ortho, zoom, intp, m_stream_mode);
	}
	if (m_draw_bounds)
		DrawBounds();
	glPopMatrix();
}

void VolumeData::DrawBounds()
{
	glBegin(GL_LINE_LOOP);
		glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.min().z());
		glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.min().z());
		glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.min().z());
		glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.min().z());
	glEnd();
	glBegin(GL_LINE_LOOP);
		glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.max().z());
		glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.max().z());
		glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.max().z());
		glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.max().z());
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.min().z());
		glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.max().z());
		glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.min().z());
		glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.max().z());
		glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.min().z());
		glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.max().z());
		glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.min().z());
		glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.max().z());
	glEnd();

}

//draw mask (create the mask)
//type: 0-initial; 1-diffusion-based growing
//paint_mode: 1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear, 7-all
//			  11-posterize
//hr_mode (hidden removal): 0-none; 1-ortho; 2-persp
void VolumeData::DrawMask(int type, int paint_mode, int hr_mode,
						  double ini_thresh, double gm_falloff, double scl_falloff, double scl_translate,
						  double w2d, double bins, bool ortho)
{
	glPushMatrix();
	glScalef(m_sclx, m_scly, m_sclz);
	if (m_vr)
	{
		m_vr->set_2d_mask(m_2d_mask);
		m_vr->set_2d_weight(m_2d_weight1, m_2d_weight2);
		m_vr->draw_mask(type, paint_mode, hr_mode, ini_thresh, gm_falloff, scl_falloff, scl_translate, w2d, bins, ortho);
	}
	glPopMatrix();
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

Color VolumeData::GetMaskColor()
{
	Color result;
	if (m_vr)
		result = m_vr->get_mask_color();
	return result;
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

//resolution  scaling and spacing
void VolumeData::GetResolution(int &res_x, int &res_y, int &res_z)
{
	res_x = m_res_x;
	res_y = m_res_y;
	res_z = m_res_z;
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

void VolumeData::GetSpacings(double &spcx, double &spcy, double & spcz)
{
	if (GetTexture())
		GetTexture()->get_spacings(spcx, spcy, spcz);
}

void VolumeData::GetFileSpacings(double &spcx, double &spcy, double &spcz)
{
	spcx = m_spcx; spcy = m_spcy; spcz = m_spcz;
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MeshData::MeshData() :
m_data(0),
m_mr(0),
m_center(0.0, 0.0, 0.0),
m_disp(true),
m_draw_bounds(false),
m_light(true),
m_mat_amb(0.5, 0.5, 0.5),
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
#ifdef _WIN32
    wxChar slash = '\\';
#else
    wxChar slash = '/';
#endif
	m_name = m_data_path.Mid(m_data_path.Find(slash, true)+1);

	if (m_data)
		delete m_data;

	string str_fn = filename.ToStdString();
	m_data = glmReadOBJ(str_fn.c_str());
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

void MeshData::Draw(int peel)
{
	if (!m_mr)
		return;
	glPushMatrix();
	glTranslated(m_trans[0]+m_center.x(), 
				 m_trans[1]+m_center.y(), 
				 m_trans[2]+m_center.z());
	glRotated(m_rot[0], 1, 0, 0);
	glRotated(m_rot[1], 0, 1, 0);
	glRotated(m_rot[2], 0, 0, 1);
	glScaled(m_scale[0], m_scale[1], m_scale[2]);
	glTranslated(-m_center.x(), -m_center.y(), -m_center.z());

	if (m_light)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	}
	m_mr->set_depth_peel(peel);
	//glEnable(GL_MULTISAMPLE);
	m_mr->draw();
	//glDisable(GL_MULTISAMPLE);
	if (m_light)
		glDisable(GL_LIGHTING);

	if (m_draw_bounds && (peel==4 || peel==5))
		DrawBounds();
	glPopMatrix();
}

void MeshData::DrawBounds()
{
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_FOG);
	//glDisable(GL_BLEND);
	glColor4d(m_mat_diff.r(), m_mat_diff.g(), m_mat_diff.b(), 1.0);

	//glBegin(GL_LINE_LOOP);
	//	glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.min().z());
	//glEnd();
	//glBegin(GL_LINE_LOOP);
	//	glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.max().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.max().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.max().z());
	//	glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.max().z());
	//glEnd();
	//glBegin(GL_LINES);
	//	glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.min().x(), m_bounds.min().y(), m_bounds.max().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.min().y(), m_bounds.max().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.max().x(), m_bounds.max().y(), m_bounds.max().z());
	//	glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.min().z());
	//	glVertex3f(m_bounds.min().x(), m_bounds.max().y(), m_bounds.max().z());
	//glEnd();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(1.5);
	m_mr->draw(false, false);
	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glPopAttrib();
}

//lighting
void MeshData::SetLighting(bool bVal)
{
	m_light = bVal;
}

bool MeshData::GetLighting()
{
	return m_light;
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
		m_mr->update();
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
			m_mr->update();
		}
		break;
	case MESH_COLOR_DIFF:
		m_mat_diff = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].diffuse[0] = m_mat_diff.r();
			m_data->materials[0].diffuse[1] = m_mat_diff.g();
			m_data->materials[0].diffuse[2] = m_mat_diff.b();
			m_mr->update();
		}
		break;
	case MESH_COLOR_SPEC:
		m_mat_spec = color;
		if (m_data && m_data->materials)
		{
			m_data->materials[0].specular[0] = m_mat_spec.r();
			m_data->materials[0].specular[1] = m_mat_spec.g();
			m_data->materials[0].specular[2] = m_mat_spec.b();
			m_mr->update();
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
			m_mr->update();
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
			m_mr->update();
		}
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
	m_mr->update();
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
		m_mr->update();
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
	//font
	m_font = BITMAP_FONT_TYPE_HELVETICA_12;
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

//(nx, ny): window size
void Annotations::Draw(bool persp)
{
	double matrix[16];
	Transform mv;
	Transform p;
	glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
	mv.set(matrix);
	glGetDoublev(GL_PROJECTION_MATRIX, matrix);
	p.set(matrix);

	beginRenderText(2, 2, true);
	for (int i=0; i<(int)m_alist.size(); i++)
	{
		AText* atext = m_alist[i];
		if (atext)
		{
			Point pos = atext->m_pos;
			if (!InsideClippingPlanes(pos))
				continue;
			if (m_tform)
				pos = m_tform->transform(pos);
			pos = mv.transform(pos);
			pos = p.transform(pos);
			if (pos.x() >= -1.0 && pos.x() <= 1.0 &&
				pos.y() >= -1.0 && pos.y() <= 1.0)
			{
				if (persp && (pos.z()<=0.0 || pos.z()>=1.0))
					continue;
				if (!persp && (pos.z()>=0.0 || pos.z()<=-1.0))
					continue;
				renderText(pos.x()+1.0, 1.0-pos.y(),
					m_font,
					atext->m_txt.c_str());
			}
		}
	}
	endRenderText();
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
	m_num++;
	m_name = wxString::Format("Ruler %d", m_num);
	m_disp = true;
	m_tform = 0;
	m_ruler_type = 0;
	m_finished = false;

	//time-dependent
	m_time_dep = false;
	m_time = 0;

	//font
	m_font = BITMAP_FONT_TYPE_HELVETICA_12;
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
	if (index>=0 && index<m_ruler.size())
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

	for (int i=1; i<(int)m_ruler.size(); i++)
	{
		Point p1 = m_ruler[i-1];
		Point p2 = m_ruler[i];
		length += (p2-p1).length();
	}

	return length;
}

double Ruler::GetAngle()
{
	double angle = 0.0;

	if (m_ruler_type == 0)
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
	else if (m_ruler_type == 1)
	{
	}

	return angle;
}

bool Ruler::AddPoint(Point &point)
{
	if (m_ruler_type == 2 &&
		m_ruler.size() == 1)
		return false;
	else if (m_ruler_type == 0 &&
		m_ruler.size() == 2)
		return false;
	else
	{
		m_ruler.push_back(point);
		if (m_ruler_type == 2 &&
			m_ruler.size() == 1)
			m_finished = true;
		else if (m_ruler_type == 0 &&
			m_ruler.size() == 2)
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

void Ruler::Draw(bool persp, double asp)
{
	double matrix[16];
	Transform mv;
	Transform p;
	glGetDoublev(GL_MODELVIEW_MATRIX, matrix);
	mv.set(matrix);
	glGetDoublev(GL_PROJECTION_MATRIX, matrix);
	p.set(matrix);

	double width = 1;
	switch (m_font)
	{
	case BITMAP_FONT_TYPE_HELVETICA_10:
	case BITMAP_FONT_TYPE_HELVETICA_12:
	case BITMAP_FONT_TYPE_TIMES_ROMAN_10:
		width = 1;
		break;
	case BITMAP_FONT_TYPE_HELVETICA_18:
		width = 2;
		break;
	case BITMAP_FONT_TYPE_TIMES_ROMAN_24:
		width = 3;
		break;
	}
	glLineWidth(GLfloat(width));
	beginRenderText(2, 2, true);
	double dx = 0.01*width/asp;
	double dy = 0.01*width;
	for (int i=0; i<(int)m_ruler.size(); i++)
	{
		Point p1, p2;
		p2 = m_ruler[i];
		//if (m_tform)
		//	p2 = m_tform->transform(p2);
		p2 = mv.transform(p2);
		p2 = p.transform(p2);
		if ((persp && (p2.z()<=0.0 || p2.z()>=1.0)) ||
			(!persp && (p2.z()>=0.0 || p2.z()<=-1.0)))
			continue;
		glBegin(GL_LINE_LOOP);
			glVertex2d(p2.x()+1.0-dx, 1.0-dy-p2.y());
			glVertex2d(p2.x()+1.0+dx, 1.0-dy-p2.y());
			glVertex2d(p2.x()+1.0+dx, 1.0+dy-p2.y());
			glVertex2d(p2.x()+1.0-dx, 1.0+dy-p2.y());
		glEnd();
		if (i+1 == m_ruler.size())
			renderText(p2.x()+1.02, 1.01-p2.y(),
			m_font,
			m_name.To8BitData().data());
		if (m_ruler_type==2 && i==0)
		{
			glBegin(GL_POINTS);
				glVertex2d(p2.x()+1.0, 1.0-p2.y());
			glEnd();
		}
		if (i > 0)
		{
			p1 = m_ruler[i-1];
			if (m_tform)
				p1 = m_tform->transform(p1);
			p1 = mv.transform(p1);
			p1 = p.transform(p1);
			if ((persp && (p1.z()<=0.0 || p1.z()>=1.0)) ||
				(!persp && (p1.z()>=0.0 || p1.z()<=-1.0)))
				continue;
			glBegin(GL_LINES);
				glVertex2d(p1.x()+1.0, 1.0-p1.y());
				glVertex2d(p2.x()+1.0, 1.0-p2.y());
			glEnd();
		}
	}
	endRenderText();
	glLineWidth(1.0f);
}

wxString Ruler::GetDelInfoValues(wxString del)
{
	wxString output;

	for (int i=0; i<m_info_values.length(); i++)
	{
		if (m_info_values[i] == '\t')
			output += del;
		else
			output += m_info_values[i];
	}

	return output;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Vertex::Read(ifstream &ifs)
{
	if (ifs.bad())
		return 0;

	unsigned char tag;
	ifs.read((char*)(&tag), sizeof(tag));
	if (tag != TAG_VERT)
		return 0;

	//cell
	ifs.read((char*)(&tag), sizeof(tag));
	if (tag != TAG_CELL)
		return 0;
	//id
	ifs.read((char*)(&id), sizeof(id));
	ifs.read((char*)(&vsize), sizeof(vsize));
	double x, y, z;
	ifs.read((char*)(&x), sizeof(double));
	ifs.read((char*)(&y), sizeof(double));
	ifs.read((char*)(&z), sizeof(double));
	center = Point(x, y, z);

	//out edges (ids)
	ifs.read((char*)(&tag), sizeof(tag));
	if (tag != TAG_EDGE)
		return 0;
	unsigned int num = 0;
	ifs.read((char*)(&num), sizeof(num));
	unsigned int tempi;
	for (unsigned int i=0; i<num; ++i)
	{
		ifs.read((char*)(&tempi), sizeof(tempi));
		out_ids.push_back(tempi);
	}
	//in edges (ids)
	ifs.read((char*)(&tag), sizeof(tag));
	if (tag != TAG_EDGE)
		return 0;
	ifs.read((char*)(&num), sizeof(num));
	for (unsigned int i=0; i<num; ++i)
	{
		ifs.read((char*)(&tempi), sizeof(tempi));
		in_ids.push_back(tempi);
	}

	return 1;
}
int Vertex::Write(ofstream &ofs)
{
	if (ofs.bad())
		return 0;

	//tag vertex
	unsigned char tag = TAG_VERT;
	ofs.write(reinterpret_cast<const char*>(&tag),
		sizeof(unsigned char));

	//cell
	//tag cell
	tag = TAG_CELL;
	ofs.write(reinterpret_cast<const char*>(&tag),
		sizeof(unsigned char));
	//id etc
	ofs.write(reinterpret_cast<const char*>(&id),
		sizeof(unsigned int));
	ofs.write(reinterpret_cast<const char*>(&vsize),
		sizeof(unsigned int));
	double x, y, z;
	x = center.x();
	y = center.y();
	z = center.z();
	ofs.write(reinterpret_cast<const char*>(&x),
		sizeof(double));
	ofs.write(reinterpret_cast<const char*>(&y),
		sizeof(double));
	ofs.write(reinterpret_cast<const char*>(&z),
		sizeof(double));

	//out edges
	unsigned int num = 0;
	unsigned int id = 0;
	//tag edge
	tag = TAG_EDGE;
	ofs.write(reinterpret_cast<const char*>(&tag),
		sizeof(unsigned char));
	//num
	num = (unsigned int)out_ids.size();
	ofs.write(reinterpret_cast<const char*>(&num),
		sizeof(unsigned int));
	//id
	for (unsigned int i=0; i<num; ++i)
	{
		id = out_ids[i];
		ofs.write(reinterpret_cast<const char*>(&id),
			sizeof(unsigned int));
	}
	//in edges
	//tag edge
	tag = TAG_EDGE;
	ofs.write(reinterpret_cast<const char*>(&tag),
		sizeof(unsigned char));
	//num
	num = (unsigned int)in_ids.size();
	ofs.write(reinterpret_cast<const char*>(&num),
		sizeof(unsigned int));
	//id
	for (unsigned int i=0; i<num; ++i)
	{
		id = in_ids[i];
		ofs.write(reinterpret_cast<const char*>(&id),
			sizeof(unsigned int));
	}

	return 1;
}

int Frame::Read(ifstream &ifs)
{
	if (ifs.bad())
		return 0;

	//id
	ifs.read((char*)(&id), sizeof(id));
	//cell map
	if (!ReadCellMap(ifs))
		return 0;
	return 1;
}
int Frame::ReadCellMap(ifstream &ifs)
{
	if (ifs.bad())
		return 0;

	unsigned char tag = 0;
	ifs.read((char*)(&tag), sizeof(tag));
	if (tag != TAG_CMAP)
		return 0;

	unsigned int num = 0;
	ifs.read((char*)(&num), sizeof(num));

	for (unsigned int i=0; i<num; i++)
	{
		Vertex vertex;
		if (vertex.Read(ifs))
			cell_map.insert(pair<unsigned int, Vertex>(vertex.id, vertex));
		else
			return 0;
	}

	return 1;
}

int Frame::Write(ofstream &ofs)
{
	if (ofs.bad())
		return 0;

	//id
	ofs.write(reinterpret_cast<const char*>(&id),
		sizeof(unsigned int));
	//cell map
	if (!WriteCellMap(ofs))
		return 0;
	return 1;
}
int Frame::WriteCellMap(ofstream &ofs)
{
	if (ofs.bad())
		return 0;

	//tag
	unsigned char tag = TAG_CMAP;
	ofs.write(reinterpret_cast<const char*>(&tag),
		sizeof(unsigned char));

	//num
	unsigned int num = cell_map.size();
	ofs.write(reinterpret_cast<const char*>(&num),
		sizeof(unsigned int));

	//vertices
	for (CellMapIter iter = cell_map.begin();
		iter != cell_map.end(); ++iter)
	{
		iter->second.Write(ofs);
	}

	return 1;
}
int TraceGroup::m_num = 0;
TraceGroup::TraceGroup()
{
	type = 8;//traces
	m_num++;
	m_name = wxString::Format("Traces %d", m_num);
	m_cur_time = -1;
	m_prv_time = -1;
	m_ghost_num = 10;
	m_cell_size = 20;

	//font
	m_font = BITMAP_FONT_TYPE_HELVETICA_12;
}

TraceGroup::~TraceGroup()
{
}

void TraceGroup::ClearIDMap()
{
	m_id_map.clear();
}

void TraceGroup::SetCurTime(int time)
{
	m_cur_time = time;
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

void TraceGroup::AddID(Lbl lbl)
{
	m_id_map.insert(pair<unsigned int, Lbl>(lbl.id, lbl));
}

//sel_labels: ids from previous time point
//m_prv_time: previous time value
//m_id_map: ids of current time point that are linked to previous
//m_cur_time: current time value
//time values are check with frame ids in the frame list
void TraceGroup::SetIDMap(boost::unordered_map<unsigned int, Lbl> &sel_labels)
{
	m_id_map.clear();
	boost::unordered_map<unsigned int, Lbl>::iterator label_iter;

	//why does the time not change?
	//because I just want to find out the current selection
	if (m_prv_time == m_cur_time)
	{
		//copy sel_labels to m_id_map
		for (label_iter = sel_labels.begin();
			label_iter != sel_labels.end();
			++label_iter)
		{
			unsigned int id = label_iter->second.id;
			if (label_iter->second.size > (unsigned int)m_cell_size)
				m_id_map.insert(pair<unsigned int, Lbl>(id, label_iter->second));
		}
		return;
	}

	//get two cell maps
	CellMap *cell_map1 = 0;
	CellMap *cell_map2 = 0;
	FrameIter frame_iter;
	frame_iter = m_frame_list.find((unsigned int)m_prv_time);
	if (frame_iter == m_frame_list.end())
		return;
	cell_map1 = &(frame_iter->second.cell_map);
	frame_iter = m_frame_list.find((unsigned int)m_cur_time);
	if (frame_iter == m_frame_list.end())
		return;
	cell_map2 = &(frame_iter->second.cell_map);

	//decide which side to go
	bool out_edge = m_prv_time < m_cur_time;
	unsigned int id = 0;
	CellMapIter cell_map_iter1, cell_map_iter2;
	vector<unsigned int> *id_list = 0;
	unsigned int i = 0;
	Lbl lbl;
	//go through sel_labels
	for (label_iter = sel_labels.begin();
		label_iter != sel_labels.end();
		++label_iter)
	{
		id = label_iter->second.id;
		if (label_iter->second.size <= (unsigned int)m_cell_size)
			continue;
		//find id in cell map 1
		cell_map_iter1 = cell_map1->find(id);
		if (cell_map_iter1 == cell_map1->end())
			continue;
		//get correct id list
		id_list = out_edge?&(cell_map_iter1->second.out_ids):
			&(cell_map_iter1->second.in_ids);
		for (i=0; i<id_list->size(); ++i)
		{
			id = (*id_list)[i];
			//add to id map
			lbl.id = id;
			lbl.size = 0;
			lbl.center = Point();
			cell_map_iter2 = cell_map2->find(id);
			if (cell_map_iter2 != cell_map2->end())
			{
				lbl.size = cell_map_iter2->second.vsize;
				lbl.center = cell_map_iter2->second.center;
			}
			m_id_map.insert(pair<unsigned int, Lbl>(id, lbl));
		}
	}
}

IDMap* TraceGroup::GetIDMap()
{
	return &m_id_map;
}

bool TraceGroup::FindID(unsigned int id)
{
	IDMapIter iter = m_id_map.find(id);
	return (iter != m_id_map.end());
}
//find id in frame list
bool TraceGroup::FindIDInFrame(unsigned int id, int time, Vertex &vertex)
{
	if (m_frame_list.empty())
		return false;
	//get current frame
	FrameIter frame_iter = m_frame_list.find((unsigned int)time);
	if (frame_iter == m_frame_list.end())
		return false;
	CellMap *cell_map = &(frame_iter->second.cell_map);
	CellMapIter cell_map_iter = cell_map->find(id);
	if (cell_map_iter == cell_map->end())
		return false;
	vertex = cell_map_iter->second;
	return true;
}

//modifications
//link two vertices
bool TraceGroup::LinkVertices(unsigned int id1, int time1,
							  unsigned int id2, int time2,
							  bool exclusive)
{
	//check validity
	if (time2!=time1+1 && time2!=time1-1)
		return false;//not consecutive frames

	FrameIter frame_iter;
	CellMap* cell_map;
	CellMapIter cell_map_iter;
	//get selected vertex in current frame
	frame_iter = m_frame_list.find((unsigned int)time1);
	if (frame_iter == m_frame_list.end())
		return false;
	cell_map = &(frame_iter->second.cell_map);
	cell_map_iter = cell_map->find(id1);
	if (cell_map_iter == cell_map->end())
		return false;
	Vertex* vertex1 = &(cell_map_iter->second);
	//get selected vertex in previous frame
	frame_iter = m_frame_list.find((unsigned int)time2);
	if (frame_iter == m_frame_list.end())
		return false;
	cell_map = &(frame_iter->second.cell_map);
	cell_map_iter = cell_map->find(id2);
	if (cell_map_iter == cell_map->end())
		return false;
	Vertex* vertex2 = &(cell_map_iter->second);

	//add id to each other
	if (time2 == time1+1)
	{
		if (exclusive)
		{
			vertex1->out_ids.clear();
			vertex2->in_ids.clear();
		}
		//vertex1: add id2 to out ids
		vertex1->out_ids.push_back(id2);
		//vertex2: add id1 to in ids
		vertex2->in_ids.push_back(id1);
	}
	else if (time2 == time1-1)
	{
		if (exclusive)
		{
			vertex1->in_ids.clear();
			vertex2->out_ids.clear();
		}
		//vertex1: add id2 to in ids
		vertex1->in_ids.push_back(id2);
		//vertex2: add id1 to out ids
		vertex2->out_ids.push_back(id1);
	}

	return true;
}

bool TraceGroup::UnlinkVertices(unsigned int id1, int time1,
								unsigned int id2, int time2)
{
	//check validity
	if (time2!=time1+1 && time2!=time1-1)
		return false;//not consecutive frames

	FrameIter frame_iter;
	CellMap* cell_map;
	CellMapIter cell_map_iter;
	//get selected vertex in current frame
	frame_iter = m_frame_list.find((unsigned int)time1);
	if (frame_iter == m_frame_list.end())
		return false;
	cell_map = &(frame_iter->second.cell_map);
	cell_map_iter = cell_map->find(id1);
	if (cell_map_iter == cell_map->end())
		return false;
	Vertex* vertex1 = &(cell_map_iter->second);
	//get selected vertex in previous frame
	frame_iter = m_frame_list.find((unsigned int)time2);
	if (frame_iter == m_frame_list.end())
		return false;
	cell_map = &(frame_iter->second.cell_map);
	cell_map_iter = cell_map->find(id2);
	if (cell_map_iter == cell_map->end())
		return false;
	Vertex* vertex2 = &(cell_map_iter->second);

	//delete from each other
	vector<unsigned int>::iterator id_iter;
	if (time2 == time1+1)
	{
		//vertex1: find id2 in out ids
		id_iter = find(vertex1->out_ids.begin(),
			vertex1->out_ids.end(), id2);
		if (id_iter != vertex1->out_ids.end())
			vertex1->out_ids.erase(id_iter);
		//vertex2: find id1 in in ids
		id_iter = find(vertex2->in_ids.begin(),
			vertex2->in_ids.end(), id1);
		if (id_iter != vertex2->in_ids.end())
			vertex2->in_ids.erase(id_iter);
	}
	else if (time2 == time1-1)
	{
		//vertex1: find id2 in in ids
		id_iter = find(vertex1->in_ids.begin(),
			vertex1->in_ids.end(), id2);
		if (id_iter != vertex1->in_ids.end())
			vertex1->in_ids.erase(id_iter);
		//vertex2: find id1 in out ids
		id_iter = find(vertex2->out_ids.begin(),
			vertex2->out_ids.end(), id1);
		if (id_iter != vertex2->out_ids.end())
			vertex2->out_ids.erase(id_iter);
	}

	return true;
}

bool TraceGroup::AddVertex(int time, unsigned int id,
	unsigned int vsize, Point& center)
{
	FrameIter frame_iter;
	CellMap* cell_map;
	CellMapIter cell_map_iter;

	//get cell map
	frame_iter = m_frame_list.find((unsigned int)time);
	if (frame_iter == m_frame_list.end())
		cell_map = &(AddFrame(time)->cell_map);
	else
		cell_map = &(frame_iter->second.cell_map);

	//create vertex
	Vertex vert;
	vert.id = id;
	vert.vsize = vsize;
	vert.center = center;
	cell_map->insert(pair<unsigned int, Vertex>(id, vert));

	return true;
}

Frame* TraceGroup::AddFrame(int time)
{
	Frame frame;
	frame.id = (unsigned int)time;
	FrameIter frame_iter;
	m_frame_list.insert(pair<unsigned int, Frame>((unsigned int)time, frame));
	frame_iter = m_frame_list.find((unsigned int)time);
	if (frame_iter != m_frame_list.end())
		return &(frame_iter->second);
	else
		return 0;
}
unsigned char TraceGroup::ReadTag(ifstream &ifs)
{
	if (ifs.bad())
		return 0;

	unsigned char tag;
	ifs.read((char*)(&tag), sizeof(tag));
	if (ifs)
		return tag;
	else
		return 0;
}

int TraceGroup::Load(wxString &filename)
{
	int result = 1;
	m_data_path = filename;

	std::string str = m_data_path.ToStdString();
	std::ifstream ifs(str.c_str(),ios::in|ios::binary);
	if (ifs.bad())
		return 0;

	char cheader[17];
	ifs.read(cheader, 16);
	cheader[16] = 0;
	string header = cheader;
	if (header != "FluoRender links")
		return 0;

	unsigned int num;
	ifs.read((char*)(&num), sizeof(num));
	//clear existing
	m_frame_list.clear();
	//read new
	for (unsigned int i=0; i<num; ++i)
	{
		//tag: frame
		if (ReadTag(ifs) != TAG_FRAM)
		{
			result = 0;
			break;
		}

		Frame frame;
		if (frame.Read(ifs))
			m_frame_list.insert(pair<unsigned int, Frame>(frame.id,frame));
	}

	ifs.close();

	return result;
}
void TraceGroup::WriteTag(ofstream&ofs, unsigned char tag)
{
	if (ofs.bad())
		return;

	ofs.write(reinterpret_cast<const char*>(&tag), sizeof(unsigned char));
}

int TraceGroup::Save(wxString &filename)
{
	int result = 1;
	m_data_path = filename;

	std::ofstream ofs(ws2s(m_data_path.ToStdWstring()).c_str(), ios::out|ios::binary);
	if (ofs.bad())
		return 0;

	//header
	string header = "FluoRender links";
	ofs.write(header.c_str(), header.size());

	//number of frames
	unsigned int num = (unsigned int)m_frame_list.size();
	ofs.write(reinterpret_cast<const char*>(&num), sizeof(num));

	//write each frame
	for (FrameIter iter = m_frame_list.begin();
		iter != m_frame_list.end(); ++iter)
	{
		WriteTag(ofs, TAG_FRAM);

		//write frame
		iter->second.Write(ofs);
	}

	ofs.close();
	return result;
}
void TraceGroup::Draw()
{
	if (m_ghost_num <= 0)
		return;

	glPushAttrib( GL_TEXTURE_BIT | GL_DEPTH_TEST |
		GL_LIGHTING | GL_COLOR_BUFFER_BIT | GL_LINE_BIT);
	double width = 1;
	switch (m_font)
	{
	case BITMAP_FONT_TYPE_HELVETICA_10:
	case BITMAP_FONT_TYPE_HELVETICA_12:
	case BITMAP_FONT_TYPE_TIMES_ROMAN_10:
		width = 1;
		break;
	case BITMAP_FONT_TYPE_HELVETICA_18:
		width = 2;
		break;
	case BITMAP_FONT_TYPE_TIMES_ROMAN_24:
		width = 3;
		break;
	}
	glLineWidth(GLfloat(width));
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	vector<CellMap*> ghosts;
	unsigned int gstart, gend;
	unsigned int cur_ghost = 0;
	gstart = m_cur_time<=m_ghost_num?0:m_cur_time-m_ghost_num;
	gend = m_cur_time+m_ghost_num;
	FrameIter frame_iter;
	for (unsigned int i=gstart; i<=gend; ++i)
	{
		frame_iter = m_frame_list.find(i);
		if (frame_iter != m_frame_list.end())
		{
			ghosts.push_back(&(frame_iter->second.cell_map));
			if (frame_iter->second.id == m_cur_time)
				cur_ghost = ghosts.size()-1;
		}
	}

	if (ghosts.size() > 0)
	{
		//
		IDMapIter id_map_iter;
		unsigned int id, id2;
		CellMap *cell_map1 = 0;
		CellMap *cell_map2 = 0;
		CellMapIter cell_map_iter1, cell_map_iter2;
		Vertex *vertex1 = 0;
		Vertex *vertex2 = 0;
		IDMap id_map_temp1, id_map_temp2;
		Lbl lbl;
		glBegin(GL_LINES);

		id_map_temp1 = m_id_map;

		if (m_cur_time >= m_prv_time)
		{
			//after
			for (int i=cur_ghost; i<int(cur_ghost+m_ghost_num); ++i)
			{
				//after
				if (i>=0 && i+1<ghosts.size())
				{
					cell_map1 = ghosts[i];
					cell_map2 = ghosts[i+1];

					for (id_map_iter = id_map_temp1.begin();
						id_map_iter != id_map_temp1.end(); ++id_map_iter)
					{
						id = id_map_iter->second.id;
						Color c(HSVColor(id%360, 1.0, 0.9));
						glColor3d(c.r(), c.g(), c.b());

						cell_map_iter1 = cell_map1->find(id);
						if (cell_map_iter1 != cell_map1->end())
						{
							vertex1 = &(cell_map_iter1->second);
							for (unsigned int i=0; i<vertex1->out_ids.size(); ++i)
							{
								id2 = vertex1->out_ids[i];
								cell_map_iter2 = cell_map2->find(id2);
								if (cell_map_iter2 != cell_map2->end())
								{
									vertex2 = &(cell_map_iter2->second);
									lbl.id = id2;
									lbl.size = vertex2->vsize;
									lbl.center = vertex2->center;
									id_map_temp2.insert(pair<unsigned int, Lbl>(id2, lbl));
									glVertex3d(vertex1->center.x(),
										vertex1->center.y(),
										vertex1->center.z());
									glVertex3d(vertex2->center.x(),
										vertex2->center.y(),
										vertex2->center.z());
								}
							}
						}
					}
				}
				id_map_temp1 = id_map_temp2;
				id_map_temp2.clear();
			}
		}
		else if (m_cur_time < m_prv_time)
		{
			//before
			for (int i=cur_ghost; i>int(cur_ghost-m_ghost_num); --i)
			{
				//before
				if (i-1>=0 && i<ghosts.size())
				{
					cell_map1 = ghosts[i];
					cell_map2 = ghosts[i-1];

					for (id_map_iter = id_map_temp1.begin();
						id_map_iter != id_map_temp1.end(); ++id_map_iter)
					{
						id = id_map_iter->second.id;
						Color c(HSVColor(id%360, 1.0, 0.9));
						glColor3d(c.r(), c.g(), c.b());

						cell_map_iter1 = cell_map1->find(id);
						if (cell_map_iter1 != cell_map1->end())
						{
							vertex1 = &(cell_map_iter1->second);
							for (unsigned int i=0; i<vertex1->in_ids.size(); ++i)
							{
								id2 = vertex1->in_ids[i];
								cell_map_iter2 = cell_map2->find(id2);
								if (cell_map_iter2 != cell_map2->end())
								{
									vertex2 = &(cell_map_iter2->second);
									lbl.id = id2;
									lbl.size = vertex2->vsize;
									lbl.center = vertex2->center;
									id_map_temp2.insert(pair<unsigned int, Lbl>(id2, lbl));
									glVertex3d(vertex1->center.x(),
										vertex1->center.y(),
										vertex1->center.z());
									glVertex3d(vertex2->center.x(),
										vertex2->center.y(),
										vertex2->center.z());
								}
							}
						}
					}
				}
				id_map_temp1 = id_map_temp2;
				id_map_temp2.clear();
			}
		}

		glEnd();
	}

	glPopAttrib();
}
//pattern search
bool TraceGroup::FindPattern(int type, unsigned int id, int time)
{
	FrameIter frame_iter = m_frame_list.find(time);
	if (frame_iter == m_frame_list.end())
		return false;
	
	Patterns patterns;
	patterns.div = 0;
	patterns.conv = 0;

	CellMap* cell_map;
	CellMapIter cell_map_iter;
	set<unsigned int> id_list1, id_list2;
	set<unsigned int>::iterator id_list_iter;
	id_list1.insert(id);
	Vertex* vertex;
	unsigned int i;
	while (frame_iter != m_frame_list.end())
	{
		cell_map = &(frame_iter->second.cell_map);

		for (id_list_iter = id_list1.begin();
			id_list_iter != id_list1.end();
			++id_list_iter)
		{
			cell_map_iter = cell_map->find(*id_list_iter);
			if (cell_map_iter != cell_map->end())
			{
				vertex = &(cell_map_iter->second);
				for (i=0; i<vertex->out_ids.size(); ++i)
					id_list2.insert(vertex->out_ids[i]);
			}
		}

		//determine pattern
		switch (type)
		{
		case 1:
			if (id_list2.size() > 2)
				return false;
			if (id_list1.size() == 1 &&
				id_list2.size() == 2)
				patterns.div = patterns.div?0:1;
			if (id_list1.size() == 2 &&
				id_list2.size() == 1)
				if (patterns.div)
					return true;
			break;
		case 2:
			if (id_list1.size() == 1 &&
				id_list2.size() > 1)
				return true;
			break;
		}

		//copy list2 to list1
		id_list1.clear();
		for (id_list_iter = id_list2.begin();
			id_list_iter != id_list2.end();
			++id_list_iter)
			id_list1.insert(*id_list_iter);
		id_list2.clear();
		
		//next frame
		++frame_iter;
	}
	return false;
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
m_vol_lcm(0.0),
m_vol_hcm(1.0),
m_vol_eap(true),
m_vol_esh(true),
m_vol_inv(false),
m_vol_mip(false),
m_vol_nrd(false),
m_vol_shw(false),
m_vol_swi(0.0),
m_vol_test_wiref(false),
m_use_defaults(true),
m_override_vox(true)
{
#ifdef _DARWIN
    
    wxString dft = wxString(getenv("HOME")) + "/Fluorender.settings/default_volume_settings.dft";
    std::ifstream tmp(dft);
    if (!tmp.good())
        dft = "FluoRender.app/Contents/Resources/default_volume_settings.dft";
    else
        tmp.close();
#else
    wxString dft = "default_volume_settings.dft";
#endif
	wxFileInputStream is(dft);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	double val;
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
	if (fconfig.Read("colormap_low", &val))
		m_vol_lcm = val;
	if (fconfig.Read("colormap_hi", &val))
		m_vol_hcm = val;

	bool bval;
	if (fconfig.Read("enable_alpha", &bval))
		m_vol_eap = bval;
	if (fconfig.Read("enable_shading", &bval))
		m_vol_esh = bval;
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
	m_load_mask = false;
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
			vd->SetSpacings(m_vol_xsp, m_vol_ysp, m_vol_zsp);
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
#ifdef _WIN32
    wxChar slash = '\\';
#else
    wxChar slash = '/';
#endif
	int sep = path.Find(slash, true);
	if (sep != wxNOT_FOUND)
		m_prj_path = path.Left(sep);
}

wxString DataManager::SearchProjectPath(wxString &filename)
{
	int i;

	wxString pathname = filename;

	if (m_prj_path == "")
		return "";
	wxString search_str;
	for (i=pathname.Length()-1; i>=0; i--)
	{
		search_str.Prepend(pathname[i]);
		if (pathname[i]=='\\' || pathname[i]=='/')
		{
			wxString name_temp = m_prj_path + search_str;
			if (wxFileExists(name_temp))
				return name_temp;
		}
	}
	return "";
}

int DataManager::LoadVolumeData(wxString &filename, int type, int ch_num, int t_num)
{
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
			reader->Preprocess();
	}
	else
	{
		//RGB tiff
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

		m_reader_list.push_back(reader);
		wstring str_w = pathname.ToStdWstring();
		reader->SetFile(str_w);
		reader->SetSliceSeq(m_sliceSequence);
		str_w = m_timeId.ToStdWstring();
		reader->SetTimeId(str_w);
		reader->Preprocess();
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
		vd->SetSkipBrick(m_skip_brick);
		Nrrd* data = reader->Convert(t_num>=0?t_num:reader->GetCurTime(), i, true);
		if (!data)
			continue;
		wxString name = wxString(reader->GetDataName());
		if (chan > 1)
			name += wxString::Format("_%d", i+1);
		bool valid_spc = reader->IsSpcInfoValid();
		if (vd && vd->Load(data, name, pathname))
		{
			if (m_load_mask)
			{
				//mask
				MSKReader msk_reader;
				std::wstring str = reader->GetPathName();
                msk_reader.SetFile(str);
                Nrrd* mask = msk_reader.Convert(t_num>=0?t_num:reader->GetCurTime(), i, true);
				if (mask)
					vd->LoadMask(mask);
				//label mask
				LBLReader lbl_reader;
                str = reader->GetPathName();
                lbl_reader.SetFile(str);

				Nrrd* label = lbl_reader.Convert(t_num>=0?t_num:reader->GetCurTime(), i, true);
				if (label)
					vd->LoadLabel(label);
			}
			vd->SetSpacings(reader->GetXSpc(),
				reader->GetYSpc(),
				reader->GetZSpc());
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
		vd->SetReader(reader);
		vd->SetCompression(m_compression);
		AddVolumeData(vd);

		SetVolumeDefault(vd);

		//get excitation wavelength
		double wavelength = reader->GetExcitationWavelength(i);
        if (wavelength > 0.0) {
            FLIVR::Color col = GetWavelengthColor(wavelength);
            vd->SetColor(col);
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
	if (data)
	{
		m_vd_list.erase(m_vd_list.begin()+index);
		delete data;
		data = 0;
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
			m_vd_list[0]->GetSpacings(spcx, spcy, spcz);
			vd->SetSpacings(spcx, spcy, spcz);
			vd->SetSpcFromFile(true);
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

void DataManager::SetWavelengthColor(int c1, int c2, int c3, int c4)
{
	switch (c1)
	{
	case 1:
		m_vol_wav[0] = Color(1.0, 0.0, 0.0);
		break;
	case 2:
		m_vol_wav[0] = Color(0.0, 1.0, 0.0);
		break;
	case 3:
		m_vol_wav[0] = Color(0.0, 0.0, 1.0);
		break;
	case 4:
		m_vol_wav[0] = Color(1.0, 0.0, 1.0);
		break;
	case 5:
		m_vol_wav[0] = Color(1.0, 1.0, 1.0);
		break;
	default:
		m_vol_wav[0] = Color(1.0, 1.0, 1.0);
		break;
	}
	switch (c2)
	{
	case 1:
		m_vol_wav[1] = Color(1.0, 0.0, 0.0);
		break;
	case 2:
		m_vol_wav[1] = Color(0.0, 1.0, 0.0);
		break;
	case 3:
		m_vol_wav[1] = Color(0.0, 0.0, 1.0);
		break;
	case 4:
		m_vol_wav[1] = Color(1.0, 0.0, 1.0);
		break;
	case 5:
		m_vol_wav[1] = Color(1.0, 1.0, 1.0);
		break;
	default:
		m_vol_wav[1] = Color(1.0, 1.0, 1.0);
		break;
	}
	switch (c3)
	{
	case 1:
		m_vol_wav[2] = Color(1.0, 0.0, 0.0);
		break;
	case 2:
		m_vol_wav[2] = Color(0.0, 1.0, 0.0);
		break;
	case 3:
		m_vol_wav[2] = Color(0.0, 0.0, 1.0);
		break;
	case 4:
		m_vol_wav[2] = Color(1.0, 0.0, 1.0);
		break;
	case 5:
		m_vol_wav[2] = Color(1.0, 1.0, 1.0);
		break;
	default:
		m_vol_wav[2] = Color(1.0, 1.0, 1.0);
		break;
	}
	switch (c4)
	{
	case 1:
		m_vol_wav[3] = Color(1.0, 0.0, 0.0);
		break;
	case 2:
		m_vol_wav[3] = Color(0.0, 1.0, 0.0);
		break;
	case 3:
		m_vol_wav[3] = Color(0.0, 0.0, 1.0);
		break;
	case 4:
		m_vol_wav[3] = Color(1.0, 0.0, 1.0);
		break;
	case 5:
		m_vol_wav[3] = Color(1.0, 1.0, 1.0);
		break;
	default:
		m_vol_wav[0] = Color(1.0, 1.0, 1.0);
		break;
	}
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
