/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2022 Scientific Computing and Imaging Institute,
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

#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <MeshData.hpp>
#include <VolumeGroup.hpp>
#include <MeshGroup.hpp>
#include <Annotations.hpp>

using namespace fluo;

Root::Root()
{
	addValue(gstSortValue, std::string("name"));
	addValue(gstSortMethod, long(SortNone));
	addValue(gstActivated, bool(false));
}

bool Root::addChild(Node* child)
{
	bool result = Group::addChild(child);
	//let child know how nodes are sorted for rendering
	if (result)
	{
		ValueCollection names{ gstSortValue, gstSortMethod };
		syncValues(names, child);
	}
	return result;
}

bool Root::insertChild(size_t index, Node* child)
{
	bool result = Group::insertChild(index, child);
	if (result)
	{
		ValueCollection names{ gstSortValue, gstSortMethod };
		syncValues(names, child);
	}
	return result;
}

bool Root::removeChildren(size_t pos, size_t num)
{
	//unsync values
	ValueCollection names{ gstSortValue, gstSortMethod };
	for (size_t i = pos; i < pos + num; ++i)
	{
		if (i < m_children.size())
			unsyncValues(names, m_children[i].get());
	}
	return Group::removeChildren(pos, num);
}

bool Root::setChild(size_t i, Node* node)
{
	//unsync original
	ValueCollection names{ gstSortValue, gstSortMethod };
	if (i < m_children.size())
	{
		Node* origNode = m_children[i].get();
		unsyncValues(names, origNode);
	}
	bool result = Group::setChild(i, node);
	if (result)
		syncValues(names, node);
	return result;
}

//currently highlighted
Renderview* Root::getCurrentRenderview()
{
	Referenced* ref;
	getRefValue(gstCurrentView, &ref);
	return dynamic_cast<Renderview*>(ref);
}

VolumeData* Root::getCurrentVolumeData()
{
	Referenced* ref;
	getRefValue(gstCurrentVolume, &ref);
	return dynamic_cast<VolumeData*>(ref);
}

VolumeGroup* Root::getCurrentVolumeGroup()
{
	Referenced* ref;
	getRefValue(gstCurrentVolumeGroup, &ref);
	return dynamic_cast<VolumeGroup*>(ref);
}

MeshData* Root::getCurrentMeshData()
{
	Referenced* ref;
	getRefValue(gstCurrentMesh, &ref);
	return dynamic_cast<MeshData*>(ref);
}

MeshGroup* Root::getCurrentMeshGroup()
{
	Referenced* ref;
	getRefValue(gstCurrentMeshGroup, &ref);
	return dynamic_cast<MeshGroup*>(ref);
}

Annotations* Root::getCurrentAnnotations()
{
	Referenced* ref;
	getRefValue(gstCurrentAnnotations, &ref);
	return dynamic_cast<Annotations*>(ref);
}
