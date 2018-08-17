#include "tests.h"
#include "asserts.h"
#include <Scenegraph/VolumeData.h>
#include <Scenegraph/Annotations.h>

using namespace std;
using namespace FL;

void SceneGraphTest()
{
	ref_ptr<VolumeData> data(new VolumeData());
	ref_ptr<Annotations> ann(new Annotations());
	ann->setValue("volume", data.get());
	VolumeData* vd;
	ann->getValue("volume", (Referenced**)&vd);
	ASSERT_EQ(data, vd);
	data->setValue("spacing x", 2.0);
	//data->setValue("spacing y", 2.0);
	//data->setValue("spacing z", 6.0);
	ref_ptr<Annotations> ann2(new Annotations());
	ann2->setValue("volume", data.get());
	ann2->getValue("volume", (Referenced**)&vd);
	ASSERT_EQ(data, vd);
}