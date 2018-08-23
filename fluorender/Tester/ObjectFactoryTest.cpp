#include "tests.h"
#include "asserts.h"
#include <vector>
#include <Flobject/Object.h>
#include <Flobject/ObjectFactory.h>

using namespace std;
using namespace FL;

void FactoryTest()
{
	ref_ptr<ObjectFactory> factory(new ObjectFactory());

	int num = 10;
	for (int i = 0; i < num; ++i)
	{
		Object* obj = factory->build();
	}

	ASSERT_EQ(num, factory->getNum());
}