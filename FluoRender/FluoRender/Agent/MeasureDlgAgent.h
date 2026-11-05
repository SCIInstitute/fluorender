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
#include <Tokens.h>

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

	virtual void ApplySettings() override;
	virtual void PreShutdown() override;

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
		gstFreehandToolState,
		gstRulerList,
		gstRulerListCur,
		gstRulerListDisp,
		gstRulerListSel,
		gstRulerGroupSel,
		gstRulerProfile,
		gstRulerMethod,
		gstRulerTransient,
		gstRulerUseTransf,
		gstRulerDisp,
		gstRulerRelaxType,
		gstRulerF1,
		gstRulerInterpolation,
		gstAlignCenter,
		gstCurrentRuler,
		gstRulerLocator,
		gstRulerProbe,
		gstRulerLine,
		gstRulerProtractor,
		gstRulerEllipse,
		gstRulerPolyline,
		gstRulerPencil,
		gstRulerGrow,
		gstRulerMove,
		gstRulerMovePoint,
		gstRulerMagnet,
		gstRulerMovePencil,
		gstRulerFlip,
		gstRulerAvg,
		gstRulerLock,
		gstRulerRelax,
		gstRulerDeleteSelection,
		gstRulerDeleteAll,
		gstRulerDeletePoint,
		gstRulerPrune,
		gstRulerProfile,
		gstRulerDistance,
		gstRulerProject,
		gstRulerExport,
		gstRulerNewGroup,
		gstRulerGroupRulers,
		gstRulerToggleGroupDisp,
		gstRulerDeleteKey,
		gstRulerDeleteAllKeys,
		gstRulerToggleDisp,
		gstRulerIntMethod,
		gstRulerDispPoint,
		gstRulerDispLine,
		gstRulerDispName,
		gstRulerDispAll
	};

	RulerListInfo GetRulerListInfo();
	RulerListDisplayInfo GetRulerListDisplayInfo();
	RulerGroupSelectionInfo GetGroupSelectionInfo();
	RulerProfileInfo GetProfileInfo();

	void ToggleDisplay();
	void SetSelectedRulers();
	void SetCurrentRuler(const RulerCurrentInfo& info);

	void SetIntensityMethod();
	void SetTransient();
	void SetUseTransfer();
	void SetDispPoint();
	void SetDispLine();
	void SetDispName();
	void SetDispAll();
	void SetRelaxData();
	void SetRelaxValue();
	void SetGroup();
	void NewGroup();
	void GroupRulers();
	void ToggleGroupDisp();
	void SetInterpolation();
	void DeleteKey();
	void DeleteAllKeys();
	void SetAlignCenter();
	void AlignRuler();
	void AlignPca();

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
};

#endif // MeasureDlgAgent_h
