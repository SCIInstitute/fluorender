#include "tests.h"
#include "asserts.h"

#include <Group.hpp>
#include <InfoVisitor.hpp>

using namespace fluo;

//define test classes
//Outline of inheritance: Object◁-Node◁-VolumeData
//Object base class stores values and handles value change events
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
			updateValue("value2", dval + 1, event);
	}
};

//Outline of inheritance: Object◁-Node◁-Group◁-VolumeGroup
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
	obj1->addRefValue("object", (Referenced*)(0));
	auto names = obj1->getValueNames();

	//allocate memory for a group and add object as its child
	MyVolumeGroup* group = new MyVolumeGroup();
	//use a referenced pointer to manage memory
	ref_ptr<MyVolumeGroup> ref_group(group);
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
	std::cout << "Initialization:" << std::endl;
	group->accept(visitor);
	std::cout << "Press Enter to continue." << std::endl; std::cin.get();

	//test begins here
	/*Level 1: sync values between objects*/				std::cout << "Level 1: sync values between objects" << std::endl;
	obj1->syncAllValues(obj2);								std::cout << "obj1->syncAllValues(obj2);" << std::endl;
	obj1->syncAllValues(obj3);								std::cout << "obj1->syncAllValues(obj3);" << std::endl;
	obj1->setValue("value1", 1.0);							std::cout << "obj1->setValue(\"value1\", 1.0);" << std::endl;
	obj1->setValue("value2", 2.0);							std::cout << "obj1->setValue(\"value2\", 2.0);" << std::endl;
	obj1->setValue("value3", 3.0);							std::cout << "obj1->setValue(\"value3\", 3.0);" << std::endl;
	obj1->setRefValue("object", obj1);							std::cout << "obj1->setValue(\"object\", obj1);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl; std::cin.get();

	/*Level 2: sync values within one object*/				std::cout << "Level 2: sync values within one object" << std::endl;
	obj2->syncValues("value2", "value3");					std::cout << "obj2->syncValues(\"value2\", \"value3\");" << std::endl;
	obj3->syncValues("value3", "value2");					std::cout << "obj3->syncValues(\"value3\", \"value2\");" << std::endl;
	obj1->setValue("value1", 4.0);							std::cout << "obj1->setValue(\"value1\", 4.0);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl;	std::cin.get();

	/*Level 3: unsync values*/								std::cout << "Level 3: unsync values" << std::endl;
	obj1->unsyncAllValues(obj2);							std::cout << "obj1->unsyncAllValues(obj2);" << std::endl;
	obj1->unsyncAllValues(obj3);							std::cout << "obj1->unsyncAllValues(obj3);" << std::endl;
	obj1->unsyncValues({ "value1", "value2", "value3" });	std::cout << "obj1->unsyncValues({ \"value1\", \"value2\", \"value3\" });" << std::endl;
	obj1->setValue("value1", 5.0);							std::cout << "obj1->setValue(\"value1\", 5.0);" << std::endl;
	obj1->setValue("value3", 7.0);							std::cout << "obj1->setValue(\"value3\", 7.0);" << std::endl;
	obj2->setValue("value1", 8.0);							std::cout << "obj2->setValue(\"value1\", 8.0);" << std::endl;
	obj3->setValue("value2", 9.0);							std::cout << "obj3->setValue(\"value2\", 9.0);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl;	std::cin.get();

	/*Level 4: sync values among objects in a group*/		std::cout << "Level 4: sync values among objects in a group" << std::endl;
	obj1->syncAllValues(group);								std::cout << "obj1->syncAllValues(group);" << std::endl;
	obj2->syncAllValues(group);								std::cout << "obj2->syncAllValues(group);" << std::endl;
	obj3->syncAllValues(group);								std::cout << "obj3->syncAllValues(group);" << std::endl;
	group->syncAllValues(obj1);								std::cout << "group->syncAllValues(obj1);" << std::endl;
	group->syncAllValues(obj2);								std::cout << "group->syncAllValues(obj2);" << std::endl;
	group->syncAllValues(obj3);								std::cout << "group->syncAllValues(obj3);" << std::endl;
	obj1->setValue("value1", 11.0);							std::cout << "obj1->setValue(\"value1\", 11.0);" << std::endl;
	obj3->setValue("value3", 10.0);							std::cout << "obj1->setValue(\"value3\", 10.0);" << std::endl;
	obj1->setRefValue("object", obj2);							std::cout << "obj1->setValue(\"object\", obj2);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl;	std::cin.get();

	/*Level 5: a bonus for going so far*/					std::cout << "Level 5: a bonus for going so far" << std::endl;
	obj2->setRefValue("object", obj1);							std::cout << "obj2->setValue(\"object\", obj1);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl;	std::cin.get();

	/*Level 6: child removal*/								std::cout << "Level 6: child removal" << std::endl;
	group->removeChild(obj1);								std::cout << "group->removeChild(obj1);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl;	std::cin.get();

	/*Level 7: directly set a self-referenstd::cing value!*/		std::cout << "Level 7: directly set a self-referenstd::cing value!" << std::endl;
	obj3->setRefValue("object", obj3);							std::cout << "obj3->setValue(\"object\", obj3);" << std::endl;
	obj3->setValue("value1", 12.0);							std::cout << "obj3->setValue(\"value1\", 12.0);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);									std::cout << "Press Enter to continue." << std::endl;	std::cin.get();

	/*Level 8: self-referenced child removal*/				std::cout << "Level 8: self-referenced child removal" << std::endl;
	group->removeChild(obj3);								std::cout << "group->removeChild(obj3);" << std::endl;
	/*Results:*/											std::cout << "Results:" << std::endl;
	group->accept(visitor);
	//All done. Quit after press Enter.
	//hopefully it doesn't crash and no memory leak is detected
}

