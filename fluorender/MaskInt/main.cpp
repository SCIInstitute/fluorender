#include "Formats/tif_reader.h"
#include "Formats/tif_writer.h"
#include "Formats/msk_reader.h"
#include "Formats/msk_writer.h"
#include <string>
#include <codecvt>
#include <deque>
#include <stack>
//#include <vld.h>

typedef struct Pixel_
{
	int x;
	int y;
	bool operator==(const Pixel_& p2)
	{
		return x == p2.x && y == p2.y;
	}
} Pixel;

typedef std::deque<Pixel> PixelList;
typedef std::stack<Pixel> PixelStack;

inline std::wstring s2ws(const std::string& utf8)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.from_bytes(utf8);
}

bool GetNeighbor(void *data,
	int nx, int ny,
	int x, int y,
	int dx, int dy,
	unsigned char &value)
{
	int ix = x + dx;
	if (ix < 0 || ix >= nx)
		return false;
	int iy = y + dy;
	if (iy < 0 || iy >= ny)
		return false;
	value = ((unsigned char*)data)[nx*iy+ix];
	return true;
}

bool BorderPixel(void* data,
	int nx, int ny,
	int i, int j)
{
	if (i < 0 || i >= nx)
		return false;
	if (j < 0 || j >= ny)
		return false;

	unsigned int index = nx*j + i;
	unsigned char value = ((unsigned char*)data)[index];
	if (!value)
		return false;
	unsigned char nv;
	//eight directions
	//if (GetNeighbor(data, nx, ny, i, j, -1, -1, nv) &&
	//	!nv)
	//	return true;
	if (GetNeighbor(data, nx, ny, i, j, 0, -1, nv) &&
		!nv)
		return true;
	//if (GetNeighbor(data, nx, ny, i, j, 1, -1, nv) &&
	//	!nv)
	//	return true;
	if (GetNeighbor(data, nx, ny, i, j, -1, 0, nv) &&
		!nv)
		return true;
	if (GetNeighbor(data, nx, ny, i, j, 1, 0, nv) &&
		!nv)
		return true;
	//if (GetNeighbor(data, nx, ny, i, j, -1, 1, nv) &&
	//	!nv)
	//	return true;
	if (GetNeighbor(data, nx, ny, i, j, 0, 1, nv) &&
		!nv)
		return true;
	//if (GetNeighbor(data, nx, ny, i, j, 1, 1, nv) &&
	//	!nv)
	//	return true;
	return false;
}

bool PixelInList(Pixel& p, PixelList &list)
{
	for (size_t i = 0; i < list.size(); ++i)
		if (p == list[i])
			return true;
	return false;
}

bool GetNextPixel(void* data, int nx, int ny,
	int x, int y, PixelList &list, Pixel &p)
{
	//eight neighbors
	p.x = x - 1; p.y = y - 1;
	if (BorderPixel(data, nx, ny, x - 1, y - 1) &&
		!PixelInList(p, list))
		return true;
	p.x = x; p.y = y - 1;
	if (BorderPixel(data, nx, ny, x, y - 1) &&
		!PixelInList(p, list))
		return true;
	p.x = x + 1; p.y = y - 1;
	if (BorderPixel(data, nx, ny, x + 1, y - 1) &&
		!PixelInList(p, list))
		return true;
	p.x = x - 1; p.y = y;
	if (BorderPixel(data, nx, ny, x - 1, y) &&
		!PixelInList(p, list))
		return true;
	p.x = x + 1; p.y = y;
	if (BorderPixel(data, nx, ny, x + 1, y) &&
		!PixelInList(p, list))
		return true;
	p.x = x - 1; p.y = y + 1;
	if (BorderPixel(data, nx, ny, x - 1, y + 1) &&
		!PixelInList(p, list))
		return true;
	p.x = x; p.y = y + 1;
	if (BorderPixel(data, nx, ny, x, y + 1) &&
		!PixelInList(p, list))
		return true;
	p.x = x + 1; p.y = y + 1;
	if (BorderPixel(data, nx, ny, x + 1, y + 1) &&
		!PixelInList(p, list))
		return true;
	return false;
}

bool SearchHead(void* data, int nx, int ny,
	int x, int y, PixelList& list)
{
	Pixel p;
	p.x = -1; p.y = -1;
	if (GetNextPixel(data, nx, ny, x, y, list, p))
	{
		list.push_front(p);
		SearchHead(data, nx, ny, p.x, p.y, list);
		return true;
	}
	return false;
}

bool SearchTail(void* data, int nx, int ny,
	int x, int y, PixelList& list)
{
	Pixel p;
	p.x = -1; p.y = -1;
	if (GetNextPixel(data, nx, ny, x, y, list, p))
	{
		list.push_back(p);
		SearchTail(data, nx, ny, p.x, p.y, list);
		return true;
	}
	return false;
}

bool GetPixelList(void* data, int nx, int ny,
	int z, PixelList &list)
{
	unsigned char* page = ((unsigned char*)data) + nx*ny*z;
	unsigned char value;
	unsigned char nv;
	//find the first bounday pixel
	Pixel sp;
	sp.x = -1; sp.y = -1;
	for (int i = 0; i < nx; ++i)
	for (int j = 0; j < ny; ++j)
	{
		if (BorderPixel(page, nx, ny, i, j))
		{
			sp.x = i;
			sp.y = j;
			break;
		}
	}

	if (sp.x == -1 || sp.y == -1)
		return false;

	list.push_back(sp);

	SearchHead(page, nx, ny, sp.x, sp.y, list);
	SearchTail(page, nx, ny, sp.x, sp.y, list);

	return true;
}

bool SearchBorder(void* page, int nx, int ny,
	int x, int y, int ix, int iy, Pixel& fp)
{
	PixelList list;
	for (int i = x, j = y;
		i < nx && j < ny;
		i += ix, j += iy)
	{
		unsigned int index = nx*j + i;
		unsigned char value = ((unsigned char*)page)[index];
		if (value)
		{
			Pixel p;
			p.x = i;
			p.y = j;
			list.push_back(p);
		}
		else if (!list.empty())
		{
			if (list.size() > 1)
				break;
			else
				list.clear();
		}
	}

	double fpx, fpy;
	fpx = fpy = 0;
	if (list.size() > 1)
	{
		for (size_t i = 0; i < list.size(); ++i)
		{
			fpx += list[i].x;
			fpy += list[i].y;
		}

		fpx /= list.size();
		fpy /= list.size();
		fp.x = int(fpx + 0.5);
		fp.y = int(fpy + 0.5);
		return true;
	}

	return false;
}

bool GetFillPixel(void* data1, void* data2,
	int nx, int ny, int z, Pixel& fp)
{
	unsigned char* page1 = ((unsigned char*)data1) + nx*ny*z;
	unsigned char* page2 = ((unsigned char*)data2) + nx*ny*z;

	Pixel p1, p2;
	//top
	if (SearchBorder(page1, nx, ny, 0, 0, 1, 0, p1) &&
		SearchBorder(page2, nx, ny, 0, 0, 1, 0, p2))
	{
		fp.x = (p1.x + p2.x) / 2;
		fp.y = (p1.y + p2.y) / 2;
		return true;
	}
	//bottom
	if (SearchBorder(page1, nx, ny, 0, ny-1, 1, 0, p1) &&
		SearchBorder(page2, nx, ny, 0, ny - 1, 1, 0, p2))
	{
		fp.x = (p1.x + p2.x) / 2;
		fp.y = (p1.y + p2.y) / 2;
		return true;
	}
	//left
	if (SearchBorder(page1, nx, ny, 0, 0, 0, 1, p1) &&
		SearchBorder(page2, nx, ny, 0, 0, 0, 1, p2))
	{
		fp.x = (p1.x + p2.x) / 2;
		fp.y = (p1.y + p2.y) / 2;
		return true;
	}
	//right
	if (SearchBorder(page1, nx, ny, nx-1, 0, 0, 1, p1) &&
		SearchBorder(page2, nx, ny, nx-1, 0, 0, 1, p2))
	{
		fp.x = (p1.x + p2.x) / 2;
		fp.y = (p1.y + p2.y) / 2;
		return true;
	}

	return false;
}

Nrrd* NewData(int nx, int ny, int nz, int bits,
	double spcx, double spcy, double spcz)
{
	Nrrd* result = nrrdNew();
	unsigned char *val8 = 0;
	unsigned short *val16 = 0;
	unsigned long long mem_size = (unsigned long long)nx*
		(unsigned long long)ny*(unsigned long long)nz;
	if (bits == 8)
	{
		val8 = new (std::nothrow) unsigned char[mem_size];
		if (!val8)
			return 0;
		memset(val8, 0, sizeof(unsigned char)*mem_size);
	}
	else if (bits == 16)
	{
		val16 = new (std::nothrow) unsigned short[mem_size];
		if (!val16)
			return 0;
		memset(val16, 0, sizeof(unsigned short)*mem_size);
	}

	if (bits == 8)
		nrrdWrap(result, val8, nrrdTypeUChar, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	else if (bits == 16)
		nrrdWrap(result, val16, nrrdTypeUShort, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	nrrdAxisInfoSet(result, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);
	nrrdAxisInfoSet(result, nrrdAxisInfoSpacing, spcx, spcy, spcz);
	nrrdAxisInfoSet(result, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(result, nrrdAxisInfoMax, spcx*nx, spcy*ny, spcz*nz);

	return result;
}

void ZeroData(void* data, int nx, int ny, int nz, int bits)
{
	memset(data, 0, nx*ny*nz*(bits / 8));
}

void WriteList(void* data, int nx, int ny, int z, PixelList& list)
{
	unsigned char* page = ((unsigned char*)data) + z*nx*ny;
	for (size_t i = 0; i < list.size(); ++i)
	{
		int index = nx*list[i].y + list[i].x;
		page[index] = 255;
	}
}

bool FillPixel(void* page, int nx, int ny, int x, int y)
{
	if (x < 0 || x >= nx)
		return false;
	if (y < 0 || y >= ny)
		return false;

	unsigned int index = nx*y + x;
	unsigned char value = ((unsigned char*)page)[index];
	if (value)
		return false;
	else
	{
		((unsigned char*)page)[index] = 255;
		return true;
	}
}

void FillData(void* data, int nx, int ny, int z, Pixel& fp)
{
	unsigned char* page = ((unsigned char*)data) + z*nx*ny;
	PixelStack stk;
	stk.push(fp);
	while (!stk.empty())
	{
		Pixel p = stk.top();
		bool filled = FillPixel(page, nx, ny, p.x, p.y);
		stk.pop();

		if (filled)
		{
			//add neighbors
			Pixel np;
			np.x = p.x - 1;
			np.y = p.y;
			stk.push(np);
			np.x = p.x + 1;
			np.y = p.y;
			stk.push(np);
			np.x = p.x;
			np.y = p.y - 1;
			stk.push(np);
			np.x = p.x;
			np.y = p.y + 1;
			stk.push(np);
		}
	}
}

void Interpolate(PixelList& list1,
	PixelList& list2, PixelList& list_out,
	double t)
{
	if (list1.size() < list2.size())
		Interpolate(list2, list1, list_out, 1.0 - t);

	Pixel p1, p2, p_out;
	int i2;
	for (size_t i = 0; i < list1.size(); ++i)
	{
		p1 = list1[i];
		i2 = int((double)i*(double)list2.size() / (double)list1.size() + 0.5);
		p2 = list2[i2];
		p_out.x = p1.x + int((p2.x - p1.x) * t + 0.5);
		p_out.y = p1.y + int((p2.y - p1.y) * t + 0.5);
		list_out.push_back(p_out);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 5)
	{
		printf("Wrong arguments.\n" \
			"Usage: MaskInt.exe mask1 mask2 num\n");
		return 1;
	}

	std::wstring mask1file = s2ws(argv[1]);
	std::wstring mask2file = s2ws(argv[2]);
	std::wstring outfile = s2ws(argv[3]);
	double int_t = atof(argv[4]);

	TIFReader reader;
	reader.SetFile(mask1file);
	reader.SetSliceSeq(false);
	std::wstring str_w = L"#$%&@*";
	reader.SetTimeId(str_w);
	reader.Preprocess();

	Nrrd* data1 = reader.Convert(0, 0, true);
	if (!data1)
	{
		printf("Volume mask 1 loading error.\n");
		return 2;
	}

	reader.SetFile(mask2file);
	reader.SetSliceSeq(false);
	str_w = L"#$%&@*";
	reader.SetTimeId(str_w);
	reader.Preprocess();

	Nrrd* data2 = reader.Convert(0, 0, true);
	if (!data2)
	{
		printf("Volume mask 2 loading error.\n");
		return 3;
	}
	int nx = data1->axis[0].size;
	int ny = data1->axis[1].size;
	int nz = data1->axis[2].size;
	double spcx = data1->axis[0].spacing;
	double spcy = data1->axis[1].spacing;
	double spcz = data1->axis[2].spacing;

	PixelList list1, list2, list_out;

	Nrrd* data_out = NewData(nx, ny, nz, 8, spcx, spcy, spcz);
	for (int z = 0; z < nz; ++z)
	{
		list1.clear();
		list2.clear();
		list_out.clear();

		if (GetPixelList(data1->data, nx, ny, z, list1) &&
			GetPixelList(data2->data, nx, ny, z, list2))
		{
			Interpolate(list1, list2, list_out, int_t);
			WriteList(data_out->data, nx, ny, z, list_out);
			Pixel fp;
			if (GetFillPixel(data1->data, data2->data, nx, ny, z, fp))
				FillData(data_out->data, nx, ny, z, fp);
		}
	}

	TIFWriter writer;
	writer.SetData(data_out);
	writer.SetSpacings(spcx, spcy, spcz);
	writer.SetCompression(false);
	writer.Save(outfile.c_str(), 0);

	printf("All done. Quit.\n");

	return 0;
}