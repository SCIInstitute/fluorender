/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include <PyDlc.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <compatibility.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <iomanip>
#include <ctime>
#include <algorithm>
#if defined(_WIN32) || defined (_DARWIN)
#include <hdf5.h>
#endif

using namespace flrd;

PyDlc::PyDlc() :
	m_nx(1280),
	m_ny(720)
{

}

PyDlc::~PyDlc()
{
}

void PyDlc::LoadDlc()
{
	Run(ot_Run_SimpleString,
		"import deeplabcut");
	//disable tqdm
	Run(ot_Run_SimpleString,
		"from tqdm import tqdm\n"\
		"from functools import partialmethod\n"\
		"tqdm.__init__ = partialmethod(tqdm.__init__, disable=True)");
}

void PyDlc::AnalyzeVideo()
{
	std::string cmd =
		"deeplabcut.analyze_videos(";
	cmd += "\"" + ws2s(m_config_file_py) + "\", ";
	cmd += "\"" + ws2s(m_video_file_py) + "\", ";
	cmd += "robust_nframes=True,";
	cmd += "save_as_csv=True)";
	Run(ot_Run_SimpleStringEx,
		cmd);
}

bool PyDlc::GetResultFile()
{
	if (m_state)
		return false;//busy

	std::filesystem::path p = m_video_file;
	m_result_file = p.replace_extension().wstring();
	std::filesystem::path path = p.parent_path();
	if (!std::filesystem::exists(path))
		return false;//doesn't exist
	m_result_file += L"*.csv";
	std::wregex rgx = REGEX(m_result_file);
	for (auto& it : std::filesystem::directory_iterator(path))
	{
		if (!std::filesystem::is_regular_file(it))
			continue;
		const std::wstring str = it.path().wstring();
		if (std::regex_match(str, rgx))
		{
			m_result_file = str;
			break;
		}
	}
	return std::filesystem::exists(m_result_file);
}

int PyDlc::GetDecodeErrorCount()
{
	int c = 0;
	if (m_output.empty())
		return c;

	std::string target("UserWarning: Could not decode frame");
	std::string::size_type pos = 0;
	while ((pos = m_output.find(target, pos)) != std::string::npos)
	{
		++c;
		pos += target.length();
	}

	return c;
}

bool PyDlc::AddRulers(RulerHandler* rhdl, size_t toff)
{
	if (!rhdl)
		return false;
#ifdef _WIN32
	std::ifstream f(m_result_file);
#else
    std::ifstream f(ws2s(m_result_file));
#endif
	if (!f.good())
		return false;

	bool has_toff = false;
	std::string line;
	bool start = false;
	std::vector<std::string> props;
	std::vector<std::string> names;
	std::vector<int> sn;
	std::vector<Ruler*> rlist;
	int ln = 0;
	while (std::getline(f, line))
	{
		std::istringstream s(line);
		std::string item;
		std::vector<std::string> entry;
		while (std::getline(s >> std::ws, item, ','))
			entry.push_back(item);

		if (entry[0] == "time_offset")
		{
			has_toff = true;
			toff = STOI(entry[1]);
			continue;
		}

		bool all_float = true;
		for (auto& i : entry)
			all_float = all_float && isFloat(i);
		if (!start)
		{
			if (all_float)
			{
				//add rulers
				start = true;
				std::vector<fluo::Point> points;
				getPoints(entry, props, points);
				size_t t = STOI(entry[0]) + toff;
				size_t c = 0;
				Ruler* r = 0;
				int rst;
				for (auto& i : points)
				{
					rst = 0;
					if (c < sn.size())
						rst = sn[c];
					if (rst < 1)
					{
						r = rhdl->AddRuler(i, t);
						if (c < names.size())
							r->SetName(s2ws(names[c]));
						r->SetRulerMode(rst == -2 ? flrd::RulerMode::Locator : flrd::RulerMode::Polyline);
						rlist.push_back(r);
					}
					else if (r)
						r->AddPoint(i);

					c++;
				}
			}
			else
			{
				//get other info
				if (ln == 1)
					getNames(entry, names, sn);
				else if (ln == 2)
					props = entry;
			}
		}
		else
		{
			//add points
			if (all_float)
			{
				std::vector<fluo::Point> points;
				getPoints(entry, props, points);
				size_t t = STOI(entry[0]) + toff;
				size_t c = 0, ri = 0, rpi = 0;
				Ruler* r = 0;
				int rst;
				for (auto& i : points)
				{
					rst = 0;
					if (c < sn.size())
						rst = sn[c];
					if (rst < 1)
					{
						if (ri < rlist.size())
							r = rlist[ri];
						ri++;
						rpi = 0;
					}
					if (r)
					{
						r->SetWorkTime(t);
						r->SetPoint(static_cast<int>(rpi), i);
						rpi++;
					}

					c++;
				}
			}
		}

		ln++;
	}
	f.close();

	if (toff > 0 && !has_toff)
	{
		//append info to the head
#ifdef _WIN32
		std::ifstream inf(m_result_file);
#else
        std::ifstream inf(ws2s(m_result_file));
#endif
		if (!inf.good())
			return true;
		inf.seekg(0, inf.end);
		std::streamsize size = inf.tellg();
		inf.seekg(0, inf.beg);
		std::vector<char> buffer(size);
		inf.read(buffer.data(), size);
		inf.close();
		std::ofstream outf;
		OutputStreamOpen(outf, ws2s(m_result_file));
		if (!outf.good())
			return true;
		outf << "time_offset," << toff << std::endl;
		std::copy(std::begin(buffer), std::end(buffer),
			std::ostream_iterator<char>(outf, ""));
		outf.close();
	}
	return true;
}

//training
std::string PyDlc::GetTrainCmd(int maxiters)
{
	std::string cmd;
	int displayiters = 1000;
	maxiters *= displayiters;
	int saveiters = maxiters;
	//start training
#ifdef _WIN32
	cmd = "python -c \"";
#else
	cmd = "python3 -c \"";
#endif
	cmd += "print('START TRAINING WITH DEEPLABCUT. FLUORENDER CAN BE CLOSED.')\n";
	cmd += "import deeplabcut\n";
	cmd += "deeplabcut.create_training_dataset(\\\"";
	cmd += ws2s(m_config_file_py);
	cmd += "\\\", augmenter_type='imgaug')\n";
	cmd += "deeplabcut.train_network(\\\"";
	cmd += ws2s(m_config_file_py);
	cmd += "\\\", ";
	cmd += "displayiters=" + std::to_string(displayiters) + ", ";
	cmd += "saveiters=" + std::to_string(saveiters) + ", ";
	cmd += "maxiters=" + std::to_string(maxiters) + ")\n";
	cmd += "wait=input('DONE. QUIT.')\"";

	return cmd;
}

void PyDlc::Train(int maxiters)
{
	std::string cmd;
	int displayiters = 1000;
	maxiters *= displayiters;
	int saveiters = maxiters;

	cmd += "deeplabcut.create_training_dataset(";
	cmd += "\"" + ws2s(m_config_file_py) + "\", ";
	cmd += "augmenter_type='imgaug')\n";
	cmd += "deeplabcut.train_network(";
	cmd += "\"" + ws2s(m_config_file_py) + "\", ";
	cmd += "displayiters=" + std::to_string(displayiters) + ", ";
	cmd += "saveiters=" + std::to_string(saveiters) + ", ";
	cmd += "maxiters=" + std::to_string(maxiters) + ")\n";

	Run(ot_Run_SimpleStringEx,
		cmd);
}

void PyDlc::CreateConfigFile(
	const std::wstring& prj_name,
	const std::wstring& usr_name,
	RulerHandler* rhdl)
{
	std::filesystem::path p = m_config_file;
	p = p.parent_path();
	if (!std::filesystem::exists(p))
	{
		try
		{
			std::filesystem::create_directory(p);
		}
		catch (...)
		{
			return;
		}
	}
	if (!std::filesystem::exists(p))
		return;

	m_prj_name = prj_name;
	m_usr_name = usr_name;

	std::ofstream cf;
	OutputStreamOpen(cf, ws2s(m_config_file));

	cf << "    # Project definitions (do not edit)" << std::endl;
	cf << "Task: " << ws2s(m_prj_name) << std::endl;
	cf << "scorer: " << ws2s(m_usr_name) << std::endl;
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	cf << "date: " << std::put_time(&tm, "%b%d") << std::endl;
	cf << "multianimalproject: false" << std::endl;
	cf << "identity:" << std::endl;
	cf << std::endl;
	cf << "    # Project path (change when moving around)" << std::endl;
	cf << "project_path: " << p.string() << std::endl;
	cf << std::endl;
	cf << "    # Annotation data set configuration(and individual video cropping parameters)" << std::endl;
	cf << "video_sets:" << std::endl;
	cf << "  " << ws2s(m_video_file) << ":" << std::endl;
	cf << "    crop: 0, " << m_nx << ", 0, " << m_ny << std::endl;
	cf << "bodyparts:" << std::endl;
	if (rhdl)
	{
		std::string str = ws2s(rhdl->PrintRulers(false));
		cf << str;
	}
	cf << std::endl;
	cf << "    # Fraction of video to start/stop when extracting frames for labeling/refinement" << std::endl;
	cf << "start: " << double(m_start_fn) / double(m_frame_num - 1) << std::endl;
	cf << "stop: " << double(m_end_fn) / double(m_frame_num - 1) << std::endl;
	int key_num = 20;
	if (rhdl)
	{
		std::set<size_t> keys;
		if (!rhdl->GetKeyFrames(keys))
			return;
		key_num = static_cast<int>(keys.size());
	}
	cf << "numframes2pick: " << key_num << std::endl;
	cf << std::endl;
	cf << "    # Plotting configuration" << std::endl;
	cf << "skeleton:" << std::endl;
	if (rhdl)
	{
		std::string str = ws2s(rhdl->PrintRulers(true));
		cf << str;
	}
	cf << "skeleton_color: black" << std::endl;
	cf << "pcutoff: 0.6" << std::endl;
	cf << "dotsize: 12" << std::endl;
	cf << "alphavalue: 0.7" << std::endl;
	cf << "colormap: rainbow" << std::endl;
	cf << std::endl;
	cf << "    # Training,Evaluation and Analysis configuration" << std::endl;
	cf << "TrainingFraction:" << std::endl;
	cf << "- 0.95" << std::endl;
	cf << "iteration: 0" << std::endl;
	cf << "default_net_type: resnet_50" << std::endl;
	cf << "default_augmenter: default" << std::endl;
	cf << "snapshotindex: -1" << std::endl;
	cf << "batch_size: 8" << std::endl;
	cf << std::endl;
	cf << "    # Cropping Parameters (for analysis and outlier frame detection)" << std::endl;
	cf << "cropping: false" << std::endl;
	cf << "    #if cropping is true for analysis, then set the values here:" << std::endl;
	cf << "x1: 0" << std::endl;
	cf << "x2: " << m_nx << std::endl;
	cf << "y1: 0" << std::endl;
	cf << "y2: " << m_ny << std::endl;
	cf << std::endl;
	cf << "    # Refinement configuration (parameters from annotation dataset configuration also relevant in this stage)" << std::endl;
	cf << "corner2move2:" << std::endl;
	cf << "- 50" << std::endl;
	cf << "- 50" << std::endl;
	cf << "move2corner: true" << std::endl;

	cf.close();

	//create dirs
	std::filesystem::path child_path;
	child_path = p;
	child_path.append("dlc-models");
	std::filesystem::create_directory(child_path);
	child_path = p;
	child_path.append("labeled-data");
	std::filesystem::create_directory(child_path);
	child_path.append(m_prj_name);
	std::filesystem::create_directory(child_path);//put extracted frames here
	m_label_path = child_path.wstring();
	child_path = p;
	child_path.append("training-datasets");
	std::filesystem::create_directory(child_path);
	child_path = p;
	child_path.append("videos");
	std::filesystem::create_directory(child_path);
}

#if defined(_WIN32) || defined(_DARWIN)
void PyDlc::WriteHDF(RulerHandler* rhdl)
{
	if (!rhdl)
		return;
	if (m_label_path.empty() ||
		m_usr_name.empty() ||
		m_prj_name.empty())
		return;
	//get keyframes
	std::set<size_t> keys;
	if (!rhdl->GetKeyFrames(keys))
		return;
	size_t kn = keys.size();
	//get ruler point num
	size_t rpn = rhdl->GetRulerPointNum();
	size_t rpn2 = rpn * 2;

	//array data
	std::vector<char> cvals;

	//open a file
	std::filesystem::path p(m_label_path);
	p /= "CollectedData_" + ws2s(m_usr_name) + ".h5";
	std::string fn = p.string();
	hid_t file_id = H5Fcreate(fn.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	//attributes
	hdf_write_attr_utf(file_id, "CLASS", u8"GROUP");
	hdf_write_attr_utf(file_id, "PYTABLES_FORMAT_VERSION", u8"2.1");
	hdf_write_attr_utf(file_id, "TITLE", u8"");
	hdf_write_attr_utf(file_id, "VERSION", u8"1.0");

	//create keypoints group
	hid_t group_id = H5Gcreate2(file_id, "/keypoints", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//attributes
	//class
	hdf_write_attr_utf(group_id, "CLASS", u8"GROUP");
	//title
	hdf_write_attr_utf(group_id, "TITLE", u8"");
	//version
	hdf_write_attr_utf(group_id, "VERSION", u8"1.0");
	//axis0_nlevels
	hdf_write_attr_int(group_id, "axis0_nlevels", 3);
	//axis0_variety
	hdf_write_attr_utf(group_id, "axis0_variety", u8"multi");
	//axis1_nlevels
	hdf_write_attr_int(group_id, "axis1_nlevels", 3);
	//axis1_variety
	hdf_write_attr_utf(group_id, "axis1_variety", u8"multi");
	//block0_items_nlevels
	hdf_write_attr_int(group_id, "block0_items_nlevels", 3);
	//block0_items_variety
	hdf_write_attr_utf(group_id, "block0_items_variety", u8"multi");
	//encoding
	hdf_write_attr_utf(group_id, "encoding", u8"UTF-8");
	//errors
	hdf_write_attr_utf(group_id, "errors", u8"strict");
	//nblocks
	hdf_write_attr_int(group_id, "nblocks", 1);
	//ndim
	hdf_write_attr_int(group_id, "ndim", 2);
	//pandas_type
	hdf_write_attr_utf(group_id, "pandas_type", u8"frame");
	//pandas_version
	hdf_write_attr_utf(group_id, "pandas_version", u8"0.15.2");

	std::vector<int> vals(rpn2, 0);
	std::vector<std::string> strs;
	std::vector<double> coords;
	//axis0
	//label0
	hdf_write_array(group_id, "axis0_label0", vals);
	//label1
	for (size_t i = 0; i < rpn; ++i)
		vals[i * 2] = vals[i * 2 + 1] = int(i);
	hdf_write_array(group_id, "axis0_label1", vals);
	//label2
	for (size_t i = 0; i < rpn; ++i)
	{
		vals[i * 2] = 0;
		vals[i * 2 + 1] = 1;
	}
	hdf_write_array(group_id, "axis0_label2", vals);
	//level0
	strs = {ws2s(m_usr_name)};
	hdf_write_array_str(group_id, "axis0_level0",
		"axis0_namescorer", u8"scorer", strs);
	//level1
	rhdl->GetRulerPointNames(strs);
	hdf_write_array_str(group_id, "axis0_level1",
		"axis0_namebodyparts", u8"bodyparts", strs);
	//level2
	strs = { "x", "y" };
	hdf_write_array_str(group_id, "axis0_level2",
		"axis0_namecoords", u8"coords", strs);
	//axis1
	//label0
	vals = std::vector<int>(kn, 0);
	hdf_write_array(group_id, "axis1_label0", vals);
	//label1
	hdf_write_array(group_id, "axis1_label1", vals);
	//label2
	for (size_t i = 0; i < kn; ++i)
		vals[i] = int(i);
	hdf_write_array(group_id, "axis1_label2", vals);
	//level0
	strs = { "labeled-data" };
	hdf_write_array_str(group_id, "axis1_level0",
		"axis1_nameNone", u8"N.", strs);
	//level1
	strs = { ws2s(m_prj_name) };
	hdf_write_array_str(group_id, "axis1_level1",
		"axis1_nameNone", u8"N.", strs);
	//level2
	strs.clear();
	for (auto i : keys)
	{
		std::ostringstream oss;
		oss << std::setw(m_fn_length) << std::setfill('0') << i;
		std::string str = "img" + oss.str() + ".tif";
		strs.push_back(str);
	}
	hdf_write_array_str(group_id, "axis1_level2",
		"axis1_nameNone", u8"N.", strs);
	//block0_items
	//label0
	vals = std::vector<int>(rpn2, 0);
	hdf_write_array(group_id, "block0_items_label0", vals);
	//label1
	for (size_t i = 0; i < rpn; ++i)
		vals[i * 2] = vals[i * 2 + 1] = int(i);
	hdf_write_array(group_id, "block0_items_label1", vals);
	//label2
	for (size_t i = 0; i < rpn; ++i)
	{
		vals[i * 2] = 0;
		vals[i * 2 + 1] = 1;
	}
	hdf_write_array(group_id, "block0_items_label2", vals);
	//level0
	strs = { ws2s(m_usr_name) };
	hdf_write_array_str(group_id, "block0_items_level0",
		"block0_items_namescorer", u8"scorer", strs);
	//level1
	rhdl->GetRulerPointNames(strs);
	hdf_write_array_str(group_id, "block0_items_level1",
		"block0_items_namebodyparts", u8"bodyparts", strs);
	//level2
	strs = { "x", "y" };
	hdf_write_array_str(group_id, "block0_items_level2",
		"block0_items_namecoords", u8"coords", strs);
	//values
	rhdl->GetRulerPointCoords(coords);
	hdf_write_array2_double(group_id, "block0_values",
		static_cast<int>(kn), static_cast<int>(rpn2), coords);

	//close group
	herr_t status = H5Gclose(group_id);
	// Close the file
	status = H5Fclose(file_id);
}

bool PyDlc::hdf_write_attr_b8(hid_t item, const std::string& name, char cval)
{
	bool r = true;
	herr_t status;
	hid_t aidi = H5Screate(H5S_SCALAR);
	hid_t attr_id = H5Acreate(item, name.c_str(), H5T_STD_B8LE, aidi, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Awrite(attr_id, H5T_STD_B8LE, &cval); r = r && (status >= 0);
	status = H5Aclose(attr_id); r = r && (status >= 0);
	status = H5Sclose(aidi); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_attr_int(hid_t item, const std::string& name, int ival)
{
	bool r = true;
	herr_t status;
	hid_t aidi = H5Screate(H5S_SCALAR);
	hid_t attr_id = H5Acreate(item, name.c_str(), H5T_NATIVE_INT, aidi, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Awrite(attr_id, H5T_NATIVE_INT, &ival); r = r && (status >= 0);
	status = H5Aclose(attr_id); r = r && (status >= 0);
	status = H5Sclose(aidi); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_attr_utf(hid_t item, const std::string& name, const std::u8string& str)
{
	bool r = true;
	herr_t status;
	hid_t aids = H5Screate(H5S_SCALAR);
	hid_t astype = H5Tcopy(H5T_C_S1);
	H5Tset_cset(astype, H5T_CSET_UTF8);
	H5Tset_size(astype, str.size());
	hid_t attr_id = H5Acreate(item, name.c_str(), astype, aids, H5P_DEFAULT, H5P_DEFAULT);
	status = H5Awrite(attr_id, astype, str.c_str()); r = r && (status >= 0);
	status = H5Aclose(attr_id); r = r && (status >= 0);
	status = H5Sclose(aids); r = r && (status >= 0);
	status = H5Tclose(astype); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_array(hid_t group, const std::string& name, const std::vector<int>& vals)
{
	auto mm = std::minmax_element(vals.begin(), vals.end());
	int minval = *mm.first;
	int maxval = *mm.second;
	if (maxval < 128 && minval > -129)
	{
		std::vector<char> cvals;
		for (auto& i : vals)
			cvals.push_back(char(i));
		return hdf_write_array_char(group, name, cvals);
	}
	else if (maxval < 32768 && minval > -32769)
	{
		std::vector<short> svals;
		for (auto& i : vals)
			svals.push_back(short(i));
		return hdf_write_array_short(group, name, svals);
	}
	else
		return hdf_write_array_int(group, name, vals);
	return false;
}

bool PyDlc::hdf_write_array_char(hid_t group, const std::string& name, const std::vector<char>& vals)
{
	bool r = true;
	herr_t status;
	hsize_t dims1[1];
	dims1[0] = vals.size();
	hid_t dspace_id = H5Screate_simple(1, dims1, NULL);
	hid_t data_id = H5Dcreate2(group, name.c_str(), H5T_STD_I8LE, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//attrs
	//class
	hdf_write_attr_utf(data_id, "CLASS", u8"ARRAY");
	//flavor
	hdf_write_attr_utf(data_id, "FLAVOR", u8"numpy");
	//title
	hdf_write_attr_utf(data_id, "TITLE", u8"");
	//version
	hdf_write_attr_utf(data_id, "VERSION", u8"2.4");
	//transposed
	hdf_write_attr_b8(data_id, "transposed", 1);

	status = H5Dwrite(data_id, H5T_NATIVE_CHAR, H5S_ALL, H5S_ALL, H5P_DEFAULT, vals.data()); r = r && (status >= 0);
	status = H5Dclose(data_id); r = r && (status >= 0);
	status = H5Sclose(dspace_id); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_array_short(hid_t group, const std::string& name, const std::vector<short>& vals)
{
	bool r = true;
	herr_t status;
	hsize_t dims1[1];
	dims1[0] = vals.size();
	hid_t dspace_id = H5Screate_simple(1, dims1, NULL);
	hid_t data_id = H5Dcreate2(group, name.c_str(), H5T_STD_I16LE, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//attrs
	//class
	hdf_write_attr_utf(data_id, "CLASS", u8"ARRAY");
	//flavor
	hdf_write_attr_utf(data_id, "FLAVOR", u8"numpy");
	//title
	hdf_write_attr_utf(data_id, "TITLE", u8"");
	//version
	hdf_write_attr_utf(data_id, "VERSION", u8"2.4");
	//transposed
	hdf_write_attr_b8(data_id, "transposed", 1);

	status = H5Dwrite(data_id, H5T_NATIVE_SHORT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vals.data()); r = r && (status >= 0);
	status = H5Dclose(data_id); r = r && (status >= 0);
	status = H5Sclose(dspace_id); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_array_int(hid_t group, const std::string& name, const std::vector<int>& vals)
{
	bool r = true;
	herr_t status;
	hsize_t dims1[1];
	dims1[0] = vals.size();
	hid_t dspace_id = H5Screate_simple(1, dims1, NULL);
	hid_t data_id = H5Dcreate2(group, name.c_str(), H5T_STD_I32LE, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//attrs
	//class
	hdf_write_attr_utf(data_id, "CLASS", u8"ARRAY");
	//flavor
	hdf_write_attr_utf(data_id, "FLAVOR", u8"numpy");
	//title
	hdf_write_attr_utf(data_id, "TITLE", u8"");
	//version
	hdf_write_attr_utf(data_id, "VERSION", u8"2.4");
	//transposed
	hdf_write_attr_b8(data_id, "transposed", 1);

	status = H5Dwrite(data_id, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, vals.data()); r = r && (status >= 0);
	status = H5Dclose(data_id); r = r && (status >= 0);
	status = H5Sclose(dspace_id); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_array_str(hid_t group,
	const std::string& name,
	const std::string& str1,
	const std::u8string& str2,
	const std::vector<std::string>& vals)
{
	bool r = true;
	herr_t status;
	hsize_t dims1[1];
	dims1[0] = vals.size();
	hid_t dspace_id = H5Screate_simple(1, dims1, NULL);
	hid_t astype = H5Tcopy(H5T_C_S1);
	H5Tset_cset(astype, H5T_CSET_ASCII);
	size_t mx = 0;
	for (auto i : vals)
		mx = std::max(mx, i.size());
	H5Tset_size(astype, mx);
	hid_t data_id = H5Dcreate2(group, name.c_str(), astype, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//attrs
	//class
	hdf_write_attr_utf(data_id, "CLASS", u8"ARRAY");
	//flavor
	hdf_write_attr_utf(data_id, "FLAVOR", u8"numpy");
	//title
	hdf_write_attr_utf(data_id, "TITLE", u8"");
	//version
	hdf_write_attr_utf(data_id, "VERSION", u8"2.4");
	//str1
	hdf_write_attr_utf(data_id, str1, str2);
	//kind
	hdf_write_attr_utf(data_id, "kind", u8"string");
	//name
	hdf_write_attr_utf(data_id, "name", str2);
	//transposed
	hdf_write_attr_b8(data_id, "transposed", 1);

	char* raw = new char[mx*vals.size()]();
	size_t c = 0;
	for (auto i : vals)
	{
		memcpy(raw + c, i.c_str(), i.size());
		c += mx;
	}
	status = H5Dwrite(data_id, astype, H5S_ALL, H5S_ALL, H5P_DEFAULT, raw); r = r && (status >= 0);
	delete[] raw;
	status = H5Dclose(data_id); r = r && (status >= 0);
	status = H5Sclose(dspace_id); r = r && (status >= 0);
	status = H5Tclose(astype); r = r && (status >= 0);
	return r;
}

bool PyDlc::hdf_write_array2_double(hid_t group, const std::string& name,
	int nx, int ny, const std::vector<double>& vals)
{
	bool r = true;
	herr_t status;
	hsize_t dims2[2];
	dims2[0] = nx;
	dims2[1] = ny;
	hid_t dspace_id = H5Screate_simple(2, dims2, NULL);
	hid_t data_id = H5Dcreate2(group, name.c_str(), H5T_IEEE_F64LE, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	//attrs
	//class
	hdf_write_attr_utf(data_id, "CLASS", u8"ARRAY");
	//flavor
	hdf_write_attr_utf(data_id, "FLAVOR", u8"numpy");
	//title
	hdf_write_attr_utf(data_id, "TITLE", u8"");
	//version
	hdf_write_attr_utf(data_id, "VERSION", u8"2.4");
	//transposed
	hdf_write_attr_b8(data_id, "transposed", 1);

	status = H5Dwrite(data_id, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, vals.data()); r = r && (status >= 0);
	status = H5Dclose(data_id); r = r && (status >= 0);
	status = H5Sclose(dspace_id); r = r && (status >= 0);
	return r;
}
#else
void PyDlc::WriteHDF(RulerHandler* rhdl)
{}

#endif
