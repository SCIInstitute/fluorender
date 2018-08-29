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
}