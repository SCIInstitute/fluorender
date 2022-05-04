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

#include <CalculationAgent.hpp>
#include <CalculationDlg.h>
#include <Names.hpp>
#include <Global.hpp>
#include <Root.hpp>
#include <Renderview.hpp>
#include <VolumeData.hpp>
#include <VolumeGroup.hpp>
#include <Calculate/VolumeCalculator.h>
#include <Calculate/CombineList.h>

using namespace fluo;

CalculationAgent::CalculationAgent(CalculationDlg &dlg) :
	InterfaceAgent(),
	dlg_(dlg)
{
}

void CalculationAgent::setObject(Renderview* obj)
{
	InterfaceAgent::setObject(obj);
}

Renderview* CalculationAgent::getObject()
{
	return dynamic_cast<Renderview*>(InterfaceAgent::getObject());
}

void CalculationAgent::UpdateFui(const ValueCollection &names)
{
	bool update_all = names.empty();
}

void CalculationAgent::LoadVolA()
{
	std::string str;
	long sel_type;
	glbin_root->getValue(gstCurrentSelect, sel_type);
	switch (sel_type)
	{
	case 2://volume
		{
			VolumeData* vd = glbin_root->getCurrentVolumeData();
			if (!vd) break;
			setValue(gstVolumeA, vd);
			str = vd->getName();
		}
		break;
	case 5://volume group
		{
			VolumeGroup* vg = glbin_root->getCurrentVolumeGroup();
			if (!vg) break;
			str = vg->getName();
		}
		break;
	}
	dlg_.m_calc_a_text->SetValue(str);
}

void CalculationAgent::LoadVolB()
{
	std::string str;
	long sel_type;
	glbin_root->getValue(gstCurrentSelect, sel_type);
	switch (sel_type)
	{
	case 2://volume
		{
			VolumeData* vd = glbin_root->getCurrentVolumeData();
			if (!vd) break;
			setValue(gstVolumeB, vd);
			str = vd->getName();
		}
		break;
	case 5://volume group
		{
			VolumeGroup* vg = glbin_root->getCurrentVolumeGroup();
			if (!vg) break;
			str = vg->getName();
		}
		break;
	}
	dlg_.m_calc_b_text->SetValue(str);
}

void CalculationAgent::CalcSub()
{
	Referenced* ref;
	getRefValue(gstVolumeA, &ref);
	VolumeData* vd1 = dynamic_cast<VolumeData*>(ref);
	getRefValue(gstVolumeB, &ref);
	VolumeData* vd2 = dynamic_cast<VolumeData*>(ref);
	if (!vd1 || !vd2)
		return;
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	cal->SetVolumeA(vd1);
	cal->SetVolumeB(vd2);
	cal->CalculateGroup(1);
}

void CalculationAgent::CalcAdd()
{
	Referenced* ref;
	getRefValue(gstVolumeA, &ref);
	VolumeData* vd1 = dynamic_cast<VolumeData*>(ref);
	getRefValue(gstVolumeB, &ref);
	VolumeData* vd2 = dynamic_cast<VolumeData*>(ref);
	if (!vd1 || !vd2)
		return;
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	cal->SetVolumeA(vd1);
	cal->SetVolumeB(vd2);
	cal->CalculateGroup(2);
}

void CalculationAgent::CalcDiv()
{
	Referenced* ref;
	getRefValue(gstVolumeA, &ref);
	VolumeData* vd1 = dynamic_cast<VolumeData*>(ref);
	getRefValue(gstVolumeB, &ref);
	VolumeData* vd2 = dynamic_cast<VolumeData*>(ref);
	if (!vd1 || !vd2)
		return;
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	cal->SetVolumeA(vd1);
	cal->SetVolumeB(vd2);
	cal->CalculateGroup(3);
}

void CalculationAgent::CalcIsc()
{
	Referenced* ref;
	getRefValue(gstVolumeA, &ref);
	VolumeData* vd1 = dynamic_cast<VolumeData*>(ref);
	getRefValue(gstVolumeB, &ref);
	VolumeData* vd2 = dynamic_cast<VolumeData*>(ref);
	if (!vd1 || !vd2)
		return;
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	cal->SetVolumeA(vd1);
	cal->SetVolumeB(vd2);
	cal->CalculateGroup(4);
}

void CalculationAgent::CalcFill()
{
	Referenced* ref;
	getRefValue(gstVolumeA, &ref);
	VolumeData* vd1 = dynamic_cast<VolumeData*>(ref);
	getRefValue(gstVolumeB, &ref);
	VolumeData* vd2 = dynamic_cast<VolumeData*>(ref);
	if (!vd1 && !vd2)
		return;
	flrd::VolumeCalculator* cal = getObject()->GetVolumeCalculator();
	if (!cal) return;
	cal->SetVolumeA(vd1?vd1:vd2);
	cal->SetVolumeB(nullptr);
	cal->CalculateGroup(9);
}

void CalculationAgent::CalcCombine()
{
	VolumeGroup* vg = glbin_root->getCurrentVolumeGroup();
	flrd::CombineList Op;
	std::string name = vg->getName();
	name += "_combined";
	Op.SetName(name);
	std::list<VolumeData*> channs;
	for (int i = 0; i < vg->getNumChildren(); ++i)
	{
		VolumeData* vd = vg->getChild(i)->asVolumeData();
		if (!vd)
			continue;
		channs.push_back(vd);
	}
	Op.SetVolumes(channs);
	if (Op.Execute())
	{
		std::list<VolumeData*> results;
		Op.GetResults(results);
		if (results.empty())
			return;

		VolumeGroup* group = 0;
		VolumeData* volume = 0;
		for (auto i = results.begin(); i != results.end(); ++i)
		{
			VolumeData* vd = *i;
			if (vd)
			{
				if (!volume) volume = vd;
				//m_frame->GetDataManager()->AddVolumeData(vd);
				if (i == results.begin())
					group = getObject()->addVolumeGroup();
				getObject()->addVolumeData(vd, group);
			}
		}
		//if (group && volume)
		//{
		//	Color col = volume->GetGamma();
		//	group->SetGammaAll(col);
		//	col = volume->GetBrightness();
		//	group->SetBrightnessAll(col);
		//	col = volume->GetHdr();
		//	group->SetHdrAll(col);
		//}
		//m_frame->UpdateList();
		//m_frame->UpdateTree(m_group->getName());
		//m_view->Update(39);
	}
}

