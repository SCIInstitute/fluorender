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
#ifndef ClipPlanePanelAgent_h
#define ClipPlanePanelAgent_h

#include <Agent.h>
#include <Names.h>
#include <memory>

namespace fluo
{
	enum class ClipPlane : int;
	class BBox;
}
class ClipPlanePanel;
class TreeLayer;
class ClipPlanePanelAgent : public Agent
{
public:
	ClipPlanePanelAgent(
		ClipPlanePanel* dlg);

	virtual ~ClipPlanePanelAgent() = default;

	ClipPlanePanel* GetPanel() const;

	void SetClipValue(fluo::ClipPlane i, int val, bool link = false);//index: 0~5 = X1~Z2
	void SetClipValues(fluo::ClipPlane i, int val1, int val2);//index: clip mask
	void SetClipValues(const std::array<int, 6>& vals);
	void ResetClipValues();
	void ResetClipValues(fluo::ClipPlane i);
	void SyncClipValue(int i);
	void UpdateClipIdle(bool bval);
	void SetLinkedDist(fluo::ClipPlane i, int val);
	void SetPlaneMask(int val);

	void SetClipRotX(double val);
	void SetClipRotY(double val);
	void SetClipRotZ(double val);

	void SetClipDistX(bool use_val = false, int val = 0);
	void SetClipDistY(bool use_val = false, int val = 0);
	void SetClipDistZ(bool use_val = false, int val = 0);

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
		gstMultiFuncTips,
		gstClipLinkChan,
		gstClipHold,
		gstClipPlaneMode,
		gstClipPlaneRanges,
		gstClipPlaneRangeColor,
		gstClipDist,
		gstClipX1,
		gstClipX2,
		gstClipY1,
		gstClipY2,
		gstClipZ1,
		gstClipZ2,
		gstClipLinkX,
		gstClipLinkY,
		gstClipLinkZ,
		gstClipRotX,
		gstClipRotY,
		gstClipRotZ,
		gstClipSetZero,
		gstClipRotReset,
		gstClipRotResetX,
		gstClipRotResetY,
		gstClipRotResetZ,
		gstVolumeSampleRate
	};

	std::shared_ptr<TreeLayer> GetObject();

	void LinkChannels();
	void HoldPlanes();
	void SetPlaneMode();
	void SetClipZero();
	void RotReset();
	void RotResetAxis(int i);
	void SyncVolumeSampleRate();
};

#endif // ClipPlanePanelAgent_h
