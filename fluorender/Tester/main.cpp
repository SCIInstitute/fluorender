#include <stdio.h>
#include <vld.h>
#include "asserts.h"
#include "tests.h"

using namespace std;

int main(int argc, char* argv[])
{
	MyTest();

	printf("All done. Quit after press Enter.\n");
	cin.get();
	return 0;
}