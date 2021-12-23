/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2021 Scientific Computing and Imaging Institute,
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

#ifndef VOLUMEGROUP_HPP
#define VOLUMEGROUP_HPP

#include <Group.hpp>
#include <Nrrd/nrrd.h>

namespace fluo
{
	class VolumeFactory;
	class VolumeData;
	class VolumeGroup : public Group
	{
	public:
		VolumeGroup();
		VolumeGroup(const VolumeGroup& group, const CopyOp& copyop = CopyOp::SHALLOW_COPY);
		VolumeGroup(const VolumeData& vd, const CopyOp& copyop = CopyOp::SHALLOW_COPY);

		virtual Object* clone(const CopyOp& copyop) const
		{
			return new VolumeGroup(*this, copyop);
		}

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const VolumeGroup*>(obj) != NULL;
		}

		virtual const char* className() const { return "VolumeGroup"; }

		virtual VolumeGroup* asVolumeGroup() { return this; }
		virtual const VolumeGroup* asVolumeGroup() const { return this; }

		//manage value syncs
		virtual bool insertChild(size_t index, Node* child);
		virtual bool setChild(size_t i, Node* node);
		virtual bool removeChildren(size_t pos, size_t num);

		void AddMask(Nrrd* mask, int op);//op: 0-replace; 1-union; 2-exclude; 3-intersect

		friend class VolumeFactory;

	protected:
		virtual ~VolumeGroup();

	private:
		void OnRandomizeColor(Event& event);//randomize color
	};
}

#endif//_VOLUMEGROUP_H_
