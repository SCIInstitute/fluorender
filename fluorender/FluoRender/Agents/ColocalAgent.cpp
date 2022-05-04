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

#include <ColocalAgent.hpp>
#include <ColocalDlg.h>
#include <VolumeData.hpp>
#include <Renderview.hpp>
#include <SearchVisitor.hpp>
#include <Calculate/Compare.h>
#include <vector>
#include <string>
#include <fstream>

using namespace fluo;

ColocalAgent::ColocalAgent(ColocalDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void ColocalAgent::setupInputs()
{

}

void ColocalAgent::setObject(VolumeGroup* vg)
{
	InterfaceAgent::setObject(vg);
}

VolumeGroup* ColocalAgent::getObject()
{
	return dynamic_cast<VolumeGroup*>(InterfaceAgent::getObject());
}

void ColocalAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
}

void ColocalAgent::Run()
{
	//get volume group
	VolumeGroup* group = getObject();
	if (!group)
		return;
	int num = group->getNumChildren();
	if (num < 2)
		return;
	//get view of group
	SearchVisitor visitor;
	visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
	visitor.matchClassName("Renderview");
	group->accept(visitor);
	Renderview* view = visitor.getRenderview();
	if (!view)
		return;

	//spacings, assuming they are all same for channels
	double spcx, spcy, spcz;
	double spc;
	std::string unit;
	VolumeData* vd = group->getChild(0)->asVolumeData();
	if (!vd)
	{
		spc = spcx = spcy = spcz = 1.0;
	}
	else
	{
		vd->getValue(gstSpcX, spcx);
		vd->getValue(gstSpcY, spcy);
		vd->getValue(gstSpcZ, spcz);
		spc = spcx * spcy * spcz;
	}
	if (view)
	{
		long sb_unit;
		view->getValue(gstScaleBarUnit, sb_unit);
		switch (sb_unit)
		{
		case 0:
			unit = "nm\u00B3";
			break;
		case 1:
		default:
			unit = "\u03BCm\u00B3";
			break;
		case 2:
			unit = "mm\u00B3";
			break;
		}
	}

	//result
	std::vector<std::vector<double>> rm;//result matrix
	rm.reserve(num);
	for (size_t i = 0; i < num; ++i)
	{
		rm.push_back(std::vector<double>());
		rm[i].reserve(num);
		for (size_t j = 0; j < num; ++j)
			rm[i].push_back(0);
	}

	std::string titles;
	std::string values;
	long method;
	getValue(gstColocalMethod, method);
	bool int_weighted, use_sel;
	getValue(gstIntWeighted, int_weighted);
	getValue(gstUseSelection, use_sel);

	//fill the matrix
	if (method == 0 || method == 1 ||
		(method == 2 && !int_weighted))
	{
		//dot product and min value
		//symmetric matrix
		for (int it1 = 0; it1 < num; ++it1)
		{
			for (int it2 = it1; it2 < num; ++it2)
			{
				VolumeData* vd1 = group->getChild(it1)->asVolumeData();
				VolumeData* vd2 = group->getChild(it2)->asVolumeData();
				if (!vd1 || !vd2)
					continue;
				bool disp1, disp2;
				vd1->getValue(gstDisplay, disp1);
				vd2->getValue(gstDisplay, disp2);
				if (!disp1 || !disp2)
					continue;

				flrd::ChannelCompare compare(vd1, vd2);
				compare.SetUseMask(use_sel);
				compare.SetIntWeighted(int_weighted);
				//boost::signals2::connection preconn =
				//	compare.prework.connect(std::bind(
				//		&ColocalDlg::StartTimer, this, std::placeholders::_1));
				//boost::signals2::connection postconn =
				//	compare.postwork.connect(std::bind(
				//		&ColocalDlg::StopTimer, this, std::placeholders::_1));
				switch (method)
				{
				case 0://dot product
					compare.Product();
					break;
				case 1://min value
					compare.MinValue();
					break;
				case 2://threshold
				{
					//get threshold values
					double th1, th2, th3, th4;
					vd1->getValue(gstLowThreshold, th1);
					vd1->getValue(gstHighThreshold, th2);
					vd2->getValue(gstLowThreshold, th3);
					vd2->getValue(gstHighThreshold, th4);
					compare.Threshold((float)th1, (float)th2, (float)th3, (float)th4);
				}
				break;
				}
				rm[it1][it2] = compare.Result();
				if (it1 != it2)
					rm[it2][it1] = compare.Result();
			}
		}
	}
	else if (method == 2 && int_weighted)
	{
		//threshold, asymmetrical
		for (int it1 = 0; it1 < num; ++it1)
			for (int it2 = 0; it2 < num; ++it2)
			{
				VolumeData* vd1 = group->getChild(it1)->asVolumeData();
				VolumeData* vd2 = group->getChild(it2)->asVolumeData();
				if (!vd1 || !vd2)
					continue;
				bool disp1, disp2;
				vd1->getValue(gstDisplay, disp1);
				vd2->getValue(gstDisplay, disp2);
				if (!disp1 || !disp2)
					continue;

				flrd::ChannelCompare compare(vd1, vd2);
				compare.SetUseMask(use_sel);
				compare.SetIntWeighted(int_weighted);
				//boost::signals2::connection preconn =
				//	compare.prework.connect(std::bind(
				//		&ColocalDlg::StartTimer, this, std::placeholders::_1));
				//boost::signals2::connection postconn =
				//	compare.postwork.connect(std::bind(
				//		&ColocalDlg::StopTimer, this, std::placeholders::_1));
				//get threshold values
				double th1, th2, th3, th4;
				vd1->getValue(gstLowThreshold, th1);
				vd1->getValue(gstHighThreshold, th2);
				vd2->getValue(gstLowThreshold, th3);
				vd2->getValue(gstHighThreshold, th4);
				compare.Threshold((float)th1, (float)th2, (float)th3, (float)th4);
				rm[it1][it2] = compare.Result();
			}
	}

	std::string name;
	double v;
	bool get_ratio, phys_size;
	getValue(gstGetRatio, get_ratio);
	getValue(gstPhysSize, phys_size);
	ResetMinMax();
	for (size_t i = 0; i < num; ++i)
	{
		titles += std::to_string(int(i + 1));
		if (get_ratio)
			titles += " (%%)";
		VolumeData* vd = group->getChild(i)->asVolumeData();
		if (vd)
			name = vd->getName();
		else
			name = "";
		titles += ": " + name;
		if (i < num - 1)
			titles += "\t";
		else
			titles += "\n";
	}
	for (int it1 = 0; it1 < num; ++it1)
	for (int it2 = 0; it2 < num; ++it2)
	{
		if (get_ratio)
		{
			if (rm[it2][it2])
			{
				v = rm[it1][it2] * 100.0 / rm[it1][it1];
				SetMinMax(v);
				values += std::to_string(v);
			}
			else
			{
				SetMinMax(0.0);
				values += "0";
			}
		}
		else
		{
			if (phys_size)
			{
				v = rm[it1][it2] * spc;
				SetMinMax(v);
				values += std::to_string(v);
				values += unit;
			}
			else
			{
				v = rm[it1][it2];
				SetMinMax(v);
				if (int_weighted)
					values += std::to_string(v);
				else
					values += std::to_string(int(v));
			}
		}
		if (it2 < num - 1)
			values += "\t";
		else
			values += "\n";
	}
	SetOutput(titles, values);
}

void ColocalAgent::SetOutput(const std::string &titles, const std::string &values)
{
	wxString copy_data;
	wxString cur_field;
	wxString cur_line;
	int i, k;

	k = 0;
	cur_line = titles;
	do
	{
		cur_field = cur_line.BeforeFirst('\t');
		cur_line = cur_line.AfterFirst('\t');
		if (dlg_.m_output_grid->GetNumberCols() <= k)
			dlg_.m_output_grid->InsertCols(k);
		dlg_.m_output_grid->SetColLabelValue(k, cur_field);
		++k;
	} while (cur_line.IsEmpty() == false);

	fluo::Color c;
	double val;
	wxColor color;
	fluo::VolumeData* vd = 0;
	bool colormap;
	getValue(gstColormapEnable, colormap);
	if (colormap)
	{
		VolumeGroup* group = getObject();
		if (group)
			vd = group->getChild(0)->asVolumeData();
	}
	double dmin, dmax;
	getValue(gstColormapLow, dmin);
	getValue(gstColormapHigh, dmax);
	colormap = colormap && vd && (dmax - dmin) > 0.0;
	bool hold_history;
	getValue(gstHoldHistory, hold_history);

	i = 0;
	copy_data = values;
	do
	{
		k = 0;
		cur_line = copy_data.BeforeFirst('\n');
		copy_data = copy_data.AfterFirst('\n');
		if (dlg_.m_output_grid->GetNumberRows() <= i ||
			hold_history)
			dlg_.m_output_grid->InsertRows(i);
		do
		{
			cur_field = cur_line.BeforeFirst('\t');
			cur_line = cur_line.AfterFirst('\t');
			dlg_.m_output_grid->SetCellValue(i, k, cur_field);
			if (colormap && cur_field.ToDouble(&val))
			{
				c = vd->GetColorFromColormap((val - dmin) / (dmax - dmin));
				color = wxColor(c.r() * 255, c.g() * 255, c.b() * 255);
				dlg_.m_output_grid->SetCellBackgroundColour(i, k, color);
			}
			else
			{
				color = *wxWHITE;
				dlg_.m_output_grid->SetCellBackgroundColour(i, k, color);
			}
			++k;
		} while (cur_line.IsEmpty() == false);
		++i;
	} while (copy_data.IsEmpty() == false);

	if (dlg_.m_output_grid->GetNumberCols() > k)
		dlg_.m_output_grid->DeleteCols(k,
			dlg_.m_output_grid->GetNumberCols() - k);

	//dlg_.m_output_grid->AutoSizeColumns(false);
	bool phys_size, get_ratio;
	getValue(gstPhysSize, phys_size);
	getValue(gstGetRatio, get_ratio);
	if (phys_size && !get_ratio)
		dlg_.m_output_grid->SetDefaultColSize(100, true);
	else
		dlg_.m_output_grid->SetDefaultColSize(70, true);
	dlg_.m_output_grid->ForceRefresh();
	dlg_.m_output_grid->ClearSelection();
}

void ColocalAgent::OnAutoUpdateChanged(Event& event)
{
	bool bval;
	getValue(gstAutoUpdate, bval);
	VolumeGroup* group = getObject();
	if (!group) return;
	//get view of group
	SearchVisitor visitor;
	visitor.setTraversalMode(NodeVisitor::TRAVERSE_PARENTS);
	visitor.matchClassName("Renderview");
	group->accept(visitor);
	Renderview* view = visitor.getRenderview();
	if (!view)
		return;
	view->setValue(gstPaintColocalize, bval);
}

void ColocalAgent::OnSettingChanged(Event& event)
{
	bool bval;
	getValue(gstAutoUpdate, bval);
	if (bval) Run();
}

