/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#include <CompSelector.h>
#include <CompAnalyzer.h>
#include <Global.h>
#include <set>

using namespace flrd;

ComponentSelector::ComponentSelector()
	: m_vd(0),
	m_sel_all(false),
	m_id(0),
	m_use_min(false),
	m_use_max(false),
	m_min_num(0),
	m_max_num(0)
{

}

ComponentSelector::~ComponentSelector()
{
}

void ComponentSelector::SelectFullComp()
{
	if (!m_vd)
		return;

	//get id
	wxString str = m_comp_id_text->GetValue();
	if (str.empty())
	{
		if (!m_view || !m_view->m_cur_vol)
			return;
		//get current mask
		glbin_comp_selector.SetVolume(m_view->m_cur_vol);
		//cell size filter
		bool use = m_analysis_min_check->GetValue();
		unsigned int num = (unsigned int)(m_analysis_min_spin->GetValue());
		glbin_comp_selector.SetMinNum(use, num);
		use = m_analysis_max_check->GetValue();
		num = (unsigned int)(m_analysis_max_spin->GetValue());
		glbin_comp_selector.SetMaxNum(use, num);
		glbin_comp_selector.SetAnalyzer(&glbin_comp_analyzer);
		glbin_comp_selector.CompFull();
	}
	else
	{
		wxCommandEvent e;
		OnCompAppend(e);
	}

	//m_view->RefreshGL(39);

	//frame
	//if (m_frame)
	//{
	//	if (m_frame->GetBrushToolDlg())
	//	{
	//		if (m_view->m_paint_count)
	//			m_frame->GetBrushToolDlg()->Update(0);
	//		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
	//	}
	//	if (m_frame->GetColocalizationDlg() &&
	//		m_view->m_paint_colocalize)
	//		m_frame->GetColocalizationDlg()->Colocalize();
	//}
}

bool ComponentSelector::GetCellList(flrd::CelpList& cl, bool links)
{
	flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
	if (!list || list->empty())
		return false;

	cl.min = list->min;
	cl.max = list->max;
	cl.sx = list->sx;
	cl.sy = list->sy;
	cl.sz = list->sz;

	bool sel_all = false;
	std::vector<unsigned int> ids;
	std::vector<unsigned int> bids;
	int bn = glbin_comp_analyzer.GetBrickNum();

	//selected cells are retrieved using different functions
	wxArrayInt seli = m_output_grid->GetSelectedCols();
	if (seli.GetCount() > 0)
		sel_all = true;
	if (!sel_all)
	{
		seli = m_output_grid->GetSelectedRows();
		AddSelArrayInt(ids, bids, seli, bn > 1);
		//wxGridCellCoordsArray sela =
		//	m_output_grid->GetSelectionBlockBottomRight();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
		//sela = m_output_grid->GetSelectionBlockTopLeft();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
		//sela = m_output_grid->GetSelectedCells();
		//AddSelCoordArray(ids, bids, sela, bn > 1);
	}

	double sx = list->sx;
	double sy = list->sy;
	double sz = list->sz;
	if (sel_all)
	{
		for (auto it = list->begin(); it != list->end(); ++it)
			FindCelps(cl, it, links);
	}
	else
	{
		for (size_t i = 0; i < ids.size(); ++i)
		{
			unsigned long long key = ids[i];
			unsigned int bid = 0;
			if (bn > 1)
			{
				key = bids[i];
				key = (key << 32) | ids[i];
				bid = bids[i];
			}
			auto it = list->find(key);
			if (it != list->end())
				FindCelps(cl, it, links);
		}
	}

	if (cl.empty())
		return false;
	return true;
}

void ComponentSelector::GetCompSelection()
{
	if (m_view)
	{
		flrd::CelpList cl;
		GetCellList(cl);
		m_view->SetCellList(cl);
		m_view->SetInteractive(false);
		m_view->RefreshGL(39);
	}
}

void ComponentSelector::SetCompSelection(std::set<unsigned long long>& ids, int mode)
{
	if (ids.empty())
		return;

	int bn = glbin_comp_analyzer.GetBrickNum();

	wxString str;
	unsigned long ulv;
	unsigned long long ull;
	bool flag = mode == 1;
	int lasti = -1;
	wxArrayInt sel = m_output_grid->GetSelectedRows();
	std::set<int> rows;
	for (int i = 0; i < sel.GetCount(); ++i)
		rows.insert(sel[i]);
	for (int i = 0; i < m_output_grid->GetNumberRows(); ++i)
	{
		str = m_output_grid->GetCellValue(i, 0);
		if (!str.ToULong(&ulv))
			continue;
		if (bn > 1)
		{
			str = m_output_grid->GetCellValue(i, 1);
			if (!str.ToULongLong(&ull))
				continue;
			ull = (ull << 32) | ulv;
		}
		else
			ull = ulv;
		if (ids.find(ull) != ids.end())
		{
			if (!flag)
			{
				m_output_grid->ClearSelection();
				flag = true;
			}
			if (mode == 0)
			{
				m_output_grid->SelectRow(i, true);
				lasti = i;
			}
			else
			{
				if (rows.find(i) != rows.end())
					m_output_grid->DeselectRow(i);
				else
				{
					m_output_grid->SelectRow(i, true);
					lasti = i;
				}
			}
		}
	}

	if (flag)
	{
		GetCompSelection();
		if (lasti >= 0)
			m_output_grid->GoToCell(lasti, 0);
	}
}

void ComponentSelector::IncludeComps()
{
	if (!m_view || !m_view->m_cur_vol)
		return;

	flrd::CelpList cl;
	if (GetCellList(cl, true))
	{
		//clear complist
		flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
		for (auto it = list->begin();
			it != list->end();)
		{
			if (cl.find(it->second->GetEId()) == cl.end())
				it = list->erase(it);
			else
				++it;
		}
		//select cl
		glbin_comp_selector.SetVolume(m_view->m_cur_vol);
		glbin_comp_selector.SelectList(cl);
		ClearOutputGrid();
		string titles, values;
		glbin_comp_analyzer.OutputFormHeader(titles);
		glbin_comp_analyzer.OutputCompListStr(values, 0);
		wxString str1(titles), str2(values);
		SetOutput(str1, str2);

		cl.clear();
		m_view->SetCellList(cl);
		m_view->SetInteractive(false);
		m_view->RefreshGL(39);

		//frame
		//if (m_frame)
		//{
		//	if (m_frame->GetBrushToolDlg())
		//	{
		//		if (m_view->m_paint_count)
		//			m_frame->GetBrushToolDlg()->Update(0);
		//		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
		//	}
		//	if (m_frame->GetColocalizationDlg() &&
		//		m_view->m_paint_colocalize)
		//		m_frame->GetColocalizationDlg()->Colocalize();
		//}
	}
}

void ComponentSelector::ExcludeComps()
{
	if (!m_view || !m_view->m_cur_vol)
		return;

	flrd::CelpList cl;
	if (GetCellList(cl, true))
	{
		//clear complist
		flrd::CelpList* list = glbin_comp_analyzer.GetCelpList();
		for (auto it = list->begin();
			it != list->end();)
		{
			if (cl.find(it->second->GetEId()) != cl.end())
				it = list->erase(it);
			else
				++it;
		}
		glbin_comp_selector.SetVolume(m_view->m_cur_vol);
		std::vector<unsigned long long> ids;
		for (auto it = list->begin();
			it != list->end(); ++it)
			ids.push_back(it->second->GetEId());
		glbin_comp_selector.Delete(ids);
		ClearOutputGrid();
		string titles, values;
		glbin_comp_analyzer.OutputFormHeader(titles);
		glbin_comp_analyzer.OutputCompListStr(values, 0);
		wxString str1(titles), str2(values);
		SetOutput(str1, str2);

		cl.clear();
		m_view->SetCellList(cl);
		m_view->SetInteractive(false);
		m_view->RefreshGL(39);

		//frame
		//if (m_frame)
		//{
		//	if (m_frame->GetBrushToolDlg())
		//	{
		//		if (m_view->m_paint_count)
		//			m_frame->GetBrushToolDlg()->Update(0);
		//		m_frame->GetBrushToolDlg()->UpdateUndoRedo();
		//	}
		//	if (m_frame->GetColocalizationDlg() &&
		//		m_view->m_paint_colocalize)
		//		m_frame->GetColocalizationDlg()->Colocalize();
		//}
	}
}

void ComponentSelector::CompFull()
{
	if (!m_vd)
		return;
	flvr::Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_>0)
		tex->push_mask();

	//get current mask
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Nrrd* nrrd_label = m_vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;
	size_t bn = bricks->size();

	//get selected IDs
	int i, j, k;
	int nx, ny, nz;
	unsigned long long index;
	unsigned int label_value;
	unsigned int brick_id;
	CelpList sel_labels;
	CelpListIter label_iter;
	m_vd->GetResolution(nx, ny, nz);

	for (size_t bi = 0; bi < bn; ++bi)
	{
		flvr::TextureBrick* b = (*bricks)[bi];
		if (!b->is_mask_valid())
			continue;

		brick_id = b->get_id();
		for (i = 0; i < b->nx(); ++i)
		for (j = 0; j < b->ny(); ++j)
		for (k = 0; k < b->nz(); ++k)
		{
			index = (unsigned long long)nx*(unsigned long long)ny*(unsigned long long)(k + b->oz()) +
				(unsigned long long)nx*(unsigned long long)(j + b->oy()) + (unsigned long long)(i + b->ox());
			if (data_mask[index] &&
				data_label[index])
			{
				label_value = data_label[index];
				label_iter = sel_labels.find(Cell::GetKey(label_value, brick_id));
				if (label_iter == sel_labels.end())
				{
					Cell* info = new Cell(label_value, brick_id);
					if (!glbin_comp_analyzer.GetAnalyzed())
						info->SetSizeUi(1);
					sel_labels.insert(std::pair<unsigned long long, Celp>
						(info->GetEId(), Celp(info)));
				}
				else if (!glbin_comp_analyzer.GetAnalyzed())
					label_iter->second->Inc();
			}
		}
	}

	CelpList list_out;
	CelpList* comp_list = GetListFromAnalyzer(sel_labels, list_out);

	//reselect
	unsigned int size;
	for (size_t bi = 0; bi < bn; ++bi)
	{
		flvr::TextureBrick* b = (*bricks)[bi];
		brick_id = b->get_id();
		if (!b->is_mask_valid() &&
			!comp_list->FindBrick(brick_id))
			continue;
		
		for (i = 0; i < b->nx(); ++i)
		for (j = 0; j < b->ny(); ++j)
		for (k = 0; k < b->nz(); ++k)
		{
			index = (unsigned long long)nx*(unsigned long long)ny*(unsigned long long)(k + b->oz()) +
				(unsigned long long)nx*(unsigned long long)(j + b->oy()) + (unsigned long long)(i + b->ox());
			if (data_label[index])
			{
				label_value = data_label[index];
				label_iter = comp_list->find(Cell::GetKey(label_value, brick_id));
				if (label_iter != comp_list->end())
				{
					if (m_use_min || m_use_max)
					{
						size = label_iter->second->GetSizeUi();
						if (CompareSize(size))
							SelectMask(data_mask, index, 255, tex);
						else
							data_mask[index] = 0;
					}
					else
						SelectMask(data_mask, index, 255, tex);
				}
				else
					data_mask[index] = 0;
			}
		}
		b->valid_mask();
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::Select(bool all, bool rmask)
{
	//get current mask
	if (!m_vd)
		return;
	flvr::Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_>0)
		tex->push_mask();

	Nrrd* nrrd_mask = m_vd->GetMask(rmask);
	if (!nrrd_mask)
	{
		m_vd->AddEmptyMask(0);
		nrrd_mask = m_vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Nrrd* nrrd_label = m_vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;

	int bn = m_vd->GetBrickNum();
	unsigned int idsel, bidsel;
	if (bn > 1)
	{
		idsel = m_id << 32 >> 32;
		bidsel = m_id >> 32;
	}
	else
	{
		idsel = m_id;
		bidsel = 0;
	}

	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx*
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long index;
	unsigned int label_value;
	CelpListIter label_iter;
	unsigned int bid;
	unsigned int size;
	if (all)
	{
		CelpList sel_labels;
		CelpList* comp_list = 0;
		if (glbin_comp_analyzer.GetAnalyzed())
			comp_list = glbin_comp_analyzer.GetCelpList();
		else
		{
			for (index = 0; index < for_size; ++index)
			{
				if (data_label[index])
				{
					label_value = data_label[index];
					bid = tex->get_brick_id(index);
					label_iter = sel_labels.find(Cell::GetKey(label_value, bid));
					if (label_iter == sel_labels.end())
					{
						Cell* info = new Cell(label_value, bid);
						info->SetSizeUi(1);
						sel_labels.insert(std::pair<unsigned long long, Celp>
							(info->GetEId(), Celp(info)));
					}
					else
						label_iter->second->Inc();
				}
			}
			comp_list = &sel_labels;
		}

		//reselect
		for (index = 0; index < for_size; ++index)
		{
			if (data_label[index])
			{
				label_value = data_label[index];
				bid = tex->get_brick_id(index);
				label_iter = comp_list->find(Cell::GetKey(label_value, bid));
				if (label_iter != comp_list->end())
				{
					if (m_use_min || m_use_max)
					{
						size = label_iter->second->GetSizeUi();
						if (CompareSize(size))
							//data_mask[index] = 255;
							SelectMask(data_mask, index, 255, tex);
						else
							data_mask[index] = 0;
					}
					else
						SelectMask(data_mask, index, 255, tex);
				}
				else if (!m_use_min && m_use_max)
					//analyzer filters small comps, make sure they are also selected here
					SelectMask(data_mask, index, 255, tex);
				else
					data_mask[index] = 0;
			}
		}
	}
	else
	{
		bool simple_select = true;
		simple_select = false;
		CelpList* analyzer_list = glbin_comp_analyzer.GetCelpList();
		auto iter = analyzer_list->find(m_id);
		if (iter == analyzer_list->end())
			simple_select = true;
		else
		{
			glbin_comp_analyzer.GetCellGraph()->ClearVisited();
			CelpList comp_list;
			if (glbin_comp_analyzer.GetCellGraph()->
				GetLinkedComps(iter->second, comp_list))
			{
				for (index = 0; index < for_size; ++index)
				{
					if (data_label[index])
					{
						label_value = data_label[index];
						bid = tex->get_brick_id(index);
						label_iter = comp_list.find(Cell::GetKey(label_value, bid));
						if (label_iter != comp_list.end())
						{
							if (m_use_min || m_use_max)
							{
								size = label_iter->second->GetSizeUi();
								if (CompareSize(size))
									SelectMask(data_mask, index, 255, tex);
								else
									data_mask[index] = 0;
							}
							else
								SelectMask(data_mask, index, 255, tex);
						}
					}
				}
			}
			else
				simple_select = true;
		}

		if (simple_select)
		{
			unsigned long long acc_size = 0;
			if (bn > 1)
			{
				for (index = 0; index < for_size; ++index)
				{
					bid = tex->get_brick_id(index);
					if (data_label[index] == idsel &&
						bid == bidsel)
						acc_size++;
				}
				if (((m_use_min || m_use_max) &&
					CompareSize((unsigned int)(acc_size))) ||
					(!m_use_min && !m_use_max))
				{
					for (index = 0; index < for_size; ++index)
					{
						bid = tex->get_brick_id(index);
						if (data_label[index] == idsel &&
							bid == bidsel)
							SelectMask(data_mask, index, 255, tex);
					}
				}
			}
			else
			{
				for (index = 0; index < for_size; ++index)
				{
					if (data_label[index] == idsel)
						acc_size++;
				}
				if (((m_use_min || m_use_max) &&
					CompareSize((unsigned int)(acc_size))) ||
					(!m_use_min && !m_use_max))
				{
					for (index = 0; index < for_size; ++index)
					{
						if (data_label[index] == idsel)
							SelectMask(data_mask, index, 255, tex);
					}
				}
			}
		}
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_mask(false);
}

void ComponentSelector::Exclusive()
{
	Clear(false);
	Select(false, false);
}

void ComponentSelector::All()
{
	//get current mask
	if (!m_vd)
		return;

	if (flvr::Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
	{
		m_vd->AddEmptyMask(0);
		nrrd_mask = m_vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;

	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	std::memset(data_mask, 255, for_size);
	m_vd->GetTexture()->valid_all_mask();
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::Clear(bool invalidate)
{
	//get current mask
	if (!m_vd)
		return;

	if (flvr::Texture::mask_undo_num_>0 &&
		m_vd->GetTexture())
		m_vd->GetTexture()->push_mask();

	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;

	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	std::memset(data_mask, 0, for_size);
	//invalidate label mask in gpu
	if (invalidate)
		m_vd->GetVR()->clear_tex_mask();
	m_vd->GetTexture()->invalid_all_mask();
}

void ComponentSelector::Delete()
{
	//get current mask
	if (!m_vd)
		return;
	flvr::Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Nrrd* nrrd_label = m_vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	for (index = 0; index < for_size; ++index)
	{
		if (m_id == data_label[index])
			SelectMask(data_mask, index, 255, tex);
		else
			data_mask[index] = 0;
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_mask();
}

void ComponentSelector::Delete(std::vector<unsigned long long> &ids)
{
	bool clear_all = ids.empty();

	//get current mask
	if (!m_vd)
		return;
	flvr::Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
		return;
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Nrrd* nrrd_label = m_vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	//select append
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	unsigned long long index;
	unsigned long long for_size = (unsigned long long)nx *
		(unsigned long long)ny * (unsigned long long)nz;
	unsigned long long key;
	int bn = m_vd->GetBrickNum();
	for (index = 0; index < for_size; ++index)
	{
		if (clear_all)
			data_mask[index] = 0;
		else
		{
			if (bn > 1)
				key = flrd::Cell::GetKey(
					data_label[index],
					m_vd->GetTexture()->
					get_brick_id(index));
			else
				key = data_label[index];
			if (find(ids.begin(), ids.end(), key)
				!= ids.end())
				SelectMask(data_mask, index, 255, tex);
			else
				data_mask[index] = 0;
		}
	}
	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_mask();
	if (clear_all)
		tex->invalid_all_mask();
}

void ComponentSelector::SelectList(CelpList& list)
{
	if (!m_vd)
		return;

	flvr::Texture* tex = m_vd->GetTexture();
	if (!tex)
		return;
	if (flvr::Texture::mask_undo_num_ > 0)
		tex->push_mask();

	Nrrd* nrrd_mask = m_vd->GetMask(true);
	if (!nrrd_mask)
	{
		m_vd->AddEmptyMask(0);
		nrrd_mask = m_vd->GetMask(false);
	}
	unsigned char* data_mask = (unsigned char*)(nrrd_mask->data);
	if (!data_mask)
		return;
	//get current label
	Nrrd* nrrd_label = m_vd->GetLabel(true);
	if (!nrrd_label)
		return;
	unsigned int* data_label = (unsigned int*)(nrrd_label->data);
	if (!data_label)
		return;
	std::vector<flvr::TextureBrick*> *bricks = tex->get_bricks();
	if (!bricks || bricks->size() == 0)
		return;
	size_t bn = bricks->size();

	//select append
	unsigned int brick_id;
	unsigned long long index;
	unsigned long long key;
	int i, j, k;
	int nx, ny, nz;
	m_vd->GetResolution(nx, ny, nz);
	for (size_t bi = 0; bi < bn; ++bi)
	{
		flvr::TextureBrick* b = (*bricks)[bi];
		brick_id = b->get_id();
		for (i = 0; i < b->nx(); ++i)
		for (j = 0; j < b->ny(); ++j)
		for (k = 0; k < b->nz(); ++k)
		{
			index = (unsigned long long)nx*(unsigned long long)ny*(unsigned long long)(k + b->oz()) +
				(unsigned long long)nx*(unsigned long long)(j + b->oy()) + (unsigned long long)(i + b->ox());
			key = brick_id;
			key = (key << 32) | data_label[index];
			if (list.find(key) != list.end())
				SelectMask(data_mask, index, 255, tex);
			else
				data_mask[index] = 0;
		}
	}

	//unsigned long long for_size = (unsigned long long)nx *
	//	(unsigned long long)ny * (unsigned long long)nz;
	//unsigned long long index;
	//for (index = 0;
	//	index < for_size; ++index)
	//{
	//	if (list.find(data_label[index]) != list.end())
	//		data_mask[index] = 255;
	//	else
	//		data_mask[index] = 0;
	//}

	//invalidate label mask in gpu
	m_vd->GetVR()->clear_tex_mask();
}

inline CelpList* ComponentSelector::GetListFromAnalyzer(CelpList &list_in, CelpList &list_out)
{
	if (glbin_comp_analyzer.GetAnalyzed())
	{
		//assign graph node identifier for sel_labels
		CelpList* analyzer_list = glbin_comp_analyzer.GetCelpList();
		for (auto iter = list_in.begin(); iter != list_in.end(); )
		{
			auto iter2 = analyzer_list->find(iter->first);
			if (iter2 == analyzer_list->end())
			{
				//remove
				iter = list_in.erase(iter);
				continue;
			}
			iter->second->SetCelVrtx(iter2->second->GetCelVrtx());
			iter->second->SetSizeUi(iter2->second->GetSizeUi());
			++iter;
		}
		if (glbin_comp_analyzer.GetCellGraph()->
			GetLinkedComps(list_in, list_out,
				glbin_comp_analyzer.GetSizeLimit()))
			return &list_out;
		else
			return &list_in;
	}
	else
		return &list_in;
	return 0;
}

void ComponentSelector::SelectMask(unsigned char* mask,
	unsigned long long idx, unsigned char v, flvr::Texture* tex)
{
	mask[idx] = v;
	if (tex)
	{
		flvr::TextureBrick* b = tex->get_brick(
			tex->get_brick_id(idx));
		if (b)
			b->valid_mask();
	}
}

void ComponentSelector::FindCelps(flrd::CelpList& list,
	flrd::CelpListIter& it, bool links)
{
	list.insert(pair<unsigned long long, flrd::Celp>
		(it->second->GetEId(), it->second));

	if (links)
	{
		flrd::CellGraph* graph = glbin_comp_analyzer.GetCellGraph();
		graph->ClearVisited();
		flrd::CelpList links;
		if (graph->GetLinkedComps(it->second, links,
			glbin_comp_analyzer.GetSizeLimit()))
		{
			for (auto it2 = links.begin();
				it2 != links.end(); ++it2)
			{
				list.insert(pair<unsigned long long, flrd::Celp>
					(it2->second->GetEId(), it2->second));
			}
		}
	}
}

