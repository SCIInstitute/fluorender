#include <stdio.h>
#include <vld.h>
#include "asserts.h"
#include "tests.h"

using namespace std;

int main(int argc, char* argv[])
{
	//char* leak = new char[1000];
	//memset(leak, 14, 1000);

	ObjectTest();

	GroupTest();

	SceneGraphTest();

	printf("All done. Quit.\n");
	cin.get();
	return 0;
}