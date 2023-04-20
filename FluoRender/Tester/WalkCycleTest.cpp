#include "tests.h"
#include "asserts.h"
#include <Distance/WalkCycle.h>
#include <filesystem>

void WalkCycleTest(const std::string& file, int l, int r)
{
	flrd::WalkCycle wc;
	wc.ReadData(file);
	flrd::Window win(l, r);
	size_t width = win.w > 2 ? win.w - 2 : 1;
	size_t left = win.l > 1 ? win.l - 2 : 0;
	std::vector<double> corrs;
	flrd::Window maxwin;
	for (size_t i = left; i < left + 5; ++i)
		for (size_t j = width; j < width + 5; ++j)
		{
			wc.Reset();
			win = flrd::Window(i, i + j - 1);
			wc.SetInitWin(win);
			wc.LoadCycle();
			wc.Extract();
			double c = wc.GetNormalizedCorr();
			corrs.push_back(c);
			if (c == *std::max_element(corrs.begin(), corrs.end()))
				maxwin = win;
		}

	wc.Reset();
	wc.SetInitWin(maxwin);
	wc.LoadCycle();
	wc.Extract();

	//save
	std::filesystem::path p(file);
	p.remove_filename();
	std::string str = p.string();
	str += "cycle.csv";
	wc.SaveCycle(str);
}