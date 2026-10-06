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
#include <Names.h>

namespace flrd
{
	class CelpList;
}
class TrackDlg;
struct TrackItem;
struct TrackViewData;
class TrackDlgAgent : public Agent
{
public:
	TrackDlgAgent(
		TrackDlg* dlg);

	virtual ~TrackDlgAgent() = default;

	TrackDlg* GetDialog() const;

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

private:
	std::vector<TrackItem> BuildTrackList(
		const flrd::CelpList& sel_cells,
		bool shuffle);

	TrackViewData GetTrackViewData();

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstTrackFile,
		gstTrackIter,
		gstTrackSize,
		gstTrackSimilarity,
		gstTrackContactFactor,
		gstTrackConsistent,
		gstTrackMerge,
		gstTrackSplit,
		gstTrackCompId,
		gstTrackCellSize,
		gstTrackUncertainLow,
		gstTrackNewCompId,
		gstTrackClusterNum,
		gstGhostNum,
		gstGhostEnable,
		gstTrackList,
		gstClearTrack,
		gstSaveTrackFile,
		gstSaveAsTrackFile,
		gstGenerateMap,
		gstRefineTime,
		gstRefineAll,
		gstTrackClearCompId,
		gstCompFull,
		gstCompExclusive,
		gstCompAppend,
		gstCompClear,
		gstShuffle,
		gstComputeUncertainty,
		gstTrackCompId2,
		gstCellExclusiveLink,
		gstCellLink,
		gstCellLinkAll,
		gstCellIsolate,
		gstCellUnlink,
		gstTrackNewCompId,
		gstTrackClearCompNewId,
		gstCreateCellNewId,
		gstCellAppendId,
		gstCellReplaceId,
		gstCellCombineId,
		gstCellSeparateId,
		gstCellSegment,
		gstCellClusterNum,
		gstTrackConvertRulers,
		gstTrackConsistent,
		gstAnalyzeComps,
		gstAnalyzeLinks,
		gstAnalyzeUncertainty,
		gstAnalyzePaths,
		gstTrackSaveResult,
		gstCellPrev,
		gstCellNext,
		gstGhostShowTail,
		gstGhostShowLead,
		gstTrackListSel,
		gstTrackListDelete
	};

	std::string m_comp_id;//select
	std::string m_comp_id3;//modify / new id

private:
	void WriteInfo(const std::wstring& str);

	void ClearTrack();
	void LoadTrackFile();
	void SaveTrackFile();
	void SaveAsTrackFile();
	void GenerateMap();
	void RefineTime();
	void RefineAll();
	void SetMapIter();
	void SetMapSize();
	void SetMapConsistent();
	void SetTryMerge();
	void SetTrySplit();
	void SetMapSimilarity();
	void SetMapContact();
	void SetCompId();
	void ClearCompId();
	void SetCompFull();
	void SetCompExclusive();
	void SetCompAppend();
	void SetCompClear();
	void SetShuffle();

	void SetCellSize();
	void ComputeUncertainty();
	void SetCompUncertaintyLow();
	void SetCompId2();
	void SetCellExclusiveLink();
	void SetCellLink();
	void SetCellLinkAll();
	void SetCellIsolate();
	void SetCellUnlink();
	void SetCellNewId();
	void ClearCompNewId();
	void CreateCellNewId();
	void SetCellAppendId();
	void SetCellReplaceId();
	void SetCellCombineId();
	void SetCellSeparateId();
	void SetCellSegment();
	void SetClusterNum();

	void ConvertRulers();
	void ConvertConsistent();
	void AnalyzeComps();
	void AnalyzeLinks();
	void AnalyzeUncertainty();
	void AnalyzePaths();
	void SaveTrackResult();
	
	void SetCellPrev();
	void SetCellNext();
	void SetGhostNum();
	void SetGhostShowTail();
	void SetGhostShowLead();
	void SetListSelection();

	void DeleteSelection();

};

#endif // TrackDlgAgent_h
