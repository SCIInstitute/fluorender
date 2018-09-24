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
#ifndef _TREEMODEL_H_
#define _TREEMODEL_H_

#include <wx/dataview.h>
#include <Flobject/Observer.h>
#include <Scenegraph/Node.h>
#include <Scenegraph/NodeVisitor.h>

namespace FUI
{
	class TreeUpdater;
	class TreeModel : public wxDataViewModel, public FL::Observer
	{
	public:
		TreeModel();
		~TreeModel();

		//observer functions
		virtual void objectDeleted(void*);
		virtual void objectChanging(void*, void* orig_node, const std::string &exp);
		virtual void objectChanged(void*, void* orig_node, const std::string &exp);
		//scenegraph events
		virtual void nodeAdded(void*, void* parent, void* child);
		virtual void nodeRemoved(void*, void* parent, void* child);

		int Compare(const wxDataViewItem &item1, const wxDataViewItem &item2,
			unsigned int column, bool ascending) const override;

		virtual unsigned int GetColumnCount() const override;

		virtual wxString GetColumnType(unsigned int col) const override;

		virtual void GetValue(wxVariant &variant,
			const wxDataViewItem &item, unsigned int col) const override;

		virtual bool SetValue(const wxVariant &variant,
			const wxDataViewItem &item, unsigned int col) override;

		virtual bool IsEnabled(const wxDataViewItem &item,
			unsigned int col) const override;

		virtual wxDataViewItem GetParent(const wxDataViewItem &item) const override;

		virtual bool IsContainer(const wxDataViewItem &item) const override;

		virtual unsigned int GetChildren(const wxDataViewItem &parent,
			wxDataViewItemArray &array) const override;

		void SetRoot(FL::Node* root)
		{
			m_root = root;
			root->addObserver(this);
		}
		FL::Node* GetRoot() { return m_root; }

		friend class TreeUpdater;

	private:
		FL::Node *m_root;//also observes root
	};

}

#endif//_TREEMODEL_H_