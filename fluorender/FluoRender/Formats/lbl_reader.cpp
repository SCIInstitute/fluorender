#include <Formats\lbl_reader.h>
#include <sstream>

LBLReader::LBLReader()
{
	m_id_string = L"FluoRender_mask_reader_id";
}

LBLReader::~LBLReader()
{
}

void LBLReader::SetFile(string &file)
{
	if (!file.empty())
	{
		if (!m_path_name.empty())
			m_path_name.clear();
		m_path_name.assign(file.length(), L' ');
		copy(file.begin(), file.end(), m_path_name.begin());
	}
}

void LBLReader::SetFile(wstring &file)
{
	m_path_name = file;
}

void LBLReader::Preprocess()
{
}

void LBLReader::SetSliceSeq(bool ss)
{
}

bool LBLReader::GetSliceSeq()
{
	return false;
}

void LBLReader::SetTimeId(wstring &id)
{
}

wstring LBLReader::GetTimeId()
{
	return wstring(L"");
}

void LBLReader::SetBatch(bool batch)
{
}

int LBLReader::LoadBatch(int index)
{
	return 0;
}

int LBLReader::LoadOffset(int offset)
{
	return 0;
}

Nrrd* LBLReader::Convert(bool get_max)
{
	return Convert(0, 0, get_max);
}

Nrrd* LBLReader::Convert(int c, bool get_max)
{
	return Convert(0, c, get_max);
}

Nrrd* LBLReader::Convert(int t, int c, bool get_max)
{
	size_t pos = m_path_name.find_last_of('.');
	if (pos == -1)
		return 0;
	wstring str_name = m_path_name.substr(0, pos);
	wostringstream strs;
	strs << str_name /*<< "_t" << t << "_c" << c*/ << ".lbl";
	str_name = strs.str();

	Nrrd *output = nrrdNew();
	NrrdIoState *nio = nrrdIoStateNew();
	nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);
	string str;
	str.assign(str_name.length(), 0);
	for (int i=0; i<(int)str_name.length(); i++)
		str[i] = (char)str_name[i];
	if (nrrdLoad(output, str.c_str(), nio))
		return 0;
	nio = nrrdIoStateNix(nio);
	if (output->dim != 3 ||
		(output->type != nrrdTypeInt &&
		output->type != nrrdTypeUInt))
	{
		nrrdNix(output);
		return 0;
	}
	int slice_num = int(output->axis[2].size);
	int x_size = int(output->axis[0].size);
	int y_size = int(output->axis[1].size);
	int data_size = slice_num * x_size * y_size;
	output->data = new unsigned int[data_size];

	if (nrrdLoad(output, str.c_str(), NULL))
	{
		delete []output->data;
		nrrdNix(output);
		return 0;
	}

	return output;
}

wstring LBLReader::GetCurName(int t, int c)
{
	return wstring(L"");
}
