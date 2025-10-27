#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <cstdlib>
#include <stdio.h>
#include <cassert>

#define GLHINT glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE)

#ifdef _WIN32

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <strsafe.h>
#define WINDOWS_LEAN_AND_MEAN
#include <Windows.h>

#ifdef _DEBUG
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
// allocations to be of _CLIENT_BLOCK type
#define DBGPRINT(kwszDebugFormatString, ...) _DBGPRINT(__FUNCTIONW__, __LINE__, kwszDebugFormatString, __VA_ARGS__)

VOID _DBGPRINT(LPCWSTR kwszFunction, INT iLineNumber, LPCWSTR kwszDebugFormatString, ...);

//visualizer structures
struct DBMIUINT8
{
	DBMIUINT8()
	{
		release_data = false;
	}
	DBMIUINT8(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new unsigned char[(unsigned long long)nx*ny]();
		nt = x * c;
		release_data = true;
	}
	~DBMIUINT8()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, unsigned char v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	unsigned char* data;
};

struct DBMIINT8
{
	DBMIINT8()
	{
		release_data = false;
	}
	DBMIINT8(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new char[(unsigned long long)nx*ny]();
		nt = x * c;
		release_data = true;
	}
	~DBMIINT8()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, char v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	char* data;
};

struct DBMIUINT16
{
	DBMIUINT16()
	{
		release_data = false;
	}
	DBMIUINT16(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new unsigned short[(unsigned long long)nx*ny]();
		nt = x * c * 2;
		release_data = true;
	}
	~DBMIUINT16()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, unsigned short v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	unsigned short* data;
};

struct DBMIINT16
{
	DBMIINT16()
	{
		release_data = false;
	}
	DBMIINT16(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new short[(unsigned long long)nx*ny]();
		nt = x * c * 2;
		release_data = true;
	}
	~DBMIINT16()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, short v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	short* data;
};

struct DBMIINT32
{
	DBMIINT32()
	{
		release_data = false;
	}
	DBMIINT32(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new unsigned int[(unsigned long long)nx*ny]();
		nt = x * c * 4;
		release_data = true;
	}
	~DBMIINT32()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, unsigned int v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	unsigned int* data;
};

struct DBMIFLOAT32
{
	DBMIFLOAT32()
	{
		release_data = false;
	}
	DBMIFLOAT32(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new float[(unsigned long long)nx*ny]();
		nt = x * c * 4;
		release_data = true;
	}
	~DBMIFLOAT32()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, float v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	float* data;
};

struct DBMIFLOAT64
{
	DBMIFLOAT64()
	{
		release_data = false;
	}
	DBMIFLOAT64(unsigned int x, unsigned int y, unsigned int c)
	{
		nx = x;
		ny = y;
		nc = c;
		data = new double[(unsigned long long)nx*ny]();
		nt = x * c * 8;
		release_data = true;
	}
	~DBMIFLOAT64()
	{
		if (release_data)
			delete[] data;
	}
	void set(unsigned int x, unsigned int y, double v)
	{
		unsigned long long idx = (unsigned long long)y * nx + x;
		data[idx] = v;
	}
	unsigned int nx;
	unsigned int ny;
	unsigned int nc;
	unsigned int nt;
	bool release_data;
	double* data;
};

#else//_DEBUG
#define DBG_NEW new
#define DBGPRINT( kwszDebugFormatString, ... ) ((void)0)
#endif//_DEBUG

#else//_WIN32

#include <cstdarg>
#include <wchar.h>
void DBGPRINT(const wchar_t* format, ...);
//{}

#endif//_WIN32

#endif//_DEBUG_H_
