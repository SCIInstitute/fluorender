#include "tests.h"
#include "asserts.h"
#include <Object.hpp>

using namespace fluo;

void SpecialValueTest()
{
	ref_ptr<Object> vd(new Object());
	fluo::Vector spacing(1.0, 1.0, 3.0);
	vd->addValue("spacing", spacing);
	fluo::Vector spacing2;
	vd->getValue("spacing", spacing2);
	ASSERT_EQ(spacing, spacing2);
	fluo::Point point;
	vd->addValue("position", point);
	fluo::Point point2;
	vd->getValue("position", point2);
	ASSERT_EQ(point, point2);

	ref_ptr<Object> vd2(
		new Object(*vd, CopyOp::DEEP_COPY_ALL));
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

	fluo::PlaneSet ps(6);
	vd->addValue("clipping_planes", ps);
	fluo::PlaneSet ps2;
	vd->getValue("clipping_planes", ps2);
	ASSERT_EQ(ps, ps2);
}