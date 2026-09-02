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
#ifndef TrackDlgAgent_h
#define TrackDlgAgent_h

#include <Agent.h>

class TrackDlg;
namespace flrd
{
	class CelpList;
}
struct TrackViewData;
class TrackDlgAgent : public Agent
{
public:
	TrackDlgAgent(
		TrackDlg* dlg);

	virtual ~TrackDlgAgent() = default;

	// Agent interface
	virtual bool Accept(
		const UpdateRequest& request) const override;

	virtual void Update(
		const UpdateRequest& request) override;

	TrackDlg* GetDialog() const;

private:
	void UpdateUI(const UpdateRequest& request);

	void UpdateData(const UpdateRequest& request);

	std::vector<TrackItem> BuildTrackList(
		const flrd::CelpList& sel_cells,
		bool shuffle);

	TrackViewData GetTrackViewData();

private:
	std::string m_comp_id;//select
	std::string m_comp_id3;//modify / new id
};

#endif // TrackDlgAgent_h
