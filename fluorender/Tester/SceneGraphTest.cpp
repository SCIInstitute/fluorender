#include "tests.h"
#include "asserts.h"
#include "SceneGraphTest.h"

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

	ann1->setValue("volume", data);
	VolumeData* vd;
	ann1->getValue("volume", (Referenced**)&vd);
	ASSERT_EQ(data, vd);

	data->addValue("annotations", ann1);

	data->setValue("spacing x", 2.0);
	data->setValue("spacing y", 2.0);
	data->setValue("spacing z", 6.0);

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
}