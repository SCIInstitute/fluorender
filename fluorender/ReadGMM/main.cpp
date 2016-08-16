#include <stdlib.h>
#include <codecvt>
#include <sstream>
#include <iomanip>
#include <wx/wx.h>
#include <wx/xml/xml.h>
#include "../compatibility.h"
#include <nrrd.h>
#include <Formats/msk_writer.h>
#include <TrackMap.h>
#include <glm/glm.hpp>
//#include <vld.h>

int nx, ny, nz;
vector<std::wstring> m_xml_list;
vector<std::wstring> m_lbl_list;
int m_digits = 0;
FL::TrackMap m_track_map;
FL::TrackMapProcessor m_tm_processor(m_track_map);

void ProcessInputName(std::wstring &infilename)
{
	//separate path and name
	int64_t pos = infilename.find_last_of(GETSLASH());
	wstring path = infilename.substr(0, pos + 1);
	wstring name = infilename.substr(pos + 1);

	//find seq number in name
	size_t pos2 = name.find_last_of(L'.');
	size_t begin2 = 0;
	int64_t end2 = -1;
	for (int i = int(pos2) - 1; i >= 0; i--)
	{
		if (iswdigit(name[i]) && end2 == -1)
			end2 = i;
		if (!iswdigit(name[i]) && end2 != -1)
		{
			begin2 = i;
			break;
		}
	}
	if (end2 != -1)
	{
		m_digits = end2 - begin2;
		//search slice sequence
		wstring regex = name.substr(0, begin2 + 1);
		int cur_time = 0;
		FIND_FILES(path, L".xml", m_xml_list, cur_time, regex);
	}
}

void ProcessOutputName(std::wstring &outfilename)
{
	int num = m_xml_list.size();
	for (int i = 0; i < num; ++i)
	{
		wstring xmlfile = m_xml_list[i];
		size_t pos2 = xmlfile.find_last_of(L'.');
		size_t begin2 = 0;
		int64_t end2 = -1;
		for (int i = int(pos2) - 1; i >= 0; i--)
		{
			if (iswdigit(xmlfile[i]) && end2 == -1)
				end2 = i;
			if (!iswdigit(xmlfile[i]) && end2 != -1)
			{
				begin2 = i;
				break;
			}
		}
		if (end2 == -1)
			continue;
		wstring filenum = xmlfile.substr(begin2+1, m_digits);
		int fnum = stoi(filenum);
		wstringstream wss;
		wss << outfilename << setfill(L'0') << setw(m_digits) << fnum << L".lbl";
		m_lbl_list.push_back(wss.str());
	}
}

glm::vec3 ReadVector(std::string &str)
{
	glm::vec3 vector;
	int c = 0;
	stringstream ss(str);
	while (1)
	{
		double v;
		ss >> v;
		if (!ss)
			break;
		if (c == 0)
			vector.x = v;
		else if (c == 1)
			vector.y = v;
		else if (c == 2)
			vector.z = v;
		c++;
	}
	return vector;
}

glm::mat3 ReadMatrix(std::string &str)
{
	glm::mat3 matrix;
	int cx = 0;
	int cy = 0;
	stringstream ss(str);
	while (1)
	{
		double v;
		ss >> v;
		if (!ss)
			break;
		matrix[cx][cy] = v;
		cx++;
		if (cx == 3)
		{
			cx = 0;
			cy++;
		}
	}
	return matrix;
}

double Gaussian(glm::vec3 x, glm::vec3 m, glm::mat3 s)
{
	glm::vec3 x1 = x - m;
	glm::vec3 x2 = s * x1;
	double p = glm::dot(x1, x2);
	return exp(-p);
}

void AddLabel(int num, wxXmlNode* node, unsigned int* label_data)
{
	unsigned long ival;
	double dval;
	wxString strItem;

	//id
	if (!node->GetAttribute("id", &strItem))
		return;
	strItem.ToULong(&ival);
	unsigned int id = ival + 1;//nonzero
	//parent
	if (!node->GetAttribute("parent", &strItem))
		return;
	strItem.ToULong(&ival);
	unsigned int prev_id = ival + 1;
	//splitScore
	if (!node->GetAttribute("splitScore", &strItem))
		return;
	strItem.ToDouble(&dval);
	//double uncertainty = 5.0 - dval;
	//scale
	if (!node->GetAttribute("scale", &strItem))
		return;
    auto item = strItem.ToStdString();
	glm::vec3 scale = ReadVector(item);
	//centroid
	if (!node->GetAttribute("m", &strItem))
		return;
    item = strItem.ToStdString();
	glm::vec3 centroid = ReadVector(item);
	//corr
	if (!node->GetAttribute("W", &strItem))
		return;
    item = strItem.ToStdString();
	glm::mat3 corr = ReadMatrix(item);

	//fill label
	unsigned int cell_size = 0;
	for (int i=0; i<nx; ++i)
	for (int j=0; j<ny; ++j)
	for (int k=0; k<nz; ++k)
	{
		glm::vec3 x = scale * glm::vec3(i, j, k);
		glm::vec3 m = centroid * scale;
		glm::mat3 scale_mat;
		scale_mat[0][0] = 1/scale.x / scale.x;
		scale_mat[1][1] = 1/scale.y / scale.y;
		scale_mat[2][2] = 1/scale.z / scale.z;
		glm::mat3 s = corr * scale_mat;
		double g = Gaussian(x, m, s);
		if (g > 0.93)
		{
			unsigned long long index = nx*ny*k + nx*j + i;
			label_data[index] = id;
			cell_size++;
		}
	}
	if (!cell_size)
		return;
	//add to track map
	FL::pCell cell = FL::pCell(new FL::Cell(id));
	FLIVR::Point center(centroid.x, centroid.y, centroid.z);
	cell->SetCenter(center);
	cell->SetSizeUi(cell_size);
	cell->SetSizeF(cell_size);
	FL::CellListIter iter;
	m_tm_processor.AddCell(cell, num, iter);
	if (prev_id)
	{
		//link
		FL::CellList list1, list2;
		list1.insert(pair<unsigned int, FL::pCell>
			(id, cell));
		FL::pCell cell2 = FL::pCell(new FL::Cell(prev_id));
		list2.insert(pair<unsigned int, FL::pCell>
			(prev_id, cell2));
		m_tm_processor.LinkCells(list1, list2, num, num-1, false);
	}
}

void ProcessXml(int num)
{
	wxXmlDocument doc;
	wxString xmlfile = m_xml_list[num];
	if (!doc.Load(xmlfile))
		return;
	wxXmlNode *root = doc.GetRoot();
	if (!root || root->GetName() != "document")
		return;

	Nrrd* nrrd_label = nrrdNew();
	unsigned long long mem_size = (unsigned long long)nx*
		(unsigned long long)ny*(unsigned long long)nz;
	unsigned int * data_label = new (std::nothrow) unsigned int[mem_size];
	memset(data_label, 0, sizeof(unsigned int)*mem_size);
	wxXmlNode *child = root->GetChildren();
	while (child)
	{
		if (child->GetName() == "GaussianMixtureModel")
		{
			AddLabel(num, child, data_label);
		}
		child = child->GetNext();
	}

	//save nrrd
	nrrdWrap(nrrd_label, data_label, nrrdTypeUInt, 3, (size_t)nx, (size_t)ny, (size_t)nz);
	nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSpacing, 1.0, 1.0, 1.0);
	nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMin, 0.0, 0.0, 0.0);
	nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoMax, double(nx), double(ny), double(nz));
	nrrdAxisInfoSet(nrrd_label, nrrdAxisInfoSize, (size_t)nx, (size_t)ny, (size_t)nz);
	MSKWriter writer;
	writer.SetData(nrrd_label);
	wstring outfile = m_lbl_list[num];
	writer.Save(outfile, 1);

	delete[] data_label;
	nrrdNix(nrrd_label);
}

int main(int argc, char* argv[])
{
	if (argc < 7)
	{
		printf("Wrong arguments.\n");
		return 1;
	}

	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring in_filename = converter.from_bytes(argv[1]);
	std::wstring out_filename = converter.from_bytes(argv[2]);
	nx = atoi(argv[3]);
	ny = atoi(argv[4]);
	nz = atoi(argv[5]);
	std::string trackfile = argv[6];

	ProcessInputName(in_filename);
	ProcessOutputName(out_filename);

	for (int i = 0; i < m_xml_list.size(); ++i)
		ProcessXml(i);

	//save trackmap
	m_tm_processor.Export(trackfile);

	printf("All done. Quit.\n");

	return 0;
}