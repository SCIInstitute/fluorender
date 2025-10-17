/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
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
#ifndef _CURRENT_OBJECTS_H_
#define _CURRENT_OBJECTS_H_

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

namespace flrd
{
	class Vertex;
	typedef std::shared_ptr<Vertex> Verp;
	typedef std::unordered_map<unsigned int, Verp> VertexList;
	class Ruler;
	class RulerList;
	typedef std::vector<Ruler*>::iterator RulerListIter;
	class EntryParams;
	class TrackMap;
	typedef std::shared_ptr<TrackMap> pTrackMap;
	class CelpList;
}
class MainFrame;
class RenderView;
class VolumeGroup;
class MeshGroup;
class VolumeData;
class MeshData;
class AnnotData;
class TrackGroup;
struct CurrentObjects
{
	CurrentObjects() :
		mainframe(0)
	{}

	//0:root, 1:view, 2:volume, 3:mesh, 4:annotations, 5:group, 6:mesh group, 7:ruler, 8:traces
	int GetType()
	{
		if (vol_data.lock())
			return 2;
		if (mesh_data.lock())
			return 3;
		if (ann_data.lock())
			return 4;
		if (vol_group.lock())
			return 5;
		if (mesh_group.lock())
			return 6;
		if (render_view.lock())
			return 1;
		return 0;
	}
	void SetRoot()
	{
		render_view.reset();
		vol_group.reset();
		mesh_group.reset();
		vol_data.reset();
		mesh_data.reset();
		ann_data.reset();
	}
	void SetRenderView(const std::shared_ptr<RenderView>& v);
	void SetVolumeGroup(const std::shared_ptr<VolumeGroup>& g);
	void SetMeshGroup(const std::shared_ptr<MeshGroup>& g);
	void SetVolumeData(const std::shared_ptr<VolumeData>& vd);
	void SetMeshData(const std::shared_ptr<MeshData>& md);
	void SetAnnotData(const std::shared_ptr<AnnotData>& ann);

	void SetSel(const std::wstring& str);

	int GetViewId(RenderView* v = 0);
	int GetCurViewId();

	flrd::RulerList* GetRulerList();
	flrd::Ruler* GetRuler();
	TrackGroup* GetTrackGroup();

	MainFrame* mainframe;//this is temporary before a global scenegraph is added
	std::weak_ptr<RenderView> render_view;
	std::weak_ptr<VolumeGroup> vol_group;
	std::weak_ptr<MeshGroup> mesh_group;
	std::weak_ptr<VolumeData> vol_data;
	std::weak_ptr<MeshData> mesh_data;
	std::weak_ptr<AnnotData> ann_data;
};

#endif//_CURRENT_OBJECTS_H_