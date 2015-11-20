#include "Formats/tif_reader.h"
#include "Formats/msk_reader.h"
#include "Formats/msk_writer.h"
#include <string>
#include <codecvt>
#include <deque>
//#include <vld.h>

typedef struct
{
	int x;
	int y;
} Pixel;

typedef std::deque<Pixel> PixelList;

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

bool GetPixelList(void* data, int nx, int ny,
	PixelList &list)
{
	unsigned int index;
	unsigned char value;
	unsigned char nv;
	//find the first bounday pixel
	Pixel sp;
	sp.x = -1; sp.y = -1;
	for (int i = 0; i < nx; ++i)
	for (int j = 0; j < ny; ++j)
	{
		index = nx*j + i;
		value = ((unsigned char*)data)[index];
		if (!value)
			continue;
		//eight directions
		if (GetNeighbor(data, nx, ny, i, j, -1, -1, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, 0, -1, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, 1, -1, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, -1, 0, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, 1, 0, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, -1, 1, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, 0, 1, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
		if (GetNeighbor(data, nx, ny, i, j, 1, 1, nv) &&
			!nv)
		{
			sp.x = i;
			sp.y = j;
			break;
		}
	}

	if (sp.x == -1 || sp.y == -1)
		return false;

	list.push_back(sp);
	return true;
}

int main(int argc, char* argv[])
{
	if (argc < 4)
	{
		printf("Wrong arguments.\n" \
			"Usage: MaskInt.exe mask1 mask2 num\n");
		return 1;
	}

	std::wstring mask1file = s2ws(argv[1]);
	std::wstring mask2file = s2ws(argv[2]);
	int int_num = atoi(argv[3]);

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
	printf("All done. Quit.\n");

	int nx = data1->axis[0].size;
	int ny = data1->axis[1].size;
	int nz = data1->axis[2].size;

	PixelList list1, list2;

	for (int z = 0; z < nz; ++z)
	{
		if (GetPixelList(data1->data, nx, ny, list1) &&
			GetPixelList(data2->data, nx, ny, list2))
		{
			printf("ph\n");
		}

	}

	return 0;
}