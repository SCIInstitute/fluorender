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
}