#include "VolumeCalculator.h"
#include <wx/progdlg.h>

VolumeCalculator::VolumeCalculator()
: m_vd_r(0),
   m_vd_a(0),
   m_vd_b(0),
   m_type(0)
{
}

VolumeCalculator::~VolumeCalculator()
{
}

void VolumeCalculator::SetVolumeA(VolumeData *vd)
{
   m_vd_a = vd;
}

void VolumeCalculator::SetVolumeB(VolumeData *vd)
{
   m_vd_b = vd;
}

VolumeData* VolumeCalculator::GetVolumeA()
{
   return m_vd_a;
}

VolumeData* VolumeCalculator::GetVolumeB()
{
   return m_vd_b;
}

VolumeData* VolumeCalculator::GetResult()
{
   return m_vd_r;
}

void VolumeCalculator::Calculate(int type)
{
   m_type = type;
   m_vd_r = 0;

   switch (m_type)
   {
   case 1:
   case 2:
   case 3:
   case 4:
   case 8://intersection with mask
      CreateVolumeResult2();
      if (!m_vd_r)
         return;
      m_vd_r->Calculate(m_type, m_vd_a, m_vd_b);
      return;
   case 5:
   case 6:
   case 7:
      if (!m_vd_a || !m_vd_a->GetMask())
         return;
      CreateVolumeResult1();
      if (!m_vd_r)
         return;
      m_vd_r->Calculate(m_type, m_vd_a, 0);
      return;
   case 9:
      if (!m_vd_a)
         return;
      CreateVolumeResult1();
      if (!m_vd_r)
         return;
      FillHoles();
      return;
   }
}

void VolumeCalculator::CreateVolumeResult1()
{
   if (!m_vd_a)
      return;

   int res_x, res_y, res_z;
   double spc_x, spc_y, spc_z;

   m_vd_a->GetResolution(res_x, res_y, res_z);
   m_vd_a->GetSpacings(spc_x, spc_y, spc_z);

   //int bits = (m_vd_a->GetMaxValue()>255.0)?
   //  16:8;
   int bits = 8;  //it has an unknown problem with 16 bit data

   m_vd_r = new VolumeData();
   m_vd_r->AddEmptyData(bits,
         res_x, res_y, res_z,
         spc_x, spc_y, spc_z);
   m_vd_r->SetSpcFromFile(true);

   wxString name = m_vd_a->GetName();
   wxString str_type;
   switch (m_type)
   {
   case 5://substraction
      str_type = "_EXTRACTED";
      break;
   case 6:
      str_type = "_DELETED";
      break;
   case 9:
      str_type = "_FILLED";
      break;
   }
   m_vd_r->SetName(name + str_type);
}

void VolumeCalculator::CreateVolumeResult2()
{
   if (!m_vd_a || !m_vd_b)
      return;

   int res_x_a, res_y_a, res_z_a;
   int res_x_b, res_y_b, res_z_b;
   double spc_x_a, spc_y_a, spc_z_a;
   double spc_x_b, spc_y_b, spc_z_b;

   m_vd_a->GetResolution(res_x_a, res_y_a, res_z_a);
   m_vd_b->GetResolution(res_x_b, res_y_b, res_z_b);
   m_vd_a->GetSpacings(spc_x_a, spc_y_a, spc_z_a);
   m_vd_b->GetSpacings(spc_x_b, spc_y_b, spc_z_b);

   //int bits = (m_vd_a->GetMaxValue()>255.0||m_vd_b->GetMaxValue()>255.0)?
   //  16:8;
   int bits = 8;  //it has an unknown problem with 16 bit data
   int res_x, res_y, res_z;
   double spc_x, spc_y, spc_z;

   res_x = max(res_x_a, res_x_b);
   res_y = max(res_y_a, res_y_b);
   res_z = max(res_z_a, res_z_b);
   spc_x = max(spc_x_a, spc_x_b);
   spc_y = max(spc_y_a, spc_y_b);
   spc_z = max(spc_z_a, spc_z_b);

   m_vd_r = new VolumeData();
   m_vd_r->AddEmptyData(bits,
         res_x, res_y, res_z,
         spc_x, spc_y, spc_z);
   m_vd_r->SetSpcFromFile(true);

   wxString name_a = m_vd_a->GetName();
   wxString name_b = m_vd_b->GetName();
   size_t len = 15;
   if (name_a.length() > len)
      name_a = name_a.Left(len);
   if (name_b.length() > len)
      name_b = name_b.Left(len);
   wxString str_type;
   switch (m_type)
   {
   case 1://substraction
      str_type = "_SUB_";
      break;
   case 2://addition
      str_type = "_ADD_";
      break;
   case 3://division
      str_type = "_DIV_";
      break;
   case 4://intersection
      str_type = "_AND_";
      break;
   }
   wxString name = name_a + str_type + name_b;
   m_vd_r->SetName(name);
}

//fill holes
void VolumeCalculator::FillHoles()
{
   if (!m_vd_a || !m_vd_r)
      return;

   Texture* tex_a = m_vd_a->GetTexture();
   if (!tex_a)
      return;
   Nrrd* nrrd_a = tex_a->get_nrrd(0);
   if (!nrrd_a)
      return;
   void* data_a = nrrd_a->data;
   if (!data_a)
      return;

   Texture* tex_r = m_vd_r->GetTexture();
   if (!tex_r)
      return;
   Nrrd* nrrd_r = tex_r->get_nrrd(0);
   if (!nrrd_r)
      return;
   void* data_r = nrrd_r->data;
   if (!data_r)
      return;

   //resolution
   int nx, ny, nz;
   m_vd_a->GetResolution(nx, ny, nz);

   wxProgressDialog *prog_diag = new wxProgressDialog(
         "FluoRender: Filling holes...",
         "Filling... Please wait.",
         100, 0,
         wxPD_SMOOTH|wxPD_ELAPSED_TIME|wxPD_AUTO_HIDE);
   int progress = 0;
   int total_prg = nx*2;

   int i, j, k;
   BBox bbox;
   //first pass: finding BBox
   for (i=0; i<nx; i++)
   {
      for (j=0; j<ny; j++)
         for (k=0; k<nz; k++)
         {
            int index = nx*ny*k + nx*j + i;
            unsigned char value_a = 0;
            if (nrrd_a->type == nrrdTypeUChar)
               value_a = ((unsigned char*)data_a)[index];
            else if (nrrd_a->type == nrrdTypeUShort)
               value_a = (unsigned char)((double)(((unsigned short*)data_a)[index])*m_vd_a->GetScalarScale()/257.0);
            if (value_a)
            {
               bbox.extend(Point(i, j, k));
               ((unsigned char*)data_r)[index] = 255;
            }
         }
      if (prog_diag)
      {
         progress++;
         prog_diag->Update(95*(progress+1)/total_prg);
      }
   }

   //second pass: fill holes
   for (i=int(bbox.min().x()); i<=int(bbox.max().x()); i++)
   {
      for (j=int(bbox.min().y()); j<=int(bbox.max().y()); j++)
         for (k=int(bbox.min().z()); k<=int(bbox.max().z()); k++)
         {
            int index = nx*ny*k + nx*j + i;
            unsigned char value_r = ((unsigned char*)data_r)[index];
            if (!value_r)
            {
               //search index
               int si;
               //search -X
               int s_n_x = i;
               while (s_n_x >= int(bbox.min().x()))
               {
                  si = nx*ny*k + nx*j + s_n_x;
                  if (((unsigned char*)data_r)[si])
                     break;
                  s_n_x--;
               }
               //search +X
               int s_p_x = i;
               while (s_p_x <= int(bbox.max().x()))
               {
                  si = nx*ny*k + nx*j + s_p_x;
                  if (((unsigned char*)data_r)[si])
                     break;
                  s_p_x++;
               }
               //found X direction?
               if (s_n_x >= int(bbox.min().x()) &&
                     s_p_x <= int(bbox.max().x()))
               {
                  ((unsigned char*)data_r)[index] = 255;
                  continue;
               }
               //search -Y
               int s_n_y = j;
               while (s_n_y >= int(bbox.min().y()))
               {
                  si = nx*ny*k + nx*s_n_y + i;
                  if (((unsigned char*)data_r)[si])
                     break;
                  s_n_y--;
               }
               //search +Y
               int s_p_y = j;
               while (s_p_y <= int(bbox.max().y()))
               {
                  si = nx*ny*k + nx*s_p_y + i;
                  if (((unsigned char*)data_r)[si])
                     break;
                  s_p_y++;
               }
               //found Y direction?
               if (s_n_y >= int(bbox.min().y()) &&
                     s_p_y <= int(bbox.max().y()))
               {
                  ((unsigned char*)data_r)[index] = 255;
                  continue;
               }
               /*//search -Z
                 int s_n_z = k;
                 while (s_n_z >= int(bbox.min().z()))
                 {
                 si = nx*ny*s_n_z + nx*j + i;
                 if (((unsigned char*)data_r)[si])
                 break;
                 s_n_z--;
                 }
               //search +Z
               int s_p_z = k;
               while (s_p_z <= int(bbox.max().z()))
               {
               si = nx*ny*s_p_z + nx*j + i;
               if (((unsigned char*)data_r)[si])
               break;
               s_p_z++;
               }
               //found Z direction?
               if (s_n_z >= int(bbox.min().z()) &&
               s_p_z <= int(bbox.max().z()))
               {
               ((unsigned char*)data_r)[index] = 255;
               continue;
               }*/
            }
         }

      if (prog_diag)
      {
         progress++;
         prog_diag->Update(95*(progress+1)/total_prg);
      }
   }
   delete prog_diag;
}
