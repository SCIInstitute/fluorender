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
#ifndef OutputAdjPanelAgent_h
#define OutputAdjPanelAgent_h

#include <Agent.h>
#include <memory>

class OutputAdjPanel;
class OutputAdjPanelAgent : public Agent
{
public:
	OutputAdjPanelAgent(
		OutputAdjPanel* dlg);

	virtual ~OutputAdjPanelAgent() = default;

	// Agent interface
	virtual bool Accept(
		const UpdateRequest& request) const override;

	virtual void Update(
		const UpdateRequest& request) override;

	OutputAdjPanel* GetPanel() const;

private:
	void UpdateUI(const UpdateRequest& request);

	void UpdateData(const UpdateRequest& request);

	void UpdateSync();
	void SetSync(int i, bool val, bool update = true);
	void SetGamma(int i, double val, bool update = true);
	void SetBrightness(int i, double val, bool update = true);
	void SetHdr(int i, double val, bool update = true);

	void SyncColor(fluo::Color& c, double val);
	void SyncGamma(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify);
	void SyncBrightness(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify);
	void SyncHdr(fluo::Color& c, int i, double val, fluo::ValueCollection& vc, bool notify);
	void SyncGamma(int i);
	void SyncBrightness(int i);
	void SyncHdr(int i);

private:
	bool m_enable_all = true;
	//sync flags
	bool m_sync[3] = { true, true, true };//for rgb

};

#endif // OutputAdjPanelAgent_h
