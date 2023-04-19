#include "tests.h"
#include "asserts.h"
#include <Distance/WalkCycle.h>
#include <filesystem>

void WalkCycleTest(const std::string& file, int l, int r)
{
	flrd::WalkCycle wc;
	wc.ReadData(file);
	flrd::Window win(l, r);
	wc.SetInitWin(win);
	wc.LoadCycle();
	wc.Extract();
	//save
	std::filesystem::path p(file);
	p.remove_filename();
	std::string str = p.string();
	str += "cycle.csv";
	wc.SaveCycle(str);
}