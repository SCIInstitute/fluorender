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
#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <Progress.h>
#include <Color.h>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

#define LOAD_TYPE_IMAGEJ	0
#define LOAD_TYPE_NRRD		1
#define LOAD_TYPE_TIFF		2
#define LOAD_TYPE_OIB		3
#define LOAD_TYPE_OIF		4
#define LOAD_TYPE_LSM		5
#define LOAD_TYPE_PVXML		6
#define LOAD_TYPE_BRKXML	7
#define LOAD_TYPE_CZI		8
#define LOAD_TYPE_ND2		9
#define LOAD_TYPE_LIF		10
#define LOAD_TYPE_LOF		11
#define LOAD_TYPE_MPG		12
#define LOAD_TYPE_PNG		13
#define LOAD_TYPE_JPG		14
#define LOAD_TYPE_DCM		15
#define LOAD_TYPE_JP2		16

class MainFrame;
class Root;
class VolumeData;
class MeshData;
class AnnotData;
class BaseReader;
namespace flvr
{
	class CacheQueue;
}
struct _GLMmodel;
typedef struct _GLMmodel GLMmodel;
class DataManager : public Progress
{
public:
	DataManager();
	~DataManager();

	void SetFrame(MainFrame* frame);

	void ClearAll();

	//set project path
	//when data and project are moved, use project file's path
	//if data's directory doesn't exist
	void SetProjectPath(const std::wstring& path);
	std::wstring SearchProjectPath(const std::wstring &filename);
	std::wstring GetProjectFile();

	//root
	Root* GetRoot()
	{
		return m_root.get();
	}

	//load volume
	void LoadVolumes(const std::vector<std::wstring>& files, bool withImageJ);
	void StartupLoad(const std::vector<std::wstring>& files, bool run_mov, bool with_imagej);
	size_t LoadVolumeData(const std::wstring &filename, int type, bool withImageJ, int ch_num=-1, int t_num=-1);
	//set default
	void SetVolumeDefault(const std::shared_ptr<VolumeData>& vd);
	void AddVolumeData(const std::shared_ptr<VolumeData>& vd);
	std::shared_ptr<VolumeData> DuplicateVolumeData(const std::shared_ptr<VolumeData>& vd);
	void RemoveVolumeData(size_t index);
	void RemoveVolumeData(const std::wstring &name);
	size_t GetVolumeNum();
	std::shared_ptr<VolumeData> GetVolumeData(size_t index);
	std::shared_ptr<VolumeData> GetVolumeData(const std::wstring &name);
	size_t GetVolumeIndex(const std::wstring &name);
	std::shared_ptr<VolumeData> GetLastVolumeData();

	//load mesh
	void LoadMeshes(const std::vector<std::wstring>& files);
	bool LoadMeshData(const std::wstring &filename);
	bool LoadMeshData(GLMmodel* mesh);
	void AddMeshData(const std::shared_ptr<MeshData>& md);
	size_t GetMeshNum();
	std::shared_ptr<MeshData> GetMeshData(size_t index);
	std::shared_ptr<MeshData> GetMeshData(const std::wstring &name);
	size_t GetMeshIndex(const std::wstring &name);
	std::shared_ptr<MeshData> GetLastMeshData();
	void RemoveMeshData(size_t index);
	void ClearMeshSelection();

	//annotations
	bool LoadAnnotData(const std::wstring &filename);
	void AddAnnotData(const std::shared_ptr<AnnotData>& ann);
	void RemoveAnnotData(size_t index);
	size_t GetAnnotNum();
	std::shared_ptr<AnnotData> GetAnnotData(size_t index);
	std::shared_ptr<AnnotData> GetAnnotData(const std::wstring &name);
	size_t GetAnnotIndex(const std::wstring &name);
	std::shared_ptr<AnnotData> GetLastAnnotData();

	bool CheckNames(const std::wstring &str);

	//wavelength to color
	fluo::Color GetWavelengthColor(double wavelength);
	fluo::Color GetColor(int);

	//get vol cache queue
	flvr::CacheQueue* GetCacheQueue(VolumeData* vd);

	//update stream rendering mode
	void UpdateStreamMode(double data_size);//input is a newly loaded data size in mb

private:
	MainFrame* m_frame;
	std::unique_ptr<Root> m_root;// root of the scene graph
	std::vector<std::shared_ptr<VolumeData>> m_vd_list;
	std::vector<std::shared_ptr<MeshData>> m_md_list;
	std::vector<std::shared_ptr<BaseReader>> m_reader_list;
	std::vector<std::shared_ptr<AnnotData>> m_ad_list;

	//4d cache for volume data
	std::unordered_map<VolumeData*, std::shared_ptr<flvr::CacheQueue>> m_vd_cache_queue;
	//project path
	std::wstring m_prj_path;
	std::wstring m_prj_file;

	//for reading files and channels
	size_t m_cur_file;
	size_t m_file_num;
};

#endif//_DATAMANAGER_H_
