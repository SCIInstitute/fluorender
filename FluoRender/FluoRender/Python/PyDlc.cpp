/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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
#include <Distance/RulerHandler.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <iomanip>
#include <ctime>

using namespace flrd;

PyDlc::PyDlc()
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
	cmd += "\"" + m_config_file_py + "\", ";
	cmd += "\"" + m_video_file_py + "\", ";
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
	m_result_file = p.replace_extension().string();
	std::filesystem::path path = p.parent_path();
	if (!std::filesystem::exists(path))
		return false;//doesn't exist
	m_result_file += "*.csv";
	std::regex rgx = REGEX(m_result_file);
	for (auto& it : std::filesystem::directory_iterator(path))
	{
		if (!std::filesystem::is_regular_file(it))
			continue;
		const std::string str = it.path().string();
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
	std::ifstream f(m_result_file);
	if (!f.good())
		return false;

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
		while (std::getline(s, item, ','))
			entry.push_back(item);

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
				size_t t = std::stoi(entry[0]) + toff;
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
							r->SetName(names[c]);
						r->SetRulerType(rst == -2 ? 2 : 1);
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
				size_t t = std::stoi(entry[0]) + toff;
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
						r->SetPoint(rpi, i);
						rpi++;
					}

					c++;
				}
			}
		}

		ln++;
	}

	return true;
}

//training
void PyDlc::CreateConfigFile(
	const std::string& prj_name,
	const std::string& usr_name,
	RulerHandler* rhdl)
{
	std::filesystem::path p = m_config_file;
	p = p.parent_path();
	if (!std::filesystem::exists(p))
		std::filesystem::create_directory(p);
	if (!std::filesystem::exists(p))
		return;

	std::ofstream cf;
	cf.open(m_config_file, std::ofstream::out);

	cf << "    # Project definitions (do not edit)" << std::endl;
	cf << "Task: " << prj_name << std::endl;
	cf << "scorer: " << usr_name << std::endl;
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
	cf << "  " << m_video_file << ":" << std::endl;
	cf << "    crop: 0, 1280, 0, 720" << std::endl;
	cf << "bodyparts:" << std::endl;
	if (rhdl)
	{
		std::string str = rhdl->PrintRulers(false);
		cf << str;
	}
	cf << std::endl;
	cf << "    # Fraction of video to start/stop when extracting frames for labeling/refinement" << std::endl;
	cf << "start: 0" << std::endl;
	cf << "stop: 1" << std::endl;
	cf << "numframes2pick: 20" << std::endl;
	cf << std::endl;
	cf << "    # Plotting configuration" << std::endl;
	cf << "skeleton:" << std::endl;
	if (rhdl)
	{
		std::string str = rhdl->PrintRulers(true);
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
	cf << "x2: 640" << std::endl;
	cf << "y1: 277" << std::endl;
	cf << "y2: 624" << std::endl;
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
	child_path.append(prj_name);
	std::filesystem::create_directory(child_path);
	child_path = p;
	child_path.append("training-datasets");
	std::filesystem::create_directory(child_path);
	child_path = p;
	child_path.append("videos");
	std::filesystem::create_directory(child_path);
}

void PyDlc::Train()
{

}