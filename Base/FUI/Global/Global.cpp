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

#include "Global.hpp"
#include "Names.hpp"
#include <Annotations/AnnotationFactory.hpp>
#include <Mesh/MeshFactory.hpp>
#include <Volume/VolumeFactory.hpp>
#include <Base_Agent/AgentFactory.hpp>
#include <Renderer/RendererFactory.hpp>
#include <Renderer/RendererGroupFactory.hpp>
#include <SearchVisitor.hpp>

using namespace fluo;

Global Global::instance_;
Global::Global()
{
	origin_ = ref_ptr<Group>(new Group());
	origin_->setName(flstrOrigin);
	BuildFactories();
}

void Global::BuildFactories()
{
	Group* factory_group = new Group();
	factory_group->setName(flstrFactoryGroup);
	origin_->addChild(factory_group);
	BUILD_AND_ADD(VolumeFactory, factory_group);
	BUILD_AND_ADD(MeshFactory, factory_group);
	BUILD_AND_ADD(AnnotationFactory, factory_group);
	BUILD_AND_ADD(AgentFactory, factory_group);
	BUILD_AND_ADD(RendererFactory, factory_group);
	BUILD_AND_ADD(RendererGroupFactory, factory_group);
}

Object* Global::get(const std::string &name)
{
	SearchVisitor visitor;
	visitor.matchName(name);
	origin_->accept(visitor);
	ObjectList* list = visitor.getResult();
	if (list->empty())
		return 0;
	else
		return (*list)[0];
}