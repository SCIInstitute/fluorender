#include "tests.h"
#include "asserts.h"
#include "SceneGraphTest.h"
#include <Scenegraph/InfoVisitor.h>
#include <Scenegraph/VolumeGroup.h>
#include <Scenegraph/DecycleVisitor.h>

void SceneGraphTest()
{
	//ref_ptr<VolumeData> data(new VolumeData());
	VolumeDataTest* data = new VolumeDataTest();
	data->setName("data");
	//ref_ptr<Annotations> ann(new Annotations());
	AnnotationsTest* ann1 = new AnnotationsTest();
	ann1->setName("ann1");
	AnnotationsTest* ann2 = new AnnotationsTest();
	ann2->setName("ann2");
	ref_ptr<Group> group(new Group());
	group->addChild(data);
	group->addChild(ann1);
	group->addChild(ann2);

	//data->holdoffObserverNotification();
	ann1->setValue("volume", data);
	VolumeData* vd;
	ann1->getValue("volume", (Referenced**)&vd);
	ASSERT_EQ(data, vd);

	data->addValue("annotations", ann1);

	data->setValue("spacing x", 2.0);
	data->setValue("spacing y", 2.0);
	data->setValue("spacing z", 6.0);

	//data->getValue("spacing x")->holdoffObserverNotification();
	//data->holdoffObserverNotification();
	data->setValue("spacing x", 1.0, false);
	data->setValue("spacing x", 2.0);
	//data->getValue("spacing x")->resumeObserverNotification();
	//data->resumeObserverNotification();

	ann2->setValue("volume", data);
	ann2->getValue("volume", (Referenced**)&vd);
	ASSERT_EQ(data, vd);

	data->setValue("annotations", ann2);

	data->setValue("spacing x", 1.0);
	data->setValue("spacing y", 1.0);
	data->setValue("spacing z", 3.0);

	vd = 0;
	ann1->setValue("volume", vd);
	ann1->getValue("volume", (Referenced**)&vd);
	ASSERT_EQ(0, vd);

	data->setValue("spacing x", 2.0);
	data->setValue("spacing y", 2.0);
	data->setValue("spacing z", 6.0);

	InfoVisitor visitor;
	group->accept(visitor);

	group->removeChild(data);

	group->accept(visitor);
}

void SceneGraphTest2()
{
	std::vector<ref_ptr<Node>> list;
	VolumeGroup* group1 = new VolumeGroup();
	group1->setName("group1");
	VolumeGroup* group2 = new VolumeGroup();
	group2->setName("group2");
	VolumeGroup* group3 = new VolumeGroup();
	group3->setName("group3");
	list.push_back(group1);
	list.push_back(group2);
	list.push_back(group3);

	group1->addChild(group2);
	group2->addChild(group1);
	group1->addChild(group3);

	DecycleVisitor visitor;
	group1->accept(visitor);
	while (visitor.removeCycle()) {}
}
