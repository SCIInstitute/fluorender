#ifndef _TIF_WRITER_H_
#define _TIF_WRITER_H_

#include <base_writer.h>

class TIFWriter : public BaseWriter
{
public:
	TIFWriter();
	~TIFWriter();

	void SetData(Nrrd* data);
	void SetSpacings(double spcx, double spcy, double spcz);
	void SetCompression(bool value);
	void Save(wstring filename, int mode);	//mode: 0-single file
											//1-file sequence

private:
	Nrrd* m_data;
	double m_spcx, m_spcy, m_spcz;
	bool m_use_spacings;
	bool m_compression;

private:
	void SaveSingleFile(wstring filename);
	void SaveSequence(wstring filename);
};

#endif//_TIF_WRITER_H_
