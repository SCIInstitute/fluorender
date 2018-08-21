#include "tests.h"
#include "asserts.h"
#include "Scenegraph/VolumeData.h"

using namespace FL;

void SpecialValueTest()
{
	ref_ptr<VolumeData> vd(new VolumeData());
	FLTYPE::Vector spacing(1.0, 1.0, 3.0);
	vd->addValue("spacing", spacing);
	FLTYPE::Vector spacing2;
	vd->getValue("spacing", spacing2);
	ASSERT_EQ(spacing, spacing2);
	FLTYPE::Point point;
	vd->addValue("position", point);
	FLTYPE::Point point2;
	vd->getValue("position", point2);
	ASSERT_EQ(point, point2);

	ref_ptr<VolumeData> vd2(
		new VolumeData(*vd, CopyOp::DEEP_COPY_ALL));
	vd2->syncValue("spacing", vd.get());
	vd2->syncValue("position", vd.get());

	spacing.Set(10, 10, 30);
	vd2->setValue("spacing", spacing);
	vd->getValue("spacing", spacing2);
	ASSERT_EQ(spacing, spacing2);

	point.Set(100, 100, 100);
	vd2->setValue("position", point);
	vd->getValue("position", point2);
	ASSERT_EQ(point, point2);

	FLTYPE::PlaneSet ps(6);
	vd->addValue("clipping_planes", ps);
	FLTYPE::PlaneSet ps2;
	vd->getValue("clipping_planes", ps2);
	ASSERT_EQ(ps, ps2);
}