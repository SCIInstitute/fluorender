#include <stdio.h>
#include <vld.h>
#include "asserts.h"

#include <Flobject/Object.h>

using namespace std;
using namespace FL;

void Test1()
{
	//creating first object
	Object *obj1 = new Object();
	//declaring a list of objects
	vector<ref_ptr<Object>> obj_list;
	//add the first object to the list
	obj_list.push_back(obj1);
	ASSERT_EQ(1, obj_list.size());
/*	ASSERT_TRUE(obj_list[0]->isSameKindAs(obj1));
	ASSERT_STREQ("Object", obj_list[0]->className());
	unsigned int id = 10;
	obj_list[0]->setId(id);
	ASSERT_EQ(id, obj_list[0]->getId());

	bool bval = false;
	//adding a boolean value to the first object
	ASSERT_TRUE(obj_list[0]->addValue("bval", bval));
	bool bval2;
	ASSERT_TRUE(obj_list[0]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);
	bval = true;
	//changing the boolean value to the first object
	ASSERT_TRUE(obj_list[0]->setValue("bval", bval));
	ASSERT_TRUE(obj_list[0]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);

	//creating a second object by cloning the first one
	Object* obj2 = obj_list[0]->clone();
	//adding the second object to the list
	obj_list.push_back(obj2);
	ASSERT_EQ(2, obj_list.size());
	//confirming the values of the second object are the same as the first
	ASSERT_TRUE(obj_list[1]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);

	//sync the boolean value
	ASSERT_TRUE(obj_list[0]->syncValue("bval", (obj_list[1]).get()));
	ASSERT_TRUE(obj_list[1]->syncValue("bval", (obj_list[0]).get()));
	bval = false;
	ASSERT_TRUE(obj_list[0]->setValue("bval", bval));
	ASSERT_TRUE(obj_list[1]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);
	bval = true;
	ASSERT_TRUE(obj_list[1]->setValue("bval", bval));
	ASSERT_TRUE(obj_list[0]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);

	//adding the first object as a value to the second
	ASSERT_TRUE(obj_list[1]->addValue("friend", obj_list[0].get()));
	Object* obj;
	//confirming the value has been added
	ASSERT_TRUE(obj_list[1]->getValue("friend", (Referenced**)&obj));
	ASSERT_EQ(obj1, obj);
	//erasing the first object
	obj_list.erase(obj_list.begin());
	//confirming the value pointing to the first object is reset
	ASSERT_TRUE(obj_list[0]->getValue("friend", (Referenced**)&obj));
	ASSERT_EQ(0, obj);
*/
}

int main(int argc, char* argv[])
{
	Test1();

	printf("All done. Quit.\n");

	return 0;
}