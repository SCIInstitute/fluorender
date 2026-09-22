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
#ifndef MeasureDlgAgent_h
#define MeasureDlgAgent_h

#include <Agent.h>
#include <Names.h>

class MeasureDlg;
struct RulerListInfo;
struct RulerCurrentInfo;
struct RulerListDisplayInfo;
struct RulerGroupSelectionInfo;
struct RulerProfileInfo;
class MeasureDlgAgent : public Agent
{
public:
	MeasureDlgAgent(
		MeasureDlg* dlg);

	virtual ~MeasureDlgAgent() = default;

	MeasureDlg* GetDialog() const;

	void SetCurrentRuler(const RulerCurrentInfo& info);
	void SetIntensityMethod(int ival);
	void SetTransient(bool bval);
	void SetUseTransfer(bool bval);
	void SetDispPoint(bool bval);
	void SetDispLine(bool bval);
	void SetDispName(bool bval);
	void SetDispAll(bool bval);
	void SetRelaxData(int ival);
	void SetRelaxValue(double dval);
	void SetGroup(unsigned int ival);
	void SetInterpolation(int ival);
	void SetAlignCenter(bool bval);
	void AlignRuler(int ival);
	void AlignPca(int ival);

	bool GetSelectedRulerText(
		std::wstring& text) const;
	RulerCurrentInfo GetCurrentRulerInfo();

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	RulerListInfo GetRulerListInfo();
	RulerListDisplayInfo GetRulerListDisplayInfo();
	RulerGroupSelectionInfo GetGroupSelectionInfo();
	RulerProfileInfo GetProfileInfo();

	void ToggleDisplay();
	void SetSelectedRulers();

	//toolbar1
	void Locator();
	void Probe();
	void RulerLine();
	void Protractor();
	void Ellipse();
	void RulerPolyline();
	void Pencil();
	void Grow();

	//toolbar2
	void RulerMove();
	void RulerMovePoint();
	void Magnet();
	void RulerMovePencil();
	void RulerFlip();
	void RulerAvg();
	void Lock();
	void Relax();

	//toolbar3
	void DeleteSelection();
	void DeleteAll();
	void DeletePoint();
	void Prune();//remove branches with length equal to or smaller than len
	void Profile();
	void Distance();
	void Project();
	void Export();

	//others
	void NewGroup();
	void GroupRulers();
	void ToggleGroupDisp();
	void DeleteKey();
	void DeleteAllKeys();
};

#endif // MeasureDlgAgent_h
