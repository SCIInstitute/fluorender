#include <stdio.h>
//#include <vld.h>
#include "asserts.h"
#include "tests.h"
#include "reformat_data.h"

int main(int argc, char* argv[])
{
	//char* leak = new char[1000];
	//memset(leak, 14, 1000);

	//ObjectTest();
	//ObjectTest2();
	//ObjectTest3();
	//ObjectTest4();
	ObjectTest5();

	//GroupTest();
	//GroupTest2();
	//GroupTest3();
	//GroupTest4();

	//SpecialValueTest();

	//MyTest();

	//reformat_data();

	std::cout << "All done. Quit." << std::endl;
	std::cin.get();
	return 0;
}
