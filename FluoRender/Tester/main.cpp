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

	PythonTest(argv[1], argv[2]);

	cout << "All done. Quit." << endl;
	cin.get();
	return 0;
}