#include "tests.h"
#include "asserts.h"
#include <vector>
#include <Scenegraph/VolumeData.h>
#include <Scenegraph/VolumeFactory.h>
#include <Scenegraph/VolumeGroup.h>
#include <Scenegraph/InfoVisitor.h>

using namespace std;
using namespace FL;

void FactoryTest()
{
	ref_ptr<VolumeFactory> factory(new VolumeFactory());
	std::string filename = "E:\\PROJECTS\\fluorender_yexp_sln\\bin\\Debug\\default_volume_settings.xml";
	factory->setValue("default filename", filename);

	ref_ptr<VolumeGroup> group(new VolumeGroup());

	int num = 10;
	for (int i = 0; i < num; ++i)
	{
		VolumeData* vd = factory->build();
		group->addChild(vd);
	}

	ASSERT_EQ(num, factory->getNum());
	ASSERT_EQ(num, group->getNumChildren());

	InfoVisitor visitor;
	group->accept(visitor);

	factory->writeDefault();

	factory->writeDefault(std::cout);

	VolumeData* vd = group->getChild(0)->asVolumeData();
	if (vd)
	{
		long size;
		vd->getValue("resize x", size);
		ASSERT_EQ(0, size);
	}
}

void ObjectTest4()
{
	InfoVisitor visitor;
	VolumeData* obj1 = new VolumeData();
	obj1->setName("obj1");
	ref_ptr<VolumeGroup> group(new VolumeGroup());
	group->setName("group");
	group->addChild(obj1);
	obj1->addValue("value1", double(0));
	obj1->addValue("value2", double(0));
	obj1->addValue("value3", double(0));
	obj1->addValue("object", (Referenced*)(0));
	VolumeData* obj2 = new VolumeData(*obj1, CopyOp::SHALLOW_COPY);
	obj2->setName("obj2");
	group->addChild(obj2);
	VolumeData* obj3 = new VolumeData(*obj2, CopyOp::DEEP_COPY_ALL);
	obj3->setName("obj3");
	group->addChild(obj3);

	obj1->syncAllValues(obj2); cout << "obj1->syncAllValues(obj2);" << endl;
	obj1->syncAllValues(obj3); cout << "obj1->syncAllValues(obj3);" << endl;
	obj1->setValue("value1", 1.0); cout << "obj1->setValue(\"value1\", 1.0);" << endl;
	obj1->setValue("value2", 2.0); cout << "obj1->setValue(\"value2\", 2.0);" << endl;
	obj1->setValue("value3", 3.0); cout << "obj1->setValue(\"value3\", 3.0);" << endl;
	obj1->setValue("object", obj1); cout << "obj1->setValue(\"object\", obj1);" << endl;
	group->accept(visitor);
	cin.get();

	obj1->syncValues("value1", "value2"); cout << "obj1->syncValues(\"value1\", \"value2\");" << endl;
	obj2->syncValues("value3", "value2"); cout << "obj2->syncValues(\"value3\", \"value2\");" << endl;
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
	obj1->setValue("value2", 6.0); cout << "obj1->setValue(\"value2\", 6.0);" << endl;
	obj1->setValue("value3", 7.0); cout << "obj1->setValue(\"value3\", 7.0);" << endl;
	obj2->setValue("value1", 8.0); cout << "obj2->setValue(\"value1\", 8.0);" << endl;
	group->accept(visitor);
	cin.get();

	group->addValue("value1", double(0));
	group->addValue("value2", double(0));
	group->addValue("value3", double(0));
	group->addValue("object", (Referenced*)(0));
	obj1->syncAllValues(group.get()); cout << "obj1->syncAllValues(group);" << endl;
	obj2->syncAllValues(group.get()); cout << "obj2->syncAllValues(group);" << endl;
	obj3->syncAllValues(group.get()); cout << "obj3->syncAllValues(group);" << endl;
	group->syncAllValues(obj1); cout << "group->syncAllValues(obj1);" << endl;
	group->syncAllValues(obj2); cout << "group->syncAllValues(obj2);" << endl;
	group->syncAllValues(obj3); cout << "group->syncAllValues(obj3);" << endl;
	obj1->setValue("value1", 9.0); cout << "obj1->setValue(\"value1\", 9.0);" << endl;
	obj2->setValue("value2", 10.0); cout << "obj1->setValue(\"value2\", 10.0);" << endl;
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

