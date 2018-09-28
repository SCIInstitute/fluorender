/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#include <Scenegraph/VolumeFactory.h>
#include <Scenegraph/VolumeGroup.h>

using namespace FL;

VolumeFactory::VolumeFactory()
{
	m_name = "volume factory";
	default_object_name_ = "default volume";

	addValue("current", (VolumeData*)(0));//current volume data
}

VolumeFactory::~VolumeFactory()
{

}

void VolumeFactory::createDefault()
{
	if (!getDefault())
	{
		VolumeData* vd = new VolumeData();
		vd->setName(default_object_name_);
		objects_.push_front(vd);

		//add default values here
		//output (2d) adjustments
		vd->addValue("gamma r", double(1));
		vd->addValue("gamma g", double(1));
		vd->addValue("gamma b", double(1));
		vd->addValue("brightness r", double(1));
		vd->addValue("brightness g", double(1));
		vd->addValue("brightness b", double(1));
		vd->addValue("equalize r", double(0));
		vd->addValue("equalize g", double(0));
		vd->addValue("equalize b", double(0));

		vd->addValue("bounds", FLTYPE::BBox());
		vd->addValue("clip planes", FLTYPE::PlaneSet(6));
		vd->addValue("clip bounds", FLTYPE::BBox());

		vd->addValue("tex path", std::wstring());//path to original file
		vd->addValue("channel", long(0));//channel index of the original file
		vd->addValue("time", long(0));//time index of the original file

		//modes
		//blend mode
		vd->addValue("blend mode", long(0));//0: ignore; 1: layered; 2: depth; 3: composite
		//mip
		vd->addValue("mip mode", long(0));//0-normal; 1-MIP; 2-white shading; 3-white mip
		vd->addValue("saved mode", long(0));//save the mode for restoring
		vd->addValue("stream mode", long(0));//0-normal; 1-MIP; 2-shading; 3-shadow, 4-mask
		vd->addValue("mask mode", long(0));//0-normal, 1-render with mask, 2-render with mask excluded,
											//3-random color with label, 4-random color with label+mask
		vd->addValue("use mask thresh", bool(false));// use mask threshold
		vd->addValue("mask thresh", double(0));//mask threshold
		vd->addValue("invert", bool(false));//invert intensity values

		//volume properties
		vd->addValue("soft thresh", double(0));//soft threshold
		vd->addValue("int scale", double(1));//scaling factor for intensity values
		vd->addValue("gm scale", double(1));//scaling factor for gradient magnitude values
		vd->addValue("max int", double(255));//max intensity value from integer reading
		vd->addValue("gamma 3d", double(1));
		vd->addValue("extract boundary", double(0));//previously called gm thresh
		vd->addValue("saturation", double(1));//previously called offset, low offset
		vd->addValue("low threshold", double(0));
		vd->addValue("high threshold", double(1));
		vd->addValue("alpha", double(1));
		vd->addValue("alpha enable", bool(true));
		vd->addValue("mat amb", double(1));//materials
		vd->addValue("mat diff", double(1));
		vd->addValue("mat spec", double(1));
		vd->addValue("mat shine", double(10));
		vd->addValue("noise redct", bool(false));//noise reduction
		vd->addValue("shading enable", bool(false));//shading
		vd->addValue("low shading", double(0));//low shading
		vd->addValue("high shading", double(0));//highg shading
		vd->addValue("shadow enable", bool(false));//shadow
		vd->addValue("shadow int", double(0));
		vd->addValue("sample rate", double(1));//sample rate
		//color
		vd->addValue("color", FLTYPE::Color(1.0));
		vd->addValue("hsv", FLTYPE::HSVColor(FLTYPE::Color(1.0)));
		vd->addValue("luminance", double(1.0));
		vd->addValue("sec color", FLTYPE::Color(1.0));//secondary color
		vd->addValue("sec color set", bool(false));

		//resolution
		vd->addValue("res x", long(0));
		vd->addValue("res y", long(0));
		vd->addValue("res z", long(0));
		vd->addValue("base res x", long(0));
		vd->addValue("base res y", long(0));
		vd->addValue("base res z", long(0));
		//scaling
		vd->addValue("scale x", double(1));
		vd->addValue("scale y", double(1));
		vd->addValue("scale z", double(1));
		//spacing
		vd->addValue("spc x", double(1));
		vd->addValue("spc y", double(1));
		vd->addValue("spc z", double(3));
		vd->addValue("spc from file", bool(false));//if spacing value are from original file, otherwise use default settings
		//added for multires data
		vd->addValue("base spc x", double(1));
		vd->addValue("base spc y", double(1));
		vd->addValue("base spc z", double(3));
		vd->addValue("spc scl x", double(1));
		vd->addValue("spc scl y", double(1));
		vd->addValue("spc scl z", double(1));

		//display control
		vd->addValue("bits", long(8));//bits
		vd->addValue("display", bool(true));
		vd->addValue("draw bounds", bool(false));
		vd->addValue("test wire", bool(false));

		//color map settings
		vd->addValue("colormap enable", bool(false));
		vd->addValue("colormap mode", long(0));
		vd->addValue("colormap type", long(0));
		vd->addValue("colormap low", double(0));
		vd->addValue("colormap high", double(1));
		vd->addValue("colormap proj", long(0));

		//texture ids will be removed later
		vd->addValue("2d mask id", (unsigned long)(0));//2d mask texture for segmentation
		//2d weight map for segmentation
		vd->addValue("2d weight1 id", (unsigned long)(0));//after tone mapping
		vd->addValue("2d weight2 id", (unsigned long)(0));//before tone mapping
		vd->addValue("2d dmap id", (unsigned long)(0));//2d depth map texture for rendering shadows

		//clip distance
		vd->addValue("clip dist x", long(0));
		vd->addValue("clip dist y", long(0));
		vd->addValue("clip dist z", long(0));

		//compression
		vd->addValue("compression", bool(false));

		//resize
		vd->addValue("resize", bool(false));
		vd->addValue("resize x", long(0));
		vd->addValue("resize y", long(0));
		vd->addValue("resize z", long(0));

		//brisk skipping
		vd->addValue("skip brick", bool(false));

		//shown in legend
		vd->addValue("legend", bool(true));

		//interpolate
		vd->addValue("interpolate", bool(true));

		//depth attenuation, also called fog previously
		vd->addValue("depth atten", bool(false));
		vd->addValue("da int", double(0.5));
		vd->addValue("da start", double(0));
		vd->addValue("da end", double(1));

		//valid brick number
		vd->addValue("brick num", long(0));

		//estimate threshold
		vd->addValue("estimate thresh", double(0));

		//parameters not in original class but passed to renderer
		vd->addValue("viewport", FLTYPE::GLint4());//viewport
		vd->addValue("clear color", FLTYPE::GLfloat4());//clear color
		vd->addValue("cur framebuffer", (unsigned long)(0));//current framebuffer

		//multires level
		vd->addValue("multires", bool(false));
		vd->addValue("level", long(0));
		vd->addValue("level num", long(1));

	}
}

VolumeData* VolumeFactory::build(const std::string &exp)
{
	unsigned int default_id = 0;
	return clone(default_id);
}

VolumeData* VolumeFactory::clone(VolumeData* vd)
{
	if (!vd)
		return 0;

	incCounter();

	Object* new_vd = vd->clone(CopyOp::DEEP_COPY_ALL);
	new_vd->setId(global_id_);
	std::string name = "volume" + std::to_string(local_id_);
	new_vd->setName(name);

	objects_.push_front(new_vd);
	setValue("current", new_vd);

	//notify observers
	notifyObserversNodeAdded(this, new_vd);

	return dynamic_cast<VolumeData*>(new_vd);
}

VolumeData* VolumeFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		VolumeData* vd = dynamic_cast<VolumeData*>(object);
		if (vd)
			return clone(vd);
	}
	return 0;
}

VolumeGroup* VolumeFactory::buildGroup(VolumeData* vd)
{
	VolumeGroup* group;
	if (vd)
		group = new VolumeGroup(*vd, CopyOp::DEEP_COPY_ALL);//shallow copy might be ok
	else
		group = new VolumeGroup(*getDefault(), CopyOp::DEEP_COPY_ALL);
	if (group)
		group->setName("Group");
	return group;
}