#include "Formats/tif_reader.h"
#include "Formats/msk_reader.h"
#include "Formats/msk_writer.h"
#include <string>
#include <codecvt>
//#include <vld.h>

inline std::wstring s2ws(const std::string& utf8) {
	//    return std::wstring( str.begin(), str.end() );
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
	return converter.from_bytes(utf8);
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

	return 0;
}