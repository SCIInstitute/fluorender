#include "tests.h"
#include "asserts.h"
#include <Scenegraph/Group.h>
#include <Scenegraph/InfoVisitor.h>

using namespace std;
using namespace FL;

//define test classes
//in fluorender's code, VolumeData and VolumeGroup are defined and more complex
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
			setValue("value2", dval + 1, event);
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
	//infovisitor is used to print out a tree structure
	InfoVisitor visitor;

	//allocate memory for the first object and add some values to it
	MyVolumeData* obj1 = new MyVolumeData();
	obj1->setName("obj1");
	obj1->addValue("value1", double(0));
	obj1->addValue("value2", double(0));
	obj1->addValue("value3", double(0));
	obj1->addValue("object", (Referenced*)(0));

	//allocate memory for a group and add object as its child
	ref_ptr<MyVolumeGroup> group(new MyVolumeGroup());
	group->setName("group");
	//add some values to group by copying from object
	group->copyValues(*obj1, CopyOp::DEEP_COPY_ALL);
	group->addChild(obj1);

	//allocate memory for the second object by copying the first object
	MyVolumeData* obj2 = new MyVolumeData(*obj1, CopyOp::SHALLOW_COPY, true);
	obj2->setName("obj2");
	//add object to group as a child
	group->addChild(obj2);

	//allocate memory for the third object by copying the second object
	MyVolumeData* obj3 = new MyVolumeData(*obj2, CopyOp::DEEP_COPY_ALL, true);
	obj3->setName("obj3");
	//add object to group as a child
	group->addChild(obj3);
	//now we print out the result
	//it should be one group with 3 children, all values initialized to 0
	cout << "Results:" << endl;
	group->accept(visitor);
	cout << "Press Enter to continue." << endl;
	cin.get();

	//test begins here
	//Level 1: value syncing between objects
	obj1->syncAllValues(obj2);								cout << "obj1->syncAllValues(obj2);" << endl;
	obj1->syncAllValues(obj3);								cout << "obj1->syncAllValues(obj3);" << endl;
	obj1->setValue("value1", 1.0);							cout << "obj1->setValue(\"value1\", 1.0);" << endl;
	obj1->setValue("value2", 2.0);							cout << "obj1->setValue(\"value2\", 2.0);" << endl;
	obj1->setValue("value3", 3.0);							cout << "obj1->setValue(\"value3\", 3.0);" << endl;
	obj1->setValue("object", obj1);							cout << "obj1->setValue(\"object\", obj1);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl; cin.get();

	//Level 2: value syncing within object
	obj2->syncValues("value2", "value3");					cout << "obj2->syncValues(\"value2\", \"value3\");" << endl;
	obj3->syncValues("value3", "value2");					cout << "obj3->syncValues(\"value3\", \"value2\");" << endl;
	obj1->setValue("value1", 4.0);							cout << "obj1->setValue(\"value1\", 4.0);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl;	cin.get();

	//Level 3: unsync values
	obj1->unsyncAllValues(obj2);							cout << "obj1->unsyncAllValues(obj2);" << endl;
	obj1->unsyncAllValues(obj3);							cout << "obj1->unsyncAllValues(obj3);" << endl;
	ValueCollection vc{ "value1", "value2", "value3" };		cout << "ValueCollection vc{ \"value1\", \"value2\", \"value3\" };" << endl;
	obj1->unsyncValues(vc);									cout << "obj1->unsyncValues(vc);" << endl;
	obj1->setValue("value1", 5.0);							cout << "obj1->setValue(\"value1\", 5.0);" << endl;
	obj1->setValue("value3", 7.0);							cout << "obj1->setValue(\"value3\", 7.0);" << endl;
	obj2->setValue("value1", 8.0);							cout << "obj2->setValue(\"value1\", 8.0);" << endl;
	obj3->setValue("value2", 9.0);							cout << "obj3->setValue(\"value2\", 9.0);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl;	cin.get();

	//Level 4: syncing values within a group
	obj1->syncAllValues(group.get());						cout << "obj1->syncAllValues(group);" << endl;
	obj2->syncAllValues(group.get());						cout << "obj2->syncAllValues(group);" << endl;
	obj3->syncAllValues(group.get());						cout << "obj3->syncAllValues(group);" << endl;
	group->syncAllValues(obj1);								cout << "group->syncAllValues(obj1);" << endl;
	group->syncAllValues(obj2);								cout << "group->syncAllValues(obj2);" << endl;
	group->syncAllValues(obj3);								cout << "group->syncAllValues(obj3);" << endl;
	obj1->setValue("value1", 11.0);							cout << "obj1->setValue(\"value1\", 11.0);" << endl;
	obj3->setValue("value3", 10.0);							cout << "obj1->setValue(\"value3\", 10.0);" << endl;
	obj1->setValue("object", (Referenced*)obj2);			cout << "obj1->setValue(\"object\", obj2);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl;	cin.get();

	//Level 5: warming up for next level
	obj2->setValue("object", (Referenced*)obj1);			cout << "obj2->setValue(\"object\", obj1);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl;	cin.get();

	//Level 6: child removal
	group->removeChild(obj1);								cout << "group->removeChild(obj1);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl;	cin.get();

	//Level 7: self referencing value
	obj3->setValue("object", (Referenced*)obj3);			cout << "obj3->setValue(\"object\", obj3);" << endl;
	obj3->setValue("value1", 12.0);							cout << "obj3->setValue(\"value1\", 12.0);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);									cout << "Press Enter to continue." << endl;	cin.get();

	//Level 8: self-referenced object removal
	group->removeChild(obj3);								cout << "group->removeChild(obj3);" << endl;
	/*⇓*/													cout << "Results:" << endl;
	group->accept(visitor);
	//All done. Quit after press Enter.
	//hopefully it doesn't crash and no memory leak is detected
}

