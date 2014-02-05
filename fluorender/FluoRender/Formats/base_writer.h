#ifndef _BASE_WRITER_H_
#define _BASE_WRITER_H_

#include <string>
#include <nrrd.h>

using namespace std;

#ifdef STATIC_COMPILE
	#define nrrdWrap nrrdWrap_va
	#define nrrdAxisInfoSet nrrdAxisInfoSet_va
#endif

class BaseWriter
{
public:
	//BaseWriter();
	virtual ~BaseWriter() {};

	virtual void SetData(Nrrd* data) = 0;
	virtual void SetSpacings(double spcx, double spcy, double spcz) = 0;
	virtual void SetCompression(bool value) = 0;
	virtual void Save(wstring filename, int mode) = 0;
};

#endif//_BASE_WRITER_H_