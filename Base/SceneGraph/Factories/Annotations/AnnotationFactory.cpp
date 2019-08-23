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

#include "AnnotationFactory.hpp"

using namespace fluo;

AnnotationFactory::AnnotationFactory()
{
	m_name = "annotation factory";
	default_object_name_ = "default annotations";
}

AnnotationFactory::~AnnotationFactory()
{

}

void AnnotationFactory::createDefault()
{
	if (!getDefault())
	{
		Annotations* ad = new Annotations();
		ad->setName(default_object_name_);
		objects_.push_back(ad);

		//add default values here
		ad->addValue("volume", (Referenced*)0);
		ad->addValue("transform", FLTYPE::Transform());
		ad->addValue("display", bool(true));
		ad->addValue("memo", std::string());
		ad->addValue("memo ro", bool(true));//memo is read only
		ad->addValue("data path", std::wstring());
		ad->addValue("info meaning", std::wstring());
	}
}

Annotations* AnnotationFactory::build(Annotations* ann)
{
	unsigned int default_id = 0;
	return clone(default_id);
}

Annotations* AnnotationFactory::clone(Annotations* ad)
{
	incCounter();

	Object* new_ad = ad->clone(CopyOp::DEEP_COPY_ALL);
	new_ad->setId(global_id_);
	std::string name = "annotations" + std::to_string(local_id_);
	new_ad->setName(name);

	objects_.push_back(new_ad);

	return dynamic_cast<Annotations*>(new_ad);
}

Annotations* AnnotationFactory::clone(const unsigned int id)
{
	Object* object = find(id);
	if (object)
	{
		Annotations* ad = dynamic_cast<Annotations*>(object);
		if (ad)
			return clone(ad);
	}
	return 0;
}

