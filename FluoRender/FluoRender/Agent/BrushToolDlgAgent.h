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
#include <Names.h>

class BrushToolDlg;

class BrushToolDlgAgent : public Agent
{
public:
	BrushToolDlgAgent(
		BrushToolDlg* dlg);

	virtual ~BrushToolDlgAgent() = default;

	// Agent interface
	virtual void Update(
		const UpdateRequest& request) override;

	BrushToolDlg* GetDialog() const;

	//settings
	void SetBrushSclTranslate(double dval);
	void SetBrushGmFalloff(double dval);
	void SetW2d(double dval);
	void SetEdgeDetect(bool bval);
	void SetHiddenRemoval(bool bval);
	void SetSelectGroup(bool bval);
	void SetUpdateOrder(bool bval);
	void SetBrushSize1(double dval);
	void SetBrushSize2Enable(bool bval, double dval1, double dval2);
	void SetBrushSize2(double dval);
	void SetBrushIteration(int ival);
	void SetBrushSizeData(bool bval);
	void SetAlignCenter(bool bval);
	void SetAlignAxis(int ival);

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

private:
	void UpdateUI(const UpdateRequest& request);

	void UpdateData(const UpdateRequest& request);

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
		//timer
		gstTimerSegment
	};

	//max volume value
	double m_max_value{ 255.0 };

};

#endif // BrushToolDlgAgent_h
