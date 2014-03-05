#ifndef _NRRD_WRITER_H_
#define _NRRD_WRITER_H_

#include <base_writer.h>

class NRRDWriter : public BaseWriter
{
public:
	NRRDWriter();
	~NRRDWriter();

	void SetData(Nrrd* data);
	void SetSpacings(double spcx, double spcy, double spcz);
	void SetCompression(bool value);
	void Save(wstring filename, int mode);

private:
	Nrrd* m_data;
	double m_spcx, m_spcy, m_spcz;
	bool m_use_spacings;
};

#endif//_NRRD_WRITER_H_
