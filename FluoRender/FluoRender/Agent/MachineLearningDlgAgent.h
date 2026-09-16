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
#ifndef MachineLearningDlgAgent_h
#define MachineLearningDlgAgent_h

#include <Agent.h>
#include <Names.h>
#include <GridData.h>
#include <string>

class MachineLearningDlg;
class MachineLearningDlgAgent : public Agent
{
public:
	MachineLearningDlgAgent(
		MachineLearningDlg* dlg);

	virtual ~MachineLearningDlgAgent() = default;

	MachineLearningDlg* GetDialog() const;

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
	};
};

class MachineLearningPanel;
class MachineLearningPanelAgent : public Agent
{
public:
	MachineLearningPanelAgent(
		MachineLearningPanel* panel);

	virtual ~MachineLearningPanelAgent() = default;

	MachineLearningPanel* GetPanel() const;

	std::string GetTopGridName() { return m_top_grid_name; }
	std::string GetBotGridName() { return m_bot_grid_name; }

	virtual void SetTable(const std::wstring& name) = 0;
	virtual void SetAutoStart(bool bval) = 0;
	virtual void DeleteRecord(const GridSelection& sel) = 0;
	virtual void UpdateCellChanged(const GridCellChanged& cell) = 0;
	virtual void UpdateList(int index);

protected:
	bool m_record = false;//state for recording
	std::wstring m_dir;//dir for searching tables
	std::wstring m_ext;//file extension for tables
	std::wstring m_exepath;//path to executable
	std::string m_top_grid_name;
	std::string m_bot_grid_name;

	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

	virtual void UpdateTopListFromFile();
	virtual void UpdateTopListByName();
	virtual void UpdateBotList() {};
	virtual bool MatchTableName(std::wstring& name);

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	virtual void DelTable() = 0;
	virtual void DupTable() = 0;
	virtual void StartRecording() = 0;
	virtual void ApplyRecord() = 0;
};

class MLCompGenPanel;
class MLCompGenPanelAgent : public MachineLearningPanelAgent
{
public:
	MLCompGenPanelAgent(
		MLCompGenPanel* panel);

	virtual ~MLCompGenPanelAgent();

	MLCompGenPanel* GetPanel() const;

	virtual void SetTable(const std::wstring& name) override;
	virtual void SetAutoStart(bool bval) override;
	virtual void DeleteRecord(const GridSelection& sel) override;
	virtual void UpdateCellChanged(const GridCellChanged& cell) override;

protected:
	std::span < const std::string_view>
		AcceptedValues() const override
	{
		return kAcceptedValues;
	}

	void UpdateUI(const UpdateRequest& request) override;

	void UpdateData(const UpdateRequest& request) override;

	void UpdateBotList() override;

private:
	static constexpr std::string_view kAcceptedValues[] =
	{
		gstCurrentSelect,
	};

	virtual void DelTable() override;
	virtual void DupTable() override;
	virtual void StartRecording() override;
	virtual void ApplyRecord() override;
};

class MLVolPropPanel;
class MLVolPropPanelAgent : public MachineLearningPanelAgent
{
public:
	MLVolPropPanelAgent(
		MLVolPropPanel* panel);

	virtual ~MLVolPropPanelAgent() = default;

	MLVolPropPanel* GetPanel() const;

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
	};
};

#endif // MachineLearningDlgAgent_h
