#include "nrrd_writer.h"

NRRDWriter::NRRDWriter()
{
   m_data = 0;
   m_spcx = 0.0;
   m_spcy = 0.0;
   m_spcz = 0.0;
   m_use_spacings = false;
}

NRRDWriter::~NRRDWriter()
{
}

void NRRDWriter::SetData(Nrrd *data)
{
   m_data = data;
}

void NRRDWriter::SetSpacings(double spcx, double spcy, double spcz)
{
   m_spcx = spcx;
   m_spcy = spcy;
   m_spcz = spcz;
   m_use_spacings = true;
}

void NRRDWriter::SetCompression(bool value)
{
}

void NRRDWriter::Save(wstring filename, int mode)
{
   if (!m_data)
      return;

   if (m_use_spacings &&
         m_data->dim == 3)
   {
      nrrdAxisInfoSet(m_data, nrrdAxisInfoSpacing, m_spcx, m_spcy, m_spcz);
      nrrdAxisInfoSet(m_data, nrrdAxisInfoMax,
            m_spcx*m_data->axis[0].size,
            m_spcy*m_data->axis[1].size,
            m_spcz*m_data->axis[2].size);
   }

   string str;
   str.assign(filename.length(), 0);
   for (int i=0; i<(int)filename.length(); i++)
      str[i] = (char)filename[i];
   nrrdSave(str.c_str(), m_data, NULL);
}
