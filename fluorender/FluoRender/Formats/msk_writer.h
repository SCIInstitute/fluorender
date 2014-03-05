#ifndef _MSK_WRITER_H_
#define _MSK_WRITER_H_

#include <base_writer.h>

class MSKWriter : public BaseWriter
{
public:
	MSKWriter();
	~MSKWriter();

	void SetData(Nrrd* data);
	void SetSpacings(double spcx, double spcy, double spcz);
	void SetCompression(bool value);
	void Save(wstring filename, int mode);//mode: 0-normal mask; 1-label mask

	void SetTC(int t, int c);

private:
	Nrrd* m_data;
	double m_spcx, m_spcy, m_spcz;
	bool m_use_spacings;

	int m_time;
	int m_channel;
};

#endif//_MSK_WRITER_H_
