#include "VolumeSelector.h"
#include "VRenderFrame.h"
#include "utility.h"
#include <wx/wx.h>

VolumeSelector::VolumeSelector() :
   m_vd(0),
   m_2d_mask(0),
   m_2d_weight1(0),
   m_2d_weight2(0),
   m_iter_num(20),
   m_mode(0),
   m_use2d(true),
   m_ini_thresh(0.0),
   m_gm_falloff(0.0),
   m_scl_falloff(0.0),
   m_scl_translate(0.0),
   m_select_multi(0),
   m_use_brush_size2(false),
   m_edge_detect(false),
   m_hidden_removal(false),
   m_ortho(true),
   m_w2d(0.0),
   m_iter_label(1),
   m_label_thresh(0.0),
   m_label_falloff(1.0),
   m_min_voxels(0.0),
   m_max_voxels(0.0),
   m_annotations(0),
   m_prog_diag(0),
   m_progress(0),
   m_total_pr(0),
   m_ca_comps(0),
   m_ca_volume(0),
   m_randv(113),
   m_ps(false),
   m_ps_size(0.0),
m_size_map(false)
{
}

VolumeSelector::~VolumeSelector()
{
}

void VolumeSelector::SetVolume(VolumeData *vd)
{
   m_vd = vd;
}

VolumeData* VolumeSelector::GetVolume()
{
   return m_vd;
}

void VolumeSelector::Set2DMask(GLuint mask)
{
   m_2d_mask = mask;
}

void VolumeSelector::Set2DWeight(GLuint weight1, GLuint weight2)
{
   m_2d_weight1 = weight1;
   m_2d_weight2 = weight2;
}

void VolumeSelector::SetProjection(double *mvmat, double *prjmat)
{
   memcpy(m_mvmat, mvmat, 16*sizeof(double));
   memcpy(m_prjmat, prjmat, 16*sizeof(double));
}

void VolumeSelector::Select(double radius)
{
   if (!m_vd)
      return;
   //if (!glIsTexture(m_2d_mask))
   //  return;

   //insert the mask volume into m_vd
   m_vd->AddEmptyMask();
   m_vd->Set2dMask(m_2d_mask);
   if (m_use2d && glIsTexture(m_2d_weight1) && glIsTexture(m_2d_weight2))
      m_vd->Set2DWeight(m_2d_weight1, m_2d_weight2);
   else
      m_vd->Set2DWeight(0, 0);

   //segment the volume with 2d mask
   //result in 3d mask
   //clear if the select mode
   double ini_thresh, gm_falloff, scl_falloff;
   if (m_use_brush_size2)
   {
      if (m_ini_thresh > 0.0)
         ini_thresh = m_ini_thresh;
      else
         ini_thresh = sqrt(m_scl_translate);
      if (m_scl_falloff > 0.0)
         scl_falloff = m_scl_falloff;
      else
         scl_falloff = 0.01;
   }
   else
   {
      ini_thresh = m_scl_translate;
      scl_falloff = 0.0;
   }
   if (m_edge_detect)
   {
      if (m_gm_falloff > 0.0)
         gm_falloff = m_gm_falloff;
      else
         gm_falloff = 0.01;
   }
   else
      gm_falloff = 0.0;

   //there is some unknown problem of clearing the mask
   if (m_mode == 1)
   {
      m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0);
      m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0);
      m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0);
   }
   else if (m_mode == 6)
      m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0);

   //initialization
   int hr_mode = m_hidden_removal?(m_ortho?1:2):0;
   m_vd->DrawMask(0, m_mode, hr_mode, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0);

   //grow the selection when paint mode is select, append, erase, or invert
   if (m_mode==1 ||
         m_mode==2 ||
         m_mode==3 ||
         m_mode==4)
   {
      //loop for growing
      int iter = m_iter_num*(radius/200.0>1.0?radius/200.0:1.0);
      for (int i=0; i<iter; i++)
         m_vd->DrawMask(1, m_mode, 0, ini_thresh, gm_falloff, scl_falloff, m_scl_translate, m_w2d, 0.0);
   }

   if (m_mode == 6)
      m_vd->SetUseMaskThreshold(false);
}

//mode: 0-normal; 1-posterized; 2-noraml,copy; 3-poster, copy
void VolumeSelector::Label(int mode)
{
   if (!m_vd)
      return;

   int label_mode = 0;
   if (mode==2||mode==3)
      label_mode = 2;
   //insert the label volume to m_vd
   m_vd->AddEmptyLabel(label_mode);

   //apply ids to the label volume
	m_vd->DrawLabel(0, mode, m_label_thresh, m_label_falloff);

   //filter the label volume by maximum intensity filtering
   for (int i=0; i<m_iter_label; i++)
   {
		m_vd->DrawLabel(1, mode, m_label_thresh, m_label_falloff);
      if (m_prog_diag)
      {
         m_progress++;
         m_prog_diag->Update(95*(m_progress+1)/m_total_pr);
      }
   }
}

int VolumeSelector::CompAnalysis(double min_voxels, double max_voxels, double thresh, double falloff, bool select, bool gen_ann)
{
   int return_val = 0;
   m_label_thresh = thresh;
	m_label_falloff = falloff;
   if (!m_vd)
      return return_val;

   bool use_sel = false;
   if (select && m_vd->GetTexture() && m_vd->GetTexture()->nmask()!=-1)
      use_sel = true;

   m_prog_diag = new wxProgressDialog(
         "FluoRender: Component Analysis...",
         "Analyzing... Please wait.",
         100, 0,
         wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);
   m_progress = 0;

   if (use_sel)
   {
      //calculate on selection only
      int nx, ny, nz;
      m_vd->GetResolution(nx, ny, nz);
      m_iter_label = Max(nx, Max(ny, nz));
      m_total_pr = m_iter_label+nx*2;
   }
   else
   {
      //calculate on the whole volume
      int nx, ny, nz;
      m_vd->GetResolution(nx, ny, nz);
      m_iter_label = Max(nx, Max(ny, nz));
      m_total_pr = m_iter_label+nx*2;
      //first, grow in the whole volume
      m_vd->AddEmptyMask();
      if (m_use2d && glIsTexture(m_2d_weight1) && glIsTexture(m_2d_weight2))
         m_vd->Set2DWeight(m_2d_weight1, m_2d_weight2);
      else
         m_vd->Set2DWeight(0, 0);

      m_vd->DrawMask(0, 5, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
      //next do the same as when it's selected by brush
   }
      Label(0);
      m_vd->GetVR()->return_label();
      return_val = CompIslandCount(min_voxels, max_voxels);


   if (gen_ann)
      GenerateAnnotations(use_sel);
   else
      m_annotations = 0;

	if (m_size_map)
		SetLabelBySize();

	if (m_prog_diag)
	{
		m_prog_diag->Update(100);
		delete m_prog_diag;
		m_prog_diag = 0;
	}

   return return_val;
}
int VolumeSelector::SetLabelBySize()
{
	int return_val = 0;
	if (!m_vd)
		return 0;
	//get label data
	Texture* tex = m_vd->GetTexture();
	if (!tex)
		return 0;
	Nrrd* nrrd_label = tex->get_nrrd(tex->nlabel());
	if (!nrrd_label)
		return 0;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return 0;

	//determine range first
	unsigned int min_size = 0;
	unsigned int max_size = 0;
	unsigned int counter;
	boost::unordered_map<unsigned int, Component>::iterator comp_iter;
	for (comp_iter=m_comps.begin();
		comp_iter!=m_comps.end();
		++comp_iter)
	{
		counter = comp_iter->second.counter;
		if (counter<m_min_voxels ||
			(m_max_voxels<0.0?false:
			(counter>m_max_voxels)))
			continue;
		if (comp_iter == m_comps.begin())
		{
			min_size = counter;
			max_size = counter;
		}
		else
		{
			min_size = counter<min_size?counter:min_size;
			max_size = counter>max_size?counter:max_size;
		}
	}

	//parse label data and change values
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);

	int i, j, k;
	int index;
	unsigned int id;
	for (i=0; i<nx; ++i)
	for (j=0; j<ny; ++j)
	for (k=0; k<nz; ++k)
	{
		index = nx*ny*k + nx*j + i;
		id = data_label[index];
		if (id > 0)
		{
			comp_iter = m_comps.find(id);
			if (comp_iter != m_comps.end())
			{
				counter = comp_iter->second.counter;
				if (counter >= min_size &&
					counter <= max_size)
				{
					//calculate color
					if (max_size > min_size)
						data_label[index] = 
							(unsigned int)(240.0-
							(double)(counter-min_size)/
							(double)(max_size-min_size)*
							239.0);
					else
						data_label[index] = 1;
					continue;
				}
			}

			data_label[index] = 0;
		}
	}

	return return_val;
}
int VolumeSelector::CompIslandCount(double min_voxels, double max_voxels)
{
   m_min_voxels = min_voxels;
   m_max_voxels = max_voxels;
   m_ca_comps = 0;
   m_ca_volume = 0;
   if (!m_vd)
      return 0;
   Texture* tex = m_vd->GetTexture();
   if (!tex)
      return 0;

   Nrrd* orig_nrrd = tex->get_nrrd(0);
   if (!orig_nrrd)
      return 0;
   void* orig_data = orig_nrrd->data;
   if (!orig_data)
      return 0;
   Nrrd* label_nrrd = tex->get_nrrd(tex->nlabel());
   if (!label_nrrd)
      return 0;
   unsigned int* label_data = (unsigned int*)(label_nrrd->data);
   if (!label_data)
      return 0;

   //resolution
   int nx, ny, nz;
   m_vd->GetResolution(nx, ny, nz);

   m_comps.clear();
   int i, j, k;
   //first pass: generate the component list
   for (i=0; i<nx; i++)
   {
      for (j=0; j<ny; j++)
         for (k=0; k<nz; k++)
         {
            int index = nx*ny*k + nx*j + i;
            unsigned int value = label_data[index];
            if (value > 0)
            {
               Vector pos = Vector(i, j, k);
               double intensity = 0.0;
               if (orig_nrrd->type == nrrdTypeUChar)
                  intensity = double(((unsigned char*)orig_data)[index]) / 255.0;
               else if (orig_nrrd->type == nrrdTypeUShort)
                  intensity = double(((unsigned short*)orig_data)[index]) / 65535.0;
               bool found = SearchComponentList(value, pos, intensity);
               if (!found)
               {
                  Component comp;
                  comp.counter = 1;
                  comp.id = value;
                  comp.acc_pos = pos;
                  comp.acc_int = intensity;
                  m_comps.insert(pair<unsigned int, Component>(value, comp));
               }
            }
         }
      if (m_prog_diag)
      {
         m_progress++;
         m_prog_diag->Update(95*(m_progress+1)/m_total_pr);
      }
   }

   boost::unordered_map <unsigned int, Component> :: const_iterator comp_iter;
   //second pass: combine components and remove islands
   //update mask
   Nrrd* mask_nrrd = m_vd->GetMask();
   if (!mask_nrrd)
      return 0;
   unsigned char* mask_data = (unsigned char*)(mask_nrrd->data);
   if (!mask_data)
      return 0;
   for (i=0; i<nx; i++)
   {
      for (j=0; j<ny; j++)
         for (k=0; k<nz; k++)
         {
            int index = nx*ny*k + nx*j + i;
            unsigned int label_value = label_data[index];
			unsigned char mask_value = mask_data[index];
            if (label_value>0)
            {
               comp_iter = m_comps.find(label_value);
               if (comp_iter != m_comps.end())
               {
                  if (comp_iter->second.counter<min_voxels ||
                        (max_voxels<0.0?false:(comp_iter->second.counter>max_voxels)))
                     mask_data[index] = 0;
               }
            }
            else
               mask_data[index] = 0;
         }
      if (m_prog_diag)
      {
         m_progress++;
         m_prog_diag->Update(95*(m_progress+1)/m_total_pr);
      }
   }
   m_vd->GetVR()->clear_tex_pool();

   //count
   for (comp_iter=m_comps.begin(); comp_iter!=m_comps.end(); comp_iter++)
   {
      if (comp_iter->second.counter>=min_voxels &&
            (max_voxels<0.0?true:(comp_iter->second.counter<=max_voxels)))
      {
         m_ca_comps++;
         m_ca_volume += comp_iter->second.counter;
      }
   }

   return m_ca_comps;
}

bool VolumeSelector::SearchComponentList(unsigned int cval, Vector &pos, double intensity)
{
   boost::unordered_map <unsigned int, Component> :: iterator comp_iter;
   comp_iter = m_comps.find(cval);
   if (comp_iter != m_comps.end())
   {
      comp_iter->second.counter++;
      comp_iter->second.acc_pos += pos;
      comp_iter->second.acc_int += intensity;
      return true;
   }
   else
      return false;

   return false;
}

void VolumeSelector::CompExportMultiChann(bool select)
{
   if (!m_vd ||
         !m_vd->GetTexture() ||
         (select&&m_vd->GetTexture()->nmask()==-1) ||
         m_vd->GetTexture()->nlabel()==-1)
      return;

   m_result_vols.clear();

   if (select)
      m_vd->GetVR()->return_mask();

   int i;
   //get all the data from original volume
   Texture* tex_mvd = m_vd->GetTexture();
   if (!tex_mvd) return;
   Nrrd* nrrd_mvd = tex_mvd->get_nrrd(0);
   if (!nrrd_mvd) return;
   Nrrd* nrrd_mvd_mask = tex_mvd->get_nrrd(tex_mvd->nmask());
   if (select && !nrrd_mvd_mask) return;
   Nrrd* nrrd_mvd_label = tex_mvd->get_nrrd(tex_mvd->nlabel());
   if (!nrrd_mvd_label) return;
   void* data_mvd = nrrd_mvd->data;
   unsigned char* data_mvd_mask = (unsigned char*)nrrd_mvd_mask->data;
   unsigned int* data_mvd_label = (unsigned int*)nrrd_mvd_label->data;
   if (!data_mvd || (select&&!data_mvd_mask) || !data_mvd_label) return;

   i = 1;
   boost::unordered_map <unsigned int, Component> :: const_iterator comp_iter;
   for (comp_iter=m_comps.begin(); comp_iter!=m_comps.end(); comp_iter++)
   {
      if (comp_iter->second.counter<m_min_voxels ||
            (m_max_voxels<0.0?false:(comp_iter->second.counter>m_max_voxels)))
         continue;

      //create a new volume
      int res_x, res_y, res_z;
      double spc_x, spc_y, spc_z;
      int bits = 8;
      m_vd->GetResolution(res_x, res_y, res_z);
      m_vd->GetSpacings(spc_x, spc_y, spc_z);
      double amb, diff, spec, shine;
      m_vd->GetMaterial(amb, diff, spec, shine);

      VolumeData* vd = new VolumeData();
      vd->AddEmptyData(bits,
            res_x, res_y, res_z,
            spc_x, spc_y, spc_z);
      vd->SetSpcFromFile(true);
      vd->SetName(m_vd->GetName() +
            wxString::Format("_COMP%d_SIZE%d", i++, comp_iter->second.counter));

      //populate the volume
      //the actual data
      Texture* tex_vd = vd->GetTexture();
      if (!tex_vd) continue;
      Nrrd* nrrd_vd = tex_vd->get_nrrd(0);
      if (!nrrd_vd) continue;
      unsigned char* data_vd = (unsigned char*)nrrd_vd->data;
      if (!data_vd) continue;

      int ii, jj, kk;
      for (ii=0; ii<res_x; ii++)
         for (jj=0; jj<res_y; jj++)
            for (kk=0; kk<res_z; kk++)
            {
               int index = res_x*res_y*kk + res_x*jj + ii;
               unsigned int value_label = data_mvd_label[index];
               if (value_label > 0 && value_label==comp_iter->second.id)
               {
                  unsigned char value = 0;
                  if (nrrd_mvd->type == nrrdTypeUChar)
                  {
                     if (select)
                        value = (unsigned char)((double)(((unsigned char*)data_mvd)[index]) *
                              double(data_mvd_mask[index]) / 255.0);
                     else
                        value = ((unsigned char*)data_mvd)[index];
                  }
                  else if (nrrd_mvd->type == nrrdTypeUShort)
                  {
                     if (select)
                        value = (unsigned char)((double)(((unsigned short*)data_mvd)[index]) *
                              m_vd->GetScalarScale() * double(data_mvd_mask[index]) / 65535.0);
                     else
                        value = (unsigned char)((double)(((unsigned short*)data_mvd)[index]) *
                              m_vd->GetScalarScale() / 255.0);
                  }
                  data_vd[index] = value;
               }
            }
      int randv = 0;
      while (randv < 100) randv = rand();
      unsigned int rev_value_label = bit_reverse(comp_iter->second.id);
      double hue = double(rev_value_label % randv) / double(randv) * 360.0;
      Color color(HSVColor(hue, 1.0, 1.0));
      vd->SetColor(color);

      vd->SetEnableAlpha(m_vd->GetEnableAlpha());
      vd->SetShading(m_vd->GetShading());
      vd->SetShadow(false);
      //other settings
      vd->Set3DGamma(m_vd->Get3DGamma());
      vd->SetBoundary(m_vd->GetBoundary());
      vd->SetOffset(m_vd->GetOffset());
      vd->SetLeftThresh(m_vd->GetLeftThresh());
      vd->SetRightThresh(m_vd->GetRightThresh());
      vd->SetAlpha(m_vd->GetAlpha());
      vd->SetSampleRate(m_vd->GetSampleRate());
      vd->SetMaterial(amb, diff, spec, shine);

      m_result_vols.push_back(vd);
   }
}

void VolumeSelector::CompExportRandomColor(VolumeData* vd_r,
      VolumeData* vd_g, VolumeData* vd_b, bool select, bool hide)
{
   if (!m_vd ||
         !m_vd->GetTexture() ||
         (select&&m_vd->GetTexture()->nmask()==-1) ||
         m_vd->GetTexture()->nlabel()==-1)
      return;

   if (select)
      m_vd->GetVR()->return_mask();

   m_result_vols.clear();

   //get all the data from original volume
   Texture* tex_mvd = m_vd->GetTexture();
   if (!tex_mvd) return;
   Nrrd* nrrd_mvd = tex_mvd->get_nrrd(0);
   if (!nrrd_mvd) return;
   Nrrd* nrrd_mvd_mask = tex_mvd->get_nrrd(tex_mvd->nmask());
   if (select && !nrrd_mvd_mask) return;
   Nrrd* nrrd_mvd_label = tex_mvd->get_nrrd(tex_mvd->nlabel());
   if (!nrrd_mvd_label) return;
   void* data_mvd = nrrd_mvd->data;
   unsigned char* data_mvd_mask = (unsigned char*)nrrd_mvd_mask->data;
   unsigned int* data_mvd_label = (unsigned int*)nrrd_mvd_label->data;
   if (!data_mvd || (select&&!data_mvd_mask) || !data_mvd_label) return;

   //create new volumes
   int res_x, res_y, res_z;
   double spc_x, spc_y, spc_z;
   int bits = 8;
   m_vd->GetResolution(res_x, res_y, res_z);
   m_vd->GetSpacings(spc_x, spc_y, spc_z);
   bool push_new = true;
   //red volume
   if (!vd_r)
      vd_r = new VolumeData();
   else
      push_new = false;
   vd_r->AddEmptyData(bits,
         res_x, res_y, res_z,
         spc_x, spc_y, spc_z);
   vd_r->SetSpcFromFile(true);
   vd_r->SetName(m_vd->GetName() +
         wxString::Format("_COMP1"));
   //green volume
   if (!vd_g)
      vd_g = new VolumeData();
   vd_g->AddEmptyData(bits,
         res_x, res_y, res_z,
         spc_x, spc_y, spc_z);
   vd_g->SetSpcFromFile(true);
   vd_g->SetName(m_vd->GetName() +
         wxString::Format("_COMP2"));
   //blue volume
   if (!vd_b)
      vd_b = new VolumeData();
   vd_b->AddEmptyData(bits,
         res_x, res_y, res_z,
         spc_x, spc_y, spc_z);
   vd_b->SetSpcFromFile(true);
   vd_b->SetName(m_vd->GetName() +
         wxString::Format("_COMP3"));

   //get new data
   //red volume
   Texture* tex_vd_r = vd_r->GetTexture();
   if (!tex_vd_r) return;
   Nrrd* nrrd_vd_r = tex_vd_r->get_nrrd(0);
   if (!nrrd_vd_r) return;
   unsigned char* data_vd_r = (unsigned char*)nrrd_vd_r->data;
   if (!data_vd_r) return;
   //green volume
   Texture* tex_vd_g = vd_g->GetTexture();
   if (!tex_vd_g) return;
   Nrrd* nrrd_vd_g = tex_vd_g->get_nrrd(0);
   if (!nrrd_vd_g) return;
   unsigned char* data_vd_g = (unsigned char*)nrrd_vd_g->data;
   if (!data_vd_g) return;
   //blue volume
   Texture* tex_vd_b = vd_b->GetTexture();
   if (!tex_vd_b) return;
   Nrrd* nrrd_vd_b = tex_vd_b->get_nrrd(0);
   if (!nrrd_vd_b) return;
   unsigned char* data_vd_b = (unsigned char*)nrrd_vd_b->data;
   if (!data_vd_b) return;

   if (hide)
      m_randv = int((double)rand()/(RAND_MAX)*900+100);
   //populate the data
   int ii, jj, kk;
   for (ii=0; ii<res_x; ii++)
      for (jj=0; jj<res_y; jj++)
         for (kk=0; kk<res_z; kk++)
         {
            int index = res_x*res_y*kk + res_x*jj + ii;
            unsigned int value_label = data_mvd_label[index];
            if (value_label > 0)
            {
               //intensity value
               double value = 0.0;
               if (nrrd_mvd->type == nrrdTypeUChar)
               {
                  if (select)
                     value = double(((unsigned char*)data_mvd)[index]) *
                        double(data_mvd_mask[index]) / 65025.0;
                  else
                     value = double(((unsigned char*)data_mvd)[index]) / 255.0;
               }
               else if (nrrd_mvd->type == nrrdTypeUShort)
               {
                  if (select)
                     value = double(((unsigned short*)data_mvd)[index]) *
                        m_vd->GetScalarScale() *
                        double(data_mvd_mask[index]) / 16581375.0;
                  else
                     value = double(((unsigned short*)data_mvd)[index]) *
                        m_vd->GetScalarScale() / 65535.0;
               }
               //unsigned int rev_value_label = bit_reverse(value_label);
               unsigned int rev_value_label = value_label;
               double hue = double(rev_value_label % m_randv) / double(m_randv) * 360.0;
               Color color(HSVColor(hue, 1.0, 1.0));
               //color
               value = value>1.0?1.0:value;
               data_vd_r[index] = (unsigned char)(color.r()*255.0*value);
               data_vd_g[index] = (unsigned char)(color.g()*255.0*value);
               data_vd_b[index] = (unsigned char)(color.b()*255.0*value);
            }
         }

   FLIVR::Color red    = Color(1.0,0.0,0.0);
   FLIVR::Color green  = Color(0.0,1.0,0.0);
   FLIVR::Color blue   = Color(0.0,0.0,1.0);
   vd_r->SetColor(red);
   vd_g->SetColor(green);
   vd_b->SetColor(blue);
   bool bval = m_vd->GetEnableAlpha();
   vd_r->SetEnableAlpha(bval);
   vd_g->SetEnableAlpha(bval);
   vd_b->SetEnableAlpha(bval);
   bval = m_vd->GetShading();
   vd_r->SetShading(bval);
   vd_g->SetShading(bval);
   vd_b->SetShading(bval);
   vd_r->SetShadow(false);
   vd_g->SetShadow(false);
   vd_b->SetShadow(false);
   //other settings
   double amb, diff, spec, shine;
   m_vd->GetMaterial(amb, diff, spec, shine);
   vd_r->Set3DGamma(m_vd->Get3DGamma());
   vd_r->SetBoundary(m_vd->GetBoundary());
   vd_r->SetOffset(m_vd->GetOffset());
   vd_r->SetLeftThresh(m_vd->GetLeftThresh());
   vd_r->SetRightThresh(m_vd->GetRightThresh());
   vd_r->SetAlpha(m_vd->GetAlpha());
   vd_r->SetSampleRate(m_vd->GetSampleRate());
   vd_r->SetMaterial(amb, diff, spec, shine);
   vd_g->Set3DGamma(m_vd->Get3DGamma());
   vd_g->SetBoundary(m_vd->GetBoundary());
   vd_g->SetOffset(m_vd->GetOffset());
   vd_g->SetLeftThresh(m_vd->GetLeftThresh());
   vd_g->SetRightThresh(m_vd->GetRightThresh());
   vd_g->SetAlpha(m_vd->GetAlpha());
   vd_g->SetSampleRate(m_vd->GetSampleRate());
   vd_g->SetMaterial(amb, diff, spec, shine);
   vd_b->Set3DGamma(m_vd->Get3DGamma());
   vd_b->SetBoundary(m_vd->GetBoundary());
   vd_b->SetOffset(m_vd->GetOffset());
   vd_b->SetLeftThresh(m_vd->GetLeftThresh());
   vd_b->SetRightThresh(m_vd->GetRightThresh());
   vd_b->SetAlpha(m_vd->GetAlpha());
   vd_b->SetSampleRate(m_vd->GetSampleRate());
   vd_b->SetMaterial(amb, diff, spec, shine);

   if (push_new)
   {
      m_result_vols.push_back(vd_r);
      m_result_vols.push_back(vd_g);
      m_result_vols.push_back(vd_b);
   }

   //turn off m_vd
   if (hide)
      m_vd->SetDisp(false);
}

vector<VolumeData*>* VolumeSelector::GetResultVols()
{
   return &m_result_vols;
}

//process current selection
int VolumeSelector::ProcessSel(double thresh)
{
   m_ps = false;

   if (!m_vd ||
         !m_vd->GetTexture() ||
         m_vd->GetTexture()->nmask()==-1)
      return 0;

   m_vd->GetVR()->return_mask();

   //get all the data from original volume
   Texture* tex_mvd = m_vd->GetTexture();
   if (!tex_mvd) return 0;
   Nrrd* nrrd_mvd = tex_mvd->get_nrrd(0);
   if (!nrrd_mvd) return 0;
   Nrrd* nrrd_mvd_mask = tex_mvd->get_nrrd(tex_mvd->nmask());
   if (!nrrd_mvd_mask) return 0;
   void* data_mvd = nrrd_mvd->data;
   unsigned char* data_mvd_mask = (unsigned char*)nrrd_mvd_mask->data;
   if (!data_mvd || (!data_mvd_mask)) return 0;

   //find center
   int res_x, res_y, res_z;
   double spc_x, spc_y, spc_z;
   m_vd->GetResolution(res_x, res_y, res_z);
   m_vd->GetSpacings(spc_x, spc_y, spc_z);

   m_ps_size = 0.0;
   double nw = 0.0;
   double w;
   Point sump(0.0, 0.0, 0.0);
   double value;
   int ii, jj, kk;
   int index;
   for (ii=0; ii<res_x; ii++)
      for (jj=0; jj<res_y; jj++)
         for (kk=0; kk<res_z; kk++)
         {
            index = res_x*res_y*kk + res_x*jj + ii;
            if (nrrd_mvd->type == nrrdTypeUChar)
               value = double(((unsigned char*)data_mvd)[index]) *
                  double(data_mvd_mask[index]) / 65025.0;
            else if (nrrd_mvd->type == nrrdTypeUShort)
               value = double(((unsigned short*)data_mvd)[index]) *
                  m_vd->GetScalarScale() *
                  double(data_mvd_mask[index]) / 16581375.0;
            if (value > thresh)
            {
               w = value>0.5?1.0:-16.0*value*value*value + 12.0*value*value;
               sump += Point(ii, jj, kk)*w;
               nw += w;
               m_ps_size += 1.0;
            }
         }

   //clear data_mvd_mask
	size_t set_num = res_x*res_y*res_z;
	memset(data_mvd_mask, 0, set_num);

   if (nw > 0.0)
   {
      m_ps_center = Point(sump.x()*spc_x, sump.y()*spc_y, sump.z()*spc_z) / nw +
         Vector(0.5*spc_x, 0.5*spc_y, 0.5*spc_z);
      m_ps_size *= spc_x*spc_y*spc_z;
      m_ps = true;
      return 1;
   }
   else
      return 0;
}

//get center
int VolumeSelector::GetCenter(Point& p)
{
   p = m_ps_center;
   return m_ps;
}

//get volume
int VolumeSelector::GetSize(double &s)
{
   s = m_ps_size;
   return m_ps;
}

void VolumeSelector::GenerateAnnotations(bool use_sel)
{
   if (!m_vd ||
         !m_vd->GetTexture() ||
         m_vd->GetTexture()->nmask()==-1 ||
         m_vd->GetTexture()->nlabel()==-1 ||
         m_comps.size()==0)
   {
      m_annotations = 0;
      return;
   }

   m_annotations = new Annotations();
   int nx, ny, nz;
   m_vd->GetResolution(nx, ny, nz);
	double spcx, spcy, spcz;
	m_vd->GetSpacings(spcx, spcy, spcz);

   double mul = 255.0;
   if (m_vd->GetTexture()->get_nrrd(0)->type == nrrdTypeUChar)
      mul = 255.0;
   else if (m_vd->GetTexture()->get_nrrd(0)->type == nrrdTypeUShort)
      mul = 65535.0;
   double total_int = 0.0;

   int i = 1;
   boost::unordered_map <unsigned int, Component> :: const_iterator comp_iter;
   for (comp_iter=m_comps.begin(); comp_iter!=m_comps.end(); comp_iter++)
   {
      if (comp_iter->second.counter<m_min_voxels ||
            (m_max_voxels<0.0?false:(comp_iter->second.counter>m_max_voxels)))
         continue;
      wxString str_id = wxString::Format("%d", i++);
      Vector pos = comp_iter->second.acc_pos/comp_iter->second.counter;
      pos *= Vector(nx==0?0.0:1.0/nx,
            ny==0?0.0:1.0/ny,
            nz==0?0.0:1.0/nz);
      double intensity = mul * comp_iter->second.acc_int / comp_iter->second.counter;
      total_int += intensity;
		wxString str_info = wxString::Format("%d\t%f\t%d",
			comp_iter->second.counter,
			double(comp_iter->second.counter)*(spcx*spcy*spcz),
			int(intensity+0.5));
		m_annotations->AddText(str_id.ToStdString(), Point(pos), str_info.ToStdString());
   }

   m_annotations->SetVolume(m_vd);
   m_annotations->SetTransform(m_vd->GetTexture()->transform());
	wxString info_meaning = "VOX_SIZE\tVOLUME\tAVG_VALUE";
   m_annotations->SetInfoMeaning(info_meaning);

   //memo
   wxString memo;
   memo += "Volume: " + m_vd->GetName() + "\n";
   memo += "Components: " + wxString::Format("%d", m_ca_comps) + "\n";
   memo += "Total volume: " + wxString::Format("%d", m_ca_volume) + "\n";
   memo += "Average value: " + wxString::Format("%d", int(total_int/m_ca_comps+0.5)) + "\n";
   memo += "\nSettings:\n";
   double threshold = m_label_thresh * m_vd->GetMaxValue();
   memo += "Threshold: " + wxString::Format("%f", threshold) + "\n";
	double falloff = m_label_falloff * m_vd->GetMaxValue();
	memo += "Falloff: " + wxString::Format("%f", falloff) + "\n";
   wxString str;
   if (use_sel)
      str = "Yes";
   else
      str = "No";
   memo += "Selected only: " + str + "\n";
   memo += "Minimum component size in voxel: " + wxString::Format("%d", int(m_min_voxels)) + "\n";
   if (m_max_voxels > 0.0)
      memo += "Maximum component size in voxel: " + wxString::Format("%d", int(m_max_voxels)) + "\n";
   else
      memo += "Maximum component size ignored\n";
   memo += "Iterations: " + wxString::Format("%d", m_iter_label);

   std::string str1 = memo.ToStdString();
   m_annotations->SetMemo(str1);
   m_annotations->SetMemoRO(true);
}

Annotations* VolumeSelector::GetAnnotations()
{
   return m_annotations;
}

int VolumeSelector::NoiseAnalysis(double min_voxels, double max_voxels, double bins, double thresh)
{
   if (!m_vd)
      return 0;

   m_prog_diag = new wxProgressDialog(
         "FluoRender: Noise Analysis...",
         "Analyzing... Please wait.",
         100, 0,
         wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);
   m_progress = 0;

   int nx, ny, nz;
   m_vd->GetResolution(nx, ny, nz);
   m_iter_label = Max(nx, Max(ny, nz))/10;
   m_total_pr = m_iter_label+nx*3+1;

   if (m_prog_diag)
   {
      m_progress++;
      m_prog_diag->Update(95*(m_progress+1)/m_total_pr);
   }

   //first posterize the volume and put it into the mask
   m_vd->AddEmptyMask();
   if (m_use2d && glIsTexture(m_2d_weight1) && glIsTexture(m_2d_weight2))
      m_vd->Set2DWeight(m_2d_weight1, m_2d_weight2);
   else
      m_vd->Set2DWeight(0, 0);
   double ini_thresh, gm_falloff, scl_falloff;
   if (m_ini_thresh > 0.0)
      ini_thresh = m_ini_thresh;
   else
      ini_thresh = sqrt(thresh);
   if (m_gm_falloff > 0.0)
      gm_falloff = m_gm_falloff;
   else
      gm_falloff = 0.01;
   if (m_scl_falloff > 0.0)
      scl_falloff = m_scl_falloff;
   else
      scl_falloff = 0.01;
   m_vd->DrawMask(0, 11, 0, ini_thresh, gm_falloff, scl_falloff, thresh, m_w2d, bins);

   //then label
   Label(1);
   m_vd->GetVR()->return_label();
   int return_val = CompIslandCount(min_voxels, max_voxels);

   delete m_prog_diag;

   return return_val;
}

void VolumeSelector::NoiseRemoval(int iter, double thresh, int mode)
{
   if (!m_vd || !m_vd->GetMask())
      return;

   m_prog_diag = new wxProgressDialog(
         "FluoRender: Component Analysis...",
         "Analyzing... Please wait.",
         100, 0,
         wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);
   m_progress = 0;
   m_total_pr = iter+1;

   double ini_thresh, gm_falloff, scl_falloff;
   if (m_ini_thresh > 0.0)
      ini_thresh = m_ini_thresh;
   else
      ini_thresh = sqrt(thresh);
   if (m_gm_falloff > 0.0)
      gm_falloff = m_gm_falloff;
   else
      gm_falloff = 0.01;
   if (m_scl_falloff > 0.0)
      scl_falloff = m_scl_falloff;
   else
      scl_falloff = 0.01;

   if (mode == 0)
   {
      for (int i=0; i<iter; i++)
      {
         m_vd->DrawMask(2, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
         if (m_prog_diag)
         {
            m_progress++;
            m_prog_diag->Update(95*(m_progress+1)/m_total_pr);
         }
      }
      m_vd->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, thresh, m_w2d, 0.0);
      m_vd->GetVR()->return_volume();
   }
   else if (mode == 1)
   {
      m_result_vols.clear();
      //get data and mask from the old volume
      Texture* tex_mvd = m_vd->GetTexture();
      if (!tex_mvd) return;
      Nrrd* nrrd_mvd = tex_mvd->get_nrrd(0);
      if (!nrrd_mvd) return;
      Nrrd* nrrd_mvd_mask = tex_mvd->get_nrrd(tex_mvd->nmask());
      if (!nrrd_mvd_mask) return;
      //create new volume
      int res_x, res_y, res_z;
      double spc_x, spc_y, spc_z;
      m_vd->GetResolution(res_x, res_y, res_z);
      m_vd->GetSpacings(spc_x, spc_y, spc_z);
      VolumeData* vd_new = new VolumeData();
      //add data and mask
      Nrrd *nrrd_new = nrrdNew();
      uint8 *val8 = new (std::nothrow) uint8[res_x*res_y*res_z];
      if (nrrd_mvd->type == nrrdTypeUChar)
         memcpy(val8, nrrd_mvd->data, res_x*res_y*res_z);
      else if (nrrd_mvd->type == nrrdTypeUShort)
      {
         for (int i=0; i<res_x; i++)
            for (int j=0; j<res_y; j++)
               for (int k=0; k<res_z; k++)
               {
                  int index = res_x*res_y*k + res_x*j + i;
                  val8[index] = uint8(double(((uint16*)nrrd_mvd->data)[index])*m_vd->GetScalarScale()/257.0);
               }
      }
      nrrdWrap(nrrd_new, val8, nrrdTypeUChar, 3, (size_t)res_x, (size_t)res_y, (size_t)res_z);
      nrrdAxisInfoSet(nrrd_new, nrrdAxisInfoSpacing, spc_x, spc_y, spc_z);
      nrrdAxisInfoSet(nrrd_new, nrrdAxisInfoMax, spc_x*res_x, spc_y*res_y, spc_z*res_z);
      nrrdAxisInfoSet(nrrd_new, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
      nrrdAxisInfoSet(nrrd_new, nrrdAxisInfoSize, (size_t)res_x, (size_t)res_y, (size_t)res_z);
      wxString str = "";
      vd_new->Load(nrrd_new, str, str);
      //
      Nrrd *nrrd_new_mask = nrrdNew();
      val8 = new (std::nothrow) uint8[res_x*res_y*res_z];
      memcpy(val8, nrrd_mvd_mask->data, res_x*res_y*res_z);
      nrrdWrap(nrrd_new_mask, val8, nrrdTypeUChar, 3, (size_t)res_x, (size_t)res_y, (size_t)res_z);
      nrrdAxisInfoSet(nrrd_new_mask, nrrdAxisInfoSpacing, spc_x, spc_y, spc_z);
      nrrdAxisInfoSet(nrrd_new_mask, nrrdAxisInfoMax, spc_x*res_x, spc_y*res_y, spc_z*res_z);
      nrrdAxisInfoSet(nrrd_new_mask, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
      nrrdAxisInfoSet(nrrd_new_mask, nrrdAxisInfoSize, (size_t)res_x, (size_t)res_y, (size_t)res_z);
      vd_new->LoadMask(nrrd_new_mask);
      //copy settings
      //clipping planes
      vector<Plane*> *planes = m_vd->GetVR()?m_vd->GetVR()->get_planes():0;
      if (planes && vd_new->GetVR())
         vd_new->GetVR()->set_planes(planes);
      //transfer function
      vd_new->Set3DGamma(m_vd->Get3DGamma());
      vd_new->SetBoundary(m_vd->GetBoundary());
      vd_new->SetOffset(m_vd->GetOffset());
      vd_new->SetLeftThresh(m_vd->GetLeftThresh());
      vd_new->SetRightThresh(m_vd->GetRightThresh());
      FLIVR::Color col = m_vd->GetColor();
      vd_new->SetColor(col);
      vd_new->SetAlpha(m_vd->GetAlpha());
      //shading
      vd_new->SetShading(m_vd->GetShading());
      vd_new->GetVR()->set_shading(m_vd->GetVR()->get_shading());
      double amb, diff, spec, shine;
      m_vd->GetMaterial(amb, diff, spec, shine);
      vd_new->SetMaterial(amb, diff, spec, shine);
      //shadow
      vd_new->SetShadow(m_vd->GetShadow());
      double shadow;
      m_vd->GetShadowParams(shadow);
      vd_new->SetShadowParams(shadow);
      //sample rate
      vd_new->SetSampleRate(m_vd->GetSampleRate());
      //2d adjusts
      col = m_vd->GetGamma();
      vd_new->SetGamma(col);
      col = m_vd->GetBrightness();
      vd_new->SetBrightness(col);
      col = m_vd->GetHdr();
      vd_new->SetHdr(col);
      vd_new->SetSyncR(m_vd->GetSyncR());
      vd_new->SetSyncG(m_vd->GetSyncG());
      vd_new->SetSyncB(m_vd->GetSyncB());
      //spacings
      vd_new->SetSpacings(spc_x, spc_y, spc_z);
      vd_new->SetSpcFromFile(true);
      //name etc
      vd_new->SetName(m_vd->GetName() +
            "_NR");

      for (int i=0; i<iter; i++)
      {
         vd_new->DrawMask(2, 0, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
         if (m_prog_diag)
         {
            m_progress++;
            m_prog_diag->Update(95*(m_progress+1)/m_total_pr);
         }
      }
      vd_new->DrawMask(0, 6, 0, ini_thresh, gm_falloff, scl_falloff, thresh, m_w2d, 0.0);
      vd_new->GetVR()->return_volume();

      m_result_vols.push_back(vd_new);
   }
   delete m_prog_diag;
}

void VolumeSelector::Test()
{
   /*  if (!m_vd)
       return;

       int nx, ny, nz;
       m_vd->GetResolution(nx, ny, nz);
       m_iter_label = Max(nx, Max(ny, nz))/10;

       int bins = 100;
       int value = 0;

       FILE* pfile = fopen("stats.bins", "wb");

       value = bins+1;
       fwrite(&value, sizeof(int), 1, pfile);

       for (int i=0; i<bins+1; i++)
       {
       m_label_thresh = double(i)/double(bins);
       Label(0);
       m_vd->GetVR()->return_label();
       CompIslandCount(0.0, -1.0);

       value = m_comps.size();
       fwrite(&value, sizeof(int), 1, pfile);

       unordered_map <unsigned int, Component> :: const_iterator comp_iter;
       for (comp_iter=m_comps.begin(); comp_iter!=m_comps.end(); comp_iter++)
       {
       value = comp_iter->second.counter;
       fwrite(&value, sizeof(int), 1, pfile);
       }
       }

       fclose(pfile);*/
}

