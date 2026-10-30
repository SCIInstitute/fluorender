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
#ifndef BrushToolDlgAgent_h
#define BrushToolDlgAgent_h

#include <Agent.h>
#include <Tokens.h>
#include <AsyncTimer.hpp>

class BrushToolDlg;

class BrushToolDlgAgent : public Agent
{
public:
	BrushToolDlgAgent(
		BrushToolDlg* dlg);

	virtual ~BrushToolDlgAgent() = default;

	BrushToolDlg* GetDialog() const;

	virtual void ApplySettings() override;
	virtual void PreShutdown() override;

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
		//ui
		gstCurrentSelect,
		gstSelUndo,
		gstSelRedo,
		gstFreehandToolState,
		gstSelMask,
		gstSelOptions,
		gstBrushThreshold,
		gstBrushGmFalloff,
		gstBrush2dInf,
		gstBrushSize1,
		gstBrushSize2,
		gstBrushIter,
		gstBrushSizeRel,
		gstAlignCenter,
		gstBrushHistoryEnable,
		gstBrushCountResult,
		gstBrushCountAutoUpdate,
		gstBrushSpeedResult,
		//timer
		gstTimerSegment
		//data
		gstBrushGrow,
		gstBrushAppend,
		gstBrushComp,
		gstBrushMesh,
		gstBrushSingle,
		gstBrushDiffuse,
		gstBrushSolid,
		gstBrushUnsel,
		gstBrushClear,
		gstBrushExtract,
		gstBrushDelete,
		gstMaskCopy,
		gstMaskCopyData,
		gstMaskPaste,
		gstMaskMerge,
		gstMaskExclude,
		gstMaskIntersect,
		gstAlignPca,
		gstEdgeDetect,
		gstHiddenRemoval,
		gstSelectGroup,
		gstUpdateOrder,
		gstBrushSize2Enable,
		gstAlignAxis
	};

	//max volume value
	double m_max_value{ 255.0 };

	fluo::AsyncTimer m_segment_timer;

private:
	void SelUndo();
	void SelRedo();

	void BrushGrow();
	void BrushAppend();
	void BrushComp();
	void BrushMesh();
	void BrushSingle();
	void BrushDiffuse();
	void BrushSolid();
	void BrushUnsel();
	void BrushClear();
	void BrushExtract();
	void BrushDelete();

	void MaskCopy();
	void MaskCopyData();
	void MaskPaste(int mode);
	void AlignPca();
	void TimerSegment();

	void SetBrushSclTranslate();
	void SetBrushGmFalloff();
	void SetW2d();
	void SetEdgeDetect();
	void SetHiddenRemoval();
	void SetSelectGroup();
	void SetUpdateOrder();
	void SetBrushSize1();
	void SetBrushSize2Enable();
	void SetBrushSize2();
	void SetBrushIteration();
	void SetBrushSizeData();
	void SetAlignCenter();
	void SetAlignAxis();

};

#endif // BrushToolDlgAgent_h
