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
#ifndef ComponentDlgAgent_h
#define ComponentDlgAgent_h

#include <Agent.h>
#include <Tokens.h>
#include <GridData.h>
#include <AsyncTimer.hpp>
#include <wx/wx.h>
#include <set>

class ComponentDlg;
class ComponentDlgAgent : public Agent
{
public:
	ComponentDlgAgent(
		ComponentDlg* dlg);

	virtual ~ComponentDlgAgent() = default;

	ComponentDlg* GetDialog() const;

	virtual void ApplySettings() override;
	virtual void PreShutdown() override;

	int GetMaxLines() { return m_max_lines; }

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
		gstUseSelection,
		gstUseMachineLearning,
		gstIteration,
		gstCompThreshold,
		gstUseDiffusion,
		gstDiffusionFalloff,
		gstUseDensityField,
		gstDensityFieldThresh,
		gstDensityVarThresh,
		gstDensityWindowSize,
		gstDensityStatsSize,
		gstUseDistField,
		gstDistFieldStrength,
		gstDistFieldFilterSize,
		gstMaxDist,
		gstDistFieldThresh,
		gstFixateEnable,
		gstGrowFixed,
		gstFixateSize,
		gstCleanEnable,
		gstCleanIteration,
		gstCleanSize,
		gstRecordCmd,
		gstClusterMethod,
		gstClusterNum,
		gstClusterMaxIter,
		gstClusterTol,
		gstClusterSize,
		gstClusterEps,
		gstCompIdColor,
		gstUseMin,
		gstMinValue,
		gstUseMax,
		gstMaxValue,
		gstCompConsistent,
		gstCompColocal,
		gstCompOutputType,
		gstDistNeighbor,
		gstDistNeighborValue,
		gstDistAllChan,
		gstAlignCenter,
		gstCompGenOutput,
		gstCompAnalysisResult,
		gstCompListSelection,
		gstCompGenerate,
		gstCompCluster,
		gstCompAnalyze,
		gstFixUpdate,
		gstCleanUpdate,
		gstPlayCmd,
		gstResetCmd,
		gstLoadCmd,
		gstSaveCmd,
		gstCompFull,
		gstCompExclusive,
		gstCompAppend,
		gstCompAll,
		gstCompClear,
		gstShuffle,
		gstCompNew,
		gstCompAdd,
		gstCompReplace,
		gstCompClearBkg,
		gstCompCombine,
		gstCompOutputMesh,
		gstCompOutputDist
		gstCompId,
		gstConsistentSize,
		gstCompOutputChannels,
		gstCompOutputAnnotData,
		gstAlignPca,
		gstIncludeComps,
		gstExcludeComps,
		gstCompSelectionChanged
	};

	int m_max_lines = 1000;

	fluo::AsyncTimer m_comp_gen_timer;

private:
	void SetIter();
	void SetThresh();
	void SetUseDistField();
	void SetDistStrength();
	void SetDistFilterSize();
	void SetMaxDist();
	void SetDistThresh();
	void SetUseDiff();
	void SetFalloff();
	void SetUseDensity();
	void SetDensity();
	void SetVarth();
	void SetDensityWindowSize();
	void SetDensityStatsSize();
	void SetFixate();
	void FixUpdate();
	void SetFixSize();
	void SetClean();
	void CleanUpdate();
	void SetCleanIter();
	void SetCleanLimit();
	void SetRecord();
	void PlayCmdUpdate();
	void ResetCmdUpdate();
	void LoadCmdUpdate();
	void SaveCmdUpdate();

	void SetClusterMethod();
	void SetClusterClnum();
	void SetClusterMaxiter();
	void SetClusterTol();
	void SetClusterSize();
	void SetClustereps();

	void SetCompId();
	void SetUseMin();
	void SetMin();
	void SetUseMax();
	void SetMax();

	void CompFull();
	void CompExclusive();
	void CompAppend();
	void CompAll();
	void CompClear();
	void Shuffle();
	void CompNew();
	void CompAdd();
	void CompReplace();
	void CompCleanBkg();
	void CompCombine();

	void SetConSize();
	void SetConsistent();
	void SetColocal();
	void SetOutputType();
	void SetOutputChannels();
	void SetOutputAnnotData();
	void CompOutputMesh();

	void SetUseDistNeighbor();
	void SetDistAllChan();
	void SetDistNeighbor();
	void OutputDistance();

	void SetAlignCenter();
	void SetAlignPca();

	void SetUseSel();
	void SetUseMl();

	void IncludeComps();
	void IncludeComps(const GridSelection& sel);
	void ExcludeComps();
	void ExcludeComps(const GridSelection& sel);
	void GridSelectionChanged();
	void GridSelectionChanged(
		const GridSelection& selection);

	void TimerGenerateComps();
	void CompCluster();
	void CompAnalyze();



	void OutputCompAnalysisResult();

	void UpdateSelectionData(
		const GridSelection& selection);

	std::set<int> FindRowsForIds(
		const std::set<unsigned long long>& ids);

	void UpdateGridSelection();
};

#endif // ComponentDlgAgent_h
