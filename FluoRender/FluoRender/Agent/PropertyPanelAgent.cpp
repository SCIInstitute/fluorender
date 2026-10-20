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
#include <FluiBuilder.h>
#include <VolumePropPanel.h>
#include <MeshPropPanel.h>
#include <AnnotatPropPanel.h>
#include <ManipPropPanel.h>

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

	if (request.HasValue(gstPropertyPageClose))
		ClosePage();
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

PageEntry*
PropertyPanelAgent::FindPage(
	Agent* agent)
{
	auto it = std::find_if(
		m_pages.begin(),
		m_pages.end(),
		[&](const PageEntry& page)
		{
			return page.agent == agent;
		});

	return it == m_pages.end() ?
		nullptr :
		&(*it);
}

std::optional<PageEntry>
PropertyPanelAgent::CreateChildPage(
	const PageKey& key)
{
	auto panel = GetPanel();
	if (!panel)
		return std::nullopt;

	switch (key.type)
	{
	case PropPageType::Volume:
	{
		auto vd = glbin_current.vol_data.lock();
		auto group = glbin_current.vol_group.lock();
		auto view = glbin_current.render_view.lock();
		if (!vd)
			return std::nullopt;

		auto ui =
			glbin_flui_builder.BuildVolumePropPanel(
				panel, vd, group, view);

		return PageEntry{
			key,
			ui->GetAgent(),
			ui,
			false };
	}

	case PropPageType::Mesh:
	{
		auto md = glbin_current.mesh_data.lock();
		auto group = glbin_current.mesh_group.lock();
		if (!md)
			return std::nullopt;

		auto ui =
			glbin_flui_builder.BuildMeshPropPanel(
				panel, md, group);

		return PageEntry{
			key,
			ui->GetAgent(),
			ui,
			false };
	}

	case PropPageType::Annotation:
	{
		auto ann = glbin_current.ann_data.lock();
		if (!ann)
			return std::nullopt;

		auto ui =
			glbin_flui_builder.BuildAnnotatPropPanel(
				panel, ann);

		return PageEntry{
			key,
			ui->GetAgent(),
			ui,
			false };
	}

	case PropPageType::MeshManip:
	{
		auto md = glbin_current.mesh_data.lock();
		if (!md)
			return std::nullopt;

		auto ui =
			glbin_flui_builder.BuildManipPropPanel(
				panel, md);

		return PageEntry{
			key,
			ui->GetAgent(),
			ui,
			false };
	}

	default:
		break;
	}

	return std::nullopt;
}

void PropertyPanelAgent::DeleteChildAgent(
	Agent* agent)
{
	auto it = std::find_if(
		m_pages.begin(),
		m_pages.end(),
		[&](const PageEntry& page)
		{
			return page.agent == agent;
		});

	if (it != m_pages.end())
		m_pages.erase(it);
}

void PropertyPanelAgent::CreatePage()
{
	auto key = GetSelectedPageKey();
	if (!key)
		return;

	if (FindPage(*key))
		return;

	auto page = CreateChildPage(*key);

	m_pages.push_back(std::move(*page));

	auto& stored = m_pages.back();

	GetPanel()->RegisterPage(
		{
			stored.key,
			stored.panel,
			stored.key.name
		});
}

void PropertyPanelAgent::DeletePage()
{
	auto key = GetSelectedPageKey();
	if (!key)
		return;

	auto page = FindPage(*key);
	if (!page)
		return;

	Agent* agent = page->agent;

	DeleteChildAgent(agent);
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

		GetPanel()->ShowPage(
			{
				*key,
				page->panel,
				key->name
			});
	}
	else
	{
		GetPanel()->SelectPage(
			{
				*key,
				page->panel,
				key->name
			});

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

	GetPanel()->HidePage(
		{
			*key,
			page->panel,
			key->name
		});

}

void PropertyPanelAgent::RenamePage()
{
	for (auto& page : m_pages)
	{
		auto* agent =
			dynamic_cast<DataAgent*>(
				page.agent);

		if (!agent)
			continue;

		auto data = agent->GetData();
		if (!data)
			continue;

		page.key.name = data->GetName();

		GetPanel()->RenamePage(
			{
				page.key,
				page.panel,
				page.key.name
			});
	}
}

void PropertyPanelAgent::ClosePage()
{
	auto panel = GetPanel();
	if (!panel)
		return;

	auto page_to_close = panel->GetPage();
	for (auto& page : m_pages)
	{
		if (page.panel == page_to_close)
		{
			page.visible = false;
			break;
		}
	}
}