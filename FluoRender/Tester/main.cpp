//#include <vld.h>
#include "asserts.h"
#include "tests.h"
#include "dl_test.h"
#include "reformat_data.h"

using namespace std;

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

	//WalkTest(argc, argv);

	DLTest();

	DLTest2();

	return 0;
}