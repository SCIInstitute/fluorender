#include "tests.h"
#include "asserts.h"
#include <Script/Camera2Ruler.h>

void OpenCVTest0()
{
	flrd::Camera2Ruler c2r;
	c2r.SetList(1, "E:\\DATA\\Holly\\MouseTracking\\27May2019_F_P105_GAD2-GCaMP5G_01L04.vrp");
	c2r.SetList(2, "E:\\DATA\\Holly\\MouseTracking\\27May2019_F_P105_GAD2-GCaMP5G_01R04.vrp");
	c2r.Run();
	flrd::RulerList* list = c2r.GetResult();
}

