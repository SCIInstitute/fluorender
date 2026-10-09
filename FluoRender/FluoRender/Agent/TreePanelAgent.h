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
#ifndef TreePanelAgent_h
#define TreePanelAgent_h

#include <Agent.h>
#include <Names.h>
#include <memory>

class TreePanel;
class RenderView;
class TreeLayer;
class VolumeData;
class MeshData;
class AnnotData;
class VolumeGroup;
class MeshGroup;
struct MenuItemData;
using MenuData = std::vector<MenuItemData>;
using TreeNodeId = std::uintptr_t;
struct TreeItemData;
struct TreeUpdateData;
struct TreeIconUpdateData;
struct TreeColorUpdateData;
struct TreeSelectionData;

class TreePanelAgent : public Agent
{
public:
	TreePanelAgent(
		TreePanel* panel);

	virtual ~TreePanelAgent() = default;

	TreePanel* GetPanel() const;

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
		gstTreeCtrl,
		gstTreeLayerName,
		gstTreeIcons,
		gstTreeColors,
		gstCurrentSelect,
		gstFreehandToolState,
		gstTreeContextMenu,
		gstTreeExpandSelItem,
		gstTreeScrollPos,
		gstTreeAction,
		gstAddVolumeGroup,
		gstAddMeshGroup,
		gstRemoveData,
		gstRulerLocator,
		gstRulerLine,
		gstRulerPolyline,
		gstRulerPencil,
		gstRulerMovePoint,
		gstRulerDeletePoint,
		gstBrushLocator,
		gstBrushGrow,
		gstBrushAppend,
		gstMeshConvert,
		gstBrushComp,
		gstBrushDiffuse,
		gstBrushUnsel,
		gstBrushClear,
		gstBrushExtract,
		gstBrushDelete,
		gstRandomizeColor,
		gstCloseView,
		gstIsolate,
		gstShowAll,
		gstCopyMask,
		gstPasteMask,
		gstMergeMask,
		gstExcludeMask,
		gstIntersectMask,
		gstBrushToolDlg,
		gstMeasureDlg,
		gstComponentDlg,
		gstTrackDlg,
		gstCalculationDlg,
		gstNoiseCancellingDlg,
		gstCountingDlg,
		gstColocalizationDlg,
		gstConvertDlg,
		gstOclDlg,
		gstMachineLearningDlg,
		gstManipPropPanel,
		gstTreeSelection,
		gstTreeDrag
	};

private:
	//helpers for update
	template<class T>
	TreeNodeId GetNodeId(
		const std::shared_ptr<T>& obj) const
	{
		return reinterpret_cast<TreeNodeId>(obj.get());
	}

	TreeUpdateData BuildTreeData();
	TreeIconUpdateData BuildTreeIconData();
	TreeColorUpdateData BuildTreeColorData();
	TreeSelectionData BuildTreeSelectionData();
	void BuildViewNode(std::shared_ptr<RenderView> view, TreeItemData& node);
	void BuildLayerNode(std::shared_ptr<TreeLayer> layer, TreeItemData& node);
	void BuildVolumeNode(std::shared_ptr<VolumeData> vd, TreeItemData& node);
	void BuildMeshNode(std::shared_ptr<MeshData> md, TreeItemData& node);
	void BuildAnnotNode(std::shared_ptr<AnnotData> ann, TreeItemData& node);
	void BuildVolumeGroupNode(std::shared_ptr<VolumeGroup> group, TreeItemData& node);
	void BuildMeshGroupNode(std::shared_ptr<MeshGroup> group, TreeItemData& node);
	void CollectIconUpdates(std::shared_ptr<TreeLayer> layer, TreeIconUpdateData& data);
	void CollectColorUpdates(std::shared_ptr<TreeLayer> layer, TreeColorUpdateData& data);

	void ShowContextMenu();

	//double click
	void Action();
	void AddVolGroup();
	void AddMeshGroup();
	void RemoveData();
	void RulerLocator();
	void RulerLine();
	void RulerPolyline();
	void RulerPencil();
	void RulerEdit();
	void RulerDeletePoint();
	void BrushRuler();
	void BrushGrow();
	void BrushAppend();
	void BrushComp();
	void BrushDiffuse();
	void BrushUnselect();
	void BrushClear();
	void BrushExtract();
	void BrushDelete();
	void MeshConvert();

	//delete all
	void DeleteAll();

	//menu operations
	void ToggleDisplay();
	void RandomizeColor();
	void CloseView();
	void Isolate();
	void ShowAll();

	void CopyMask();
	void PasteMask(int ival);

	//selection change
	void Select();
	//drag
	void ProcessDrag();
};

#endif // TreePanelAgent_h
