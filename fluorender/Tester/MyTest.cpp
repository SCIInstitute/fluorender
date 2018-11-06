#include "tests.h"
#include "asserts.h"
#include <vector>
#include <Scenegraph/Group.h>
#include <Scenegraph/InfoVisitor.h>

using namespace std;
using namespace FL;

class MyVolumeData : public Node
{
public:
	MyVolumeData() : Node()
	{
		setValueChangedFunction("value1",
			std::bind(&MyVolumeData::OnValue1Changed, this,
				std::placeholders::_1));
	}

	MyVolumeData(const MyVolumeData& data, const CopyOp& copyop, bool copy_values) :
		Node(data, copyop, false)
	{
		if (copy_values)
			copyValues(data, copyop);
	}

protected:
	void OnValue1Changed(Event& event)
	{
		double dval;
		if (getValue("value1", dval))
			setValue("value2", dval + 1);
	}
};

class MyVolumeGroup : public Group
{
public:
	MyVolumeGroup() : Group()
	{}
};

void MyTest()
{
	InfoVisitor visitor;

	MyVolumeData* obj1 = new MyVolumeData();
	obj1->setName("obj1");
	obj1->addValue("value1", double(0));
	obj1->addValue("value2", double(0));
	obj1->addValue("value3", double(0));
	obj1->addValue("object", (Referenced*)(0));

	ref_ptr<MyVolumeGroup> group(new MyVolumeGroup());
	group->setName("group");
	group->copyValues(*obj1, CopyOp::DEEP_COPY_ALL);
	group->addChild(obj1);

	MyVolumeData* obj2 = new MyVolumeData(*obj1, CopyOp::SHALLOW_COPY, true);
	obj2->setName("obj2");
	group->addChild(obj2);

	MyVolumeData* obj3 = new MyVolumeData(*obj2, CopyOp::DEEP_COPY_ALL, true);
	obj3->setName("obj3");
	group->addChild(obj3);
	group->accept(visitor);
	cin.get();

	obj1->syncAllValues(obj2); cout << "obj1->syncAllValues(obj2);" << endl;
	obj1->syncAllValues(obj3); cout << "obj1->syncAllValues(obj3);" << endl;
	obj1->setValue("value1", 1.0); cout << "obj1->setValue(\"value1\", 1.0);" << endl;
	obj1->setValue("value2", 2.0); cout << "obj1->setValue(\"value2\", 2.0);" << endl;
	obj1->setValue("value3", 3.0); cout << "obj1->setValue(\"value3\", 3.0);" << endl;
	obj1->setValue("object", obj1); cout << "obj1->setValue(\"object\", obj1);" << endl;
	group->accept(visitor);
	cin.get();

	obj2->syncValues("value2", "value3"); cout << "obj2->syncValues(\"value2\", \"value3\");" << endl;
	obj3->syncValues("value3", "value2"); cout << "obj3->syncValues(\"value3\", \"value2\");" << endl;
	obj1->setValue("value1", 4.0); cout << "obj1->setValue(\"value1\", 4.0);" << endl;
	group->accept(visitor);
	cin.get();

	obj1->unsyncAllValues(obj2); cout << "obj1->unsyncAllValues(obj2);" << endl;
	obj1->unsyncAllValues(obj3); cout << "obj1->unsyncAllValues(obj3);" << endl;
	std::vector<std::string> names;
	names.push_back("value1");
	names.push_back("value2");
	names.push_back("value3");
	obj1->unsyncValues(names); cout << "obj1->unsyncValues(names);" << endl;
	obj1->setValue("value1", 5.0); cout << "obj1->setValue(\"value1\", 5.0);" << endl;
	obj1->setValue("value3", 7.0); cout << "obj1->setValue(\"value3\", 7.0);" << endl;
	obj2->setValue("value1", 8.0); cout << "obj2->setValue(\"value1\", 8.0);" << endl;
	group->accept(visitor);
	cin.get();

	obj1->syncAllValues(group.get()); cout << "obj1->syncAllValues(group);" << endl;
	obj2->syncAllValues(group.get()); cout << "obj2->syncAllValues(group);" << endl;
	obj3->syncAllValues(group.get()); cout << "obj3->syncAllValues(group);" << endl;
	group->syncAllValues(obj1); cout << "group->syncAllValues(obj1);" << endl;
	group->syncAllValues(obj2); cout << "group->syncAllValues(obj2);" << endl;
	group->syncAllValues(obj3); cout << "group->syncAllValues(obj3);" << endl;
	obj1->setValue("value1", 9.0); cout << "obj1->setValue(\"value1\", 9.0);" << endl;
	obj3->setValue("value3", 11.0); cout << "obj1->setValue(\"value3\", 11.0);" << endl;
	obj1->setValue("object", (Referenced*)obj2); cout << "obj1->setValue(\"object\", obj2);" << endl;
	group->accept(visitor);
	cin.get();

	obj2->setValue("object", (Referenced*)obj1); cout << "obj2->setValue(\"object\", obj1);" << endl;
	group->accept(visitor);
	cin.get();

	group->removeChild(obj1); cout << "group->removeChild(obj1);" << endl;
	group->accept(visitor);
	cin.get();

	obj3->setValue("object", (Referenced*)obj3); cout << "obj3->setValue(\"object\", obj3);" << endl;
	group->accept(visitor);
	cin.get();

	group->removeChild(obj3); cout << "group->removeChild(obj3);" << endl;
	group->accept(visitor);
	cin.get();

}

