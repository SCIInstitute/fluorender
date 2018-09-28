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
#ifndef _LISTMODEL_H_
#define _LISTMODEL_H_

#include <wx/dataview.h>
#include <Fui/InterfaceAgent.h>
#include <Scenegraph/Node.h>

namespace FUI
{
	class ListModel : public wxDataViewVirtualListModel, public InterfaceAgent
	{
	public:
		ListModel();

		int Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
			unsigned int column, bool ascending) const override;

		//model definition
		virtual unsigned int GetColumnCount() const override;

		virtual wxString GetColumnType(unsigned int col) const override;

		virtual void GetValueByRow(wxVariant &vanriant,
			unsigned int row, unsigned int col) const override;

		virtual bool GetAttrByRow(unsigned int row, unsigned int col,
			wxDataViewItemAttr &attr) const override;

		virtual bool SetValueByRow(const wxVariant &variant,
			unsigned int row, unsigned int col) override;

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const ListModel*>(obj) != NULL;
		}

		virtual const char* className() const { return "ListModel"; }

		//observer functions
		virtual void objectDeleted(void*);
		virtual void objectChanging(void*, void* orig_node, const std::string &exp);
		virtual void objectChanged(void*, void* orig_node, const std::string &exp);
		//scenegraph events
		virtual void nodeAdded(void*, void* parent, void* child);
		virtual void nodeRemoved(void*, void* parent, void* child);

		//interface agent functions
		virtual void setObject(FL::Node* root);
		virtual FL::Node* getObject()
		{
			return dynamic_cast<FL::Node*>(InterfaceAgent::getObject());
		}

	private:
	};
}

#endif//_LISTMODEL_H_