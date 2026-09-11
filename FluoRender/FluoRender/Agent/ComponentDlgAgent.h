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
#include <Names.h>
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

	int GetMaxLines() { return m_max_lines; }

	void SetIter(int ival);
	void SetThresh(double dval);
	void SetUseDistField(bool bval);
	void SetDistStrength(double dval);
	void SetDistFilterSize(int ival);
	void SetMaxDist(int ival);
	void SetDistThresh(double dval);
	void SetUseDiff(bool bval);
	void SetFalloff(double dval);
	void SetUseDensity(bool bval);
	void SetDensity(double dval);
	void SetVarth(double dval);
	void SetDensityWindowSize(int ival);
	void SetDensityStatsSize(int ival);
	void SetFixSize(int ival);
	void SetCleanIter(int ival);
	void SetCleanLimit(int ival);

	void SetFixate(bool bval);
	void SetClean(bool bval);
	void SetRecord(bool bval);

	void SetClusterMethod(int ival);
	void SetClusterClnum(int ival);
	void SetClusterMaxiter(int ival);
	void SetClusterTol(double dval);
	void SetClusterSize(int ival);
	void SetClustereps(double dval);

	void SetCompId(const std::string& str);
	void SetUseMin(bool bval);
	void SetMin(int ival);
	void SetUseMax(bool bval);
	void SetMax(int ival);

	void SetConSize(int ival);
	void SetConsistent(bool bval);
	void SetColocal(bool bval);
	void SetOutputType(int ival);
	void SetOutputChannels(int ival);
	void SetOutputAnnotData(int ival);

	void SetUseDistNeighbor(bool bval);
	void SetDistAllChan(bool bval);
	void SetDistNeighbor(int ival);

	void SetAlignCenter(bool bval);
	void SetAlignPca(int ival);

	void SetUseSel(bool bval);
	void SetUseMl(bool bval);

	void IncludeComps(const GridSelection& sel);
	void ExcludeComps(const GridSelection& sel);

	void GridSelectionChanged(
		const GridSelection& selection);

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
	};

	int m_max_lines = 1000;

	fluo::AsyncTimer m_comp_gen_timer;

private:
	void TimerGenerateComps();
	void CompCluster();
	void CompAnalyze();

	void FixUpdate();
	void CleanUpdate();
	void PlayCmdUpdate();
	void ResetCmdUpdate();
	void LoadCmdUpdate();
	void SaveCmdUpdate();

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

	void CompOutputMesh();

	void OutputDistance();

	void OutputCompAnalysisResult();

	void UpdateSelectionData(
		const GridSelection& selection);

	std::set<int> FindRowsForIds(
		const std::set<unsigned long long>& ids);

	void UpdateGridSelection();
};

#endif // ComponentDlgAgent_h
