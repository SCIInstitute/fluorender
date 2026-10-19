/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
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

#include <PropertyPanelAgent.h>
#include <PropertyPanel.h>
#include <Global.h>
#include <Tokens.h>
#include <MainFrameAgent.h>
#include <Coordinator.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <MeshData.h>
#include <AnnotData.h>

PropertyPanelAgent::PropertyPanelAgent(
	PropertyPanel* dlg) :
	Agent(dlg)
{

}

void PropertyPanelAgent::UpdateUI(const UpdateRequest& request)
{
	auto panel = GetPanel();
	if (!panel)
		return;

	if (request.HasValue(gstPropertyPageCreate))
		CreatePage();
	else if (request.HasValue(gstPropertyPageDelete))
		DeletePage();

	if (request.HasValue(gstPropertyPageShow))
		ShowPage();
	else if (request.HasValue(gstPropertyPageHide))
		HidePage();

	if (request.HasValue(gstPropertyPageRename))
		RenamePage();
}

void PropertyPanelAgent::UpdateData(const UpdateRequest& request)
{
}

PropertyPanel* PropertyPanelAgent::GetPanel() const
{
	return static_cast<PropertyPanel*>(GetOwner());
}

std::optional<PageKey>
PropertyPanelAgent::GetSelectedPageKey() const
{
	auto& curr = glbin_current;

	switch (curr.GetType())
	{
	case 2:
	{
		auto vd = curr.vol_data.lock();
		if (!vd)
			return std::nullopt;

		return PageKey{
			PropPageType::Volume,
			vd->GetName()
		};
	}

	case 3:
	{
		auto md = curr.mesh_data.lock();
		if (!md)
			return std::nullopt;

		return PageKey{
			PropPageType::Mesh,
			md->GetName()
		};
	}

	case 4:
	{
		auto ad = curr.ann_data.lock();
		if (!ad)
			return std::nullopt;

		return PageKey{
			PropPageType::Annotation,
			ad->GetName()
		};
	}
	}

	return std::nullopt;
}

PageEntry*
PropertyPanelAgent::FindPage(const PageKey& key)
{
	auto it = std::find_if(
		m_pages.begin(),
		m_pages.end(),
		[&](const PageEntry& entry)
		{
			return entry.key == key;
		});

	return it == m_pages.end() ?
		nullptr :
		&(*it);
}

void PropertyPanelAgent::CreatePage()
{
	auto key = GetSelectedPageKey();
	if (!key)
		return;

	auto page = FindPage(*key);

	if (page)
	{
		ShowPage();
		return;
	}

	PageEntry entry;

	entry.key = *key;
	entry.agent = CreateChildAgent(*key);

	m_pages.push_back(std::move(entry));

	ShowPage();
}

void PropertyPanelAgent::ShowPage()
{
	auto key = GetSelectedPageKey();
	if (!key)
		return;

	auto page = FindPage(*key);
	if (!page)
		return;

	if (!page->visible)
	{
		page->visible = true;

		GetPanel()->ShowPage(page->agent);
	}
	else
	{
		GetPanel()->SelectPage(page->agent);
	}
}

void PropertyPanelAgent::HidePage()
{
	auto key = GetSelectedPageKey();
	if (!key)
		return;

	auto page = FindPage(*key);
	if (!page)
		return;

	page->visible = false;

	GetPanel()->HidePage(page->agent);
}

void PropertyPanelAgent::DeletePage()
{
	auto key = GetSelectedPageKey();
	if (!key)
		return;

	auto page = FindPage(*key);
	if (!page)
		return;

	GetPanel()->DeletePage(page->agent);

	DeleteChildAgent(page->agent);

	//...
}

void PropertyPanelAgent::RenamePage()
{
	for (auto& page : m_pages)
	{
		std::wstring name =
			GetCurrentPageName(page);

		page.key.name = name;

		GetPanel()->RenamePage(
			page.agent,
			name);
	}
}