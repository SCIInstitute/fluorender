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
#ifndef _TRACKAGENT_H_
#define _TRACKAGENT_H_

#include <InterfaceAgent.hpp>
#include <Renderview.hpp>
#include <Tracking/Cell.h>
#include <Tracking/VolCache.h>

class TrackDlg;
namespace fluo
{
	class TrackAgent : public InterfaceAgent
	{
	public:
		TrackAgent(TrackDlg &dlg);

		virtual bool isSameKindAs(const Object* obj) const
		{
			return dynamic_cast<const TrackAgent*>(obj) != NULL;
		}

		virtual const char* className() const { return "TrackAgent"; }

		virtual void setObject(Renderview* view);
		virtual Renderview* getObject();

		virtual void UpdateAllSettings();

		virtual TrackAgent* asTrackAgent() { return this; }
		virtual const TrackAgent* asTrackAgent() const { return this; }

		friend class AgentFactory;

		void UpdateTraces();
		void UpdateList();

		//cell operations
		void AddLabel(long item, TraceListCtrl* trace_list_ctrl, flrd::CelpList &list);
		void CellUpdate();
		void CellFull();
		void CellLink(bool exclusive);
		void CellNewID(bool append);
		void CellEraseID();
		void CellReplaceID();
		void CellCombineID();
		void CompDelete();
		void CompClear();
		//uncertain filtering
		void UncertainFilter(bool input = false);
		//link for external call
		void LinkAddedCells(flrd::CelpList &list);

		//measurement
		void SaveOutputResult(wxString &filename);

		//automatic tracking
		void GenMap();
		void RefineMap(int t = -1, bool erase_v = true);

		//track map file
		int GetTrackFileExist(bool save);//0:no trace group; 1:trace groups exists not saved; 2:saved
		std::wstring GetTrackFile();
		void LoadTrackFile(const std::wstring &file);
		void SaveTrackFile(const std::wstring &file);


	protected:
		TrackDlg &dlg_;

	private:
		//read/delete volume cache from file
		void ReadVolCache(flrd::VolCache& vol_cache);
		void DelVolCache(flrd::VolCache& vol_cache);

	};
}

#endif//_TRACKAGENT_H_