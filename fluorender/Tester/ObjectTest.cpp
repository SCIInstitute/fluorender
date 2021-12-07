#include "tests.h"
#include "asserts.h"
#include <vector>
#include <Flobject/Object.hpp>

using namespace std;
using namespace fluo;

void ObjectTest()
{
	//creating first object
	//use simple pointer so that address can be printed easily
	//also, reference count is set only when it's added to the list
	//ref_ptr<Object> obj1(new Object());
	Object *obj1 = new Object();
	obj1->setName("obj1");
	//declaring a list of objects
	vector<ref_ptr<Object>> obj_list;
	//add the first object to the list
	obj_list.push_back(obj1);
	ASSERT_EQ(1, obj_list.size());
	ASSERT_TRUE(obj_list[0]->isSameKindAs(obj1));
	string str = obj_list[0]->className();
	ASSERT_EQ("Object", str);

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
	Object* obj2 = obj_list[0]->clone(CopyOp::DEEP_COPY_ALL);
	obj2->setName("obj2");
	//adding the second object to the list
	obj_list.push_back(obj2);
	ASSERT_EQ(2, obj_list.size());
	//confirming the values of the second object are the same as the first
	ASSERT_TRUE(obj_list[1]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);

	//sync the boolean value
	//obj1's bval changes --> obj2's bval syncs
	ASSERT_TRUE(obj_list[0]->syncValue("bval", (obj_list[1]).get()));
	bval = false;
	ASSERT_TRUE(obj_list[0]->setValue("bval", bval));
	ASSERT_TRUE(obj_list[1]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);
	bval = true;
	ASSERT_TRUE(obj_list[1]->setValue("bval", bval));
	ASSERT_TRUE(obj_list[0]->getValue("bval", bval2));
	ASSERT_NEQ(bval, bval2);
	//obj2's bval changes --> obj1's bval syncs
	ASSERT_TRUE(obj_list[1]->syncValue("bval", (obj_list[0]).get()));
	bval = false;
	ASSERT_TRUE(obj_list[1]->setValue("bval", bval));
	ASSERT_TRUE(obj_list[0]->getValue("bval", bval2));
	ASSERT_EQ(bval, bval2);

	//adding the first object as a value to the second
	//obj2 is observing obj1
	ASSERT_TRUE(obj_list[1]->addRvalu("friend", obj_list[0].get()));
	//setting the same value fails
	ASSERT_FALSE(obj_list[1]->setRvalu("friend", obj_list[0].get()));
	Referenced* ref;
	//confirming the value has been added
	ASSERT_TRUE(obj_list[1]->getRvalu("friend", &ref));
	Object* obj = dynamic_cast<Object*>(ref);
	ASSERT_EQ(obj1, obj);
	//modify obj2's value
	bval = true;
	ASSERT_TRUE(obj_list[0]->setValue("bval", bval));
	//erasing the first object
	obj_list.erase(obj_list.begin());
	//confirming the value pointing to the first object is reset
	ASSERT_TRUE(obj_list[0]->getRvalu("friend", &ref));
	obj = dynamic_cast<Object*>(ref);
	ASSERT_EQ(0, obj);
}

void ObjectTest2()
{
	Object* obj1 = new Object();
	vector<ref_ptr<Object>> obj_list;
	obj_list.push_back(obj1);
	obj1->addValue("value1", 0.0);
	obj1->addValue("value2", 0.0);
	obj1->addValue("value3", 0.0);
	Object* obj2 = new Object(*obj1, CopyOp::DEEP_COPY_ALL);
	obj_list.push_back(obj2);

	obj1->syncAllValues(obj2);
	obj1->setValue("value1", 1.0);
	double value;
	obj2->getValue("value1", value);
	ASSERT_EQ(1.0, value);
	obj1->setValue("value2", 2.0);
	obj2->getValue("value2", value);
	ASSERT_EQ(2.0, value);
	obj1->unsyncAllValues(obj2);
	obj1->setValue("value1", 3.0);
	obj2->getValue("value1", value);
	ASSERT_EQ(1.0, value);

	ValueCollection names = { "value1", "value2" };
	obj1->syncValues(names, obj2);
	obj1->setValue("value1", 4.0);
	obj2->getValue("value1", value);
	ASSERT_EQ(4.0, value);
	obj1->setValue("value2", 5.0);
	obj2->getValue("value2", value);
	ASSERT_EQ(5.0, value);
	obj1->setValue("value3", 6.0);
	obj2->getValue("value3", value);
	ASSERT_EQ(0.0, value);
	obj1->unsyncAllValues(obj2);

	obj2->setValue("value1", 0.0);
	obj2->setValue("value2", 0.0);
	obj2->setValue("value3", 0.0);
	obj1->setValue("value1", 7.0);
	obj1->propValue("value1", obj2);
	obj2->getValue("value1", value);
	ASSERT_EQ(7.0, value);
	obj1->setValue("value1", 8.0);
	obj1->setValue("value2", 9.0);
	obj1->propValues(names, obj2);
	obj2->getValue("value1", value);
	ASSERT_EQ(8.0, value);
	obj2->getValue("value2", value);
	ASSERT_EQ(9.0, value);
	obj1->setValue("value1", 10.0);
	obj1->setValue("value2", 11.0);
	obj1->setValue("value3", 12.0);
	obj1->propAllValues(obj2);
	obj2->getValue("value1", value);
	ASSERT_EQ(10.0, value);
	obj2->getValue("value2", value);
	ASSERT_EQ(11.0, value);
	obj2->getValue("value3", value);
	ASSERT_EQ(12.0, value);
}

void ObjectTest3()
{
	Object* obj1 = new Object();
	vector<ref_ptr<Object>> obj_list;
	obj_list.push_back(obj1);
	obj1->addValue("value1", 0.0);
	Object* obj2 = new Object(*obj1, CopyOp::DEEP_COPY_ALL);
	obj_list.push_back(obj2);
	Object* obj3 = new Object(*obj2, CopyOp::DEEP_COPY_ALL);
	obj_list.push_back(obj3);

	obj1->syncAllValues(obj2);
	obj2->syncAllValues(obj1);
	//obj3->syncAllValues(obj1);

	bool bval = false;
	obj1->addValue("boolean", bval);
	bool bval2;
	obj1->getValue("boolean", bval2);
	ASSERT_EQ(bval, bval2);
	obj1->toggleValue("boolean", bval2);
	ASSERT_EQ(!bval, bval2);
	obj1->toggleValue("boolean", bval2);
	ASSERT_EQ(bval, bval2);
}

void ObjectTest4()
{
	class TestObject : public Object
	{
	public:
		TestObject()
		{
			m_test = new Object();
			m_test->setName("test object");
		}

		Object* getTest()
		{
			return m_test.get();
		}

	protected:
		virtual ~TestObject() {}

	private:
		ref_ptr<Object> m_test;
	};

	TestObject* obj = new TestObject();
	vector<ref_ptr<Object>> obj_list;
	obj_list.push_back(obj);

	Object* test_object = obj->getTest();
	cout << test_object->getName() << endl;

	ref_ptr<Object> obj2 = obj;

	obj2 = ref_ptr<Object>(new Object());
}