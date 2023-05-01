#include <stdio.h>
//#include <vld.h>
#include "asserts.h"
#include "tests.h"
#include "reformat_data.h"

using namespace std;

class VRenderApp
{};
VRenderApp dummy;
VRenderApp& wxGetApp()
{
	return dummy;
}

int main(int argc, char* argv[])
{
	//char* leak = new char[1000];
	//memset(leak, 14, 1000);

	//ObjectTest();
	//ObjectTest2();
	//ObjectTest3();
	//ObjectTest4();

	//GroupTest();
	//GroupTest2();
	//GroupTest3();
	//GroupTest4();

	//SpecialValueTest();

	//MyTest();

	//reformat_data();

	//TableTest();

	//PythonTest1(argv[1], argv[2]);

	//PythonTest2(argv[1], argv[2]);

	//PythonTest0();

	//OpenCVTest0();

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
	}

	cout << "All done. Quit." << endl;
	cin.get();
	return 0;
}