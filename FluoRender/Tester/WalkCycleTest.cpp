#include "tests.h"
#include "asserts.h"
//#include <WalkCycle.h>
#include <filesystem>
#include <stdio.h>

void WalkCycleAvg(const std::string& f1,
	const std::string& f2,
	const std::string& fo)
{
	//flrd::WalkCycle wc1, wc2;
	//wc1.ReadData(f1);
	//wc2.ReadData(f2);
	//flrd::WalkData& d1 = wc1.GetData();
	//flrd::WalkData& d2 = wc2.GetData();
	//d1 += d2;
	//d1.div(2);
	//wc1.SaveData(fo);
}

void WalkCycleInit(const std::string& file, int l, int r, size_t ol)
{
	//flrd::WalkCycle wc;
	//wc.ReadData(file);
	//flrd::Window win(l, r);
	//size_t width = win.w > 2 ? win.w - 2 : 1;
	//size_t left = win.l > 1 ? win.l - 2 : 0;
	//std::vector<double> corrs;
	//flrd::Window maxwin;
	//for (size_t i = left; i < left + 5; ++i)
	//	for (size_t j = width; j < width + 5; ++j)
	//	{
	//		wc.Reset();
	//		win = flrd::Window(i, i + j - 1);
	//		wc.SetInitWin(win);
	//		wc.LoadCycle();
	//		wc.Extract(ol);
	//		double c = wc.GetNormalizedCorr();
	//		corrs.push_back(c);
	//		if (c == *std::max_element(corrs.begin(), corrs.end()))
	//			maxwin = win;
	//	}

	//wc.Reset();
	//wc.SetInitWin(maxwin);
	//wc.LoadCycle();
	//wc.Extract(ol);
	////wc.Correct(1);

	////save
	//std::filesystem::path p(file);
	//p.remove_filename();
	//std::string str = p.string();
	//str += "cycle0.csv";
	//wc.SaveCycle(str);
}

void WalkCycleRefine(const std::string& datafile, const std::string& cyclefile, size_t ol)
{
	//flrd::WalkCycle wc;
	//wc.ReadData(datafile);
	//wc.LoadCycle(cyclefile);
	//wc.Extract(ol);
	////wc.Correct(1);
	////save
	//std::filesystem::path p(cyclefile);
	//p.replace_extension();
	//std::string str_base = p.string();
	//for (auto it = str_base.rbegin();
	//	it != str_base.rend(); ++it)
	//{
	//	if (std::isdigit(*it))
	//		str_base.pop_back();
	//	else
	//		break;
	//}
	//int sn = 1;
	//std::string str = str_base + std::to_string(sn) + ".csv";
	//p = std::filesystem::path(str);
	//while (std::filesystem::exists(p))
	//{
	//	sn++;
	//	str = str_base + std::to_string(sn) + ".csv";
	//	p = std::filesystem::path(str);
	//}
	//wc.SaveCycle(str);
}

void WalkCycleCompare(const std::string& datafile, const std::string& cyclefile, size_t ol)
{
	//flrd::WalkCycle wc;
	//wc.ReadData(datafile);
	//wc.LoadCycle(cyclefile);
	//wc.Compare(ol);
	////save
	//std::filesystem::path p(cyclefile);
	//p.remove_filename();
	//p += "compare0";
	//std::string str_base = p.string();
	//for (auto it = str_base.rbegin();
	//	it != str_base.rend(); ++it)
	//{
	//	if (std::isdigit(*it))
	//		str_base.pop_back();
	//	else
	//		break;
	//}
	//int sn = 1;
	//std::string str = str_base + std::to_string(sn) + ".csv";
	//p = std::filesystem::path(str);
	//while (std::filesystem::exists(p))
	//{
	//	sn++;
	//	str = str_base + std::to_string(sn) + ".csv";
	//	p = std::filesystem::path(str);
	//}
	//wc.SaveDist(str);
}

void PhaseGraph(const std::string& infile, const std::string& cyclefile, size_t ol)
{
	//flrd::WalkCycle wc;
	//wc.ReadData(infile);
	//wc.LoadCycle(cyclefile);
	//wc.Align(ol);

	////save
	//std::filesystem::path p(infile);
	//p.replace_extension();
	//std::string outfile = p.string() + "_aligned.csv";
	//wc.SaveAligned(outfile);
}

void ComputeVariance(const std::string& cycle_file, const std::vector<std::string>& sample_list, size_t ol)
{
	//flrd::WalkCycle wc;
	//wc.LoadCycle(cycle_file);
	//int count = 0;
	//double sum2 = 0;
	//for (auto& f : sample_list)
	//{
	//	wc.ReadData(f);
	//	wc.ComputeVar(ol);
	//	count += wc.GetCycleSize();
	//	sum2 += wc.GetDiff2();
	//}
	//if (count > 0)
	//{
	//	double var = sum2 / count;
	//	std::cout << "variance:\t" << var << " in " << count << " cycles" << std::endl;
	//}
}

void ComputeTime(const std::string& cycle_file, const std::vector<std::string>& sample_list, size_t ol, double d)
{
	//flrd::WalkCycle wc;
	//wc.LoadCycle(cycle_file);
	//int count = 0;
	//int cycles = 0;
	//double speed = 0;
	//for (auto& f : sample_list)
	//{
	//	wc.ReadData(f);
	//	wc.ComputeFrames(ol, d);
	//	count += wc.GetFrames();
	//	cycles += wc.GetCycleSize();
	//	speed += wc.GetCycleSpeed();
	//}
	//if (count > 0)
	//{
	//	double t = double(count) / 30.0;
	//	speed = speed / cycles;
	//	std::cout << "time:\t" << t << " in " << cycles << " cycles with speed " << speed << std::endl;
	//}
}

void WalkTest(int argc, char* argv[])
{
	size_t ol = 2;
	int type = std::stoi(argv[1]);
	switch (type)
	{
	case 0:
		WalkCycleInit(argv[2], std::stoi(argv[3]), std::stoi(argv[4]), ol);
		break;
	case 1:
		WalkCycleRefine(argv[2], argv[3], ol);
		break;
	case 2:
		WalkCycleAvg(argv[2], argv[3], argv[4]);
		break;
	case 3:
		WalkCycleCompare(argv[2], argv[3], ol);
		break;
	case 4:
		PhaseGraph(argv[2], argv[3], ol);
		break;
	case 5:
	{
		std::vector<std::string> list;
		for (int i = 3; i < argc; ++i)
			list.push_back(argv[i]);
		ComputeVariance(argv[2], list, ol);
	}
	break;
	case 6:
	{
		std::vector<std::string> list;
		for (int i = 3; i < argc; ++i)
			list.push_back(argv[i]);
		ComputeTime(argv[2], list, ol, 0.05);
	}
	}

	std::cout << "All done. Quit." << std::endl;
	std::cin.get();
}