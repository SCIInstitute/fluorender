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
#ifndef _DATAMANAGER_H_
#define _DATAMANAGER_H_

#include <glm.h>
#include <Color.h>
#include <vector>
#include <string.h>
#include <wx/string.h>

namespace fluo
{
	class VolumeData;
	class MeshData;
	class Annotations;
}
class BaseReader;
class DataManager
{
public:
	DataManager();
	~DataManager();

	void ClearAll();

	//set project path
	//when data and project are moved, use project file's path
	//if data's directory doesn't exist
	void SetProjectPath(wxString path);
	wxString SearchProjectPath(wxString &filename);

	//load volume
	int LoadVolumeData(wxString &filename, int type, bool withImageJ, int ch_num=-1, int t_num=-1);
	//set default
	void SetVolumeDefault(fluo::VolumeData* vd);
	//load volume options
	void SetSliceSequence(bool ss) {m_sliceSequence = ss;}
	void SetChannSequence(bool cs) { m_channSequence = cs; }
	void SetDigitOrder(int order) { m_digitOrder = order; }
	void SetSerNum(int num) { m_ser_num = num; }
	void SetCompression(bool compression) {m_compression = compression;}
	void SetSkipBrick(bool skip) {m_skip_brick = skip;}
	void SetTimeId(wxString str) {m_timeId = str;}
	void SetLoadMask(bool load_mask) {m_load_mask = load_mask;}
	void AddVolumeData(fluo::VolumeData* vd);
	fluo::VolumeData* DuplicateVolumeData(fluo::VolumeData* vd);
	void RemoveVolumeData(int index);
	void RemoveVolumeData(const std::string &name);
	int GetVolumeNum();
	fluo::VolumeData* GetVolumeData(int index);
	fluo::VolumeData* GetVolumeData(const std::string &name);
	int GetVolumeIndex(const std::string &name);
	fluo::VolumeData* GetLastVolumeData();

	//load mesh
	int LoadMeshData(wxString &filename);
	int LoadMeshData(GLMmodel* mesh);
	int GetMeshNum();
	fluo::MeshData* GetMeshData(int index);
	fluo::MeshData* GetMeshData(const std::string &name);
	int GetMeshIndex(const std::string &name);
	fluo::MeshData* GetLastMeshData();

	void RemoveMeshData(int index);

	//annotations
	int LoadAnnotations(wxString &filename);
	void AddAnnotations(fluo::Annotations* ann);
	void RemoveAnnotations(int index);
	int GetAnnotationNum();
	fluo::Annotations* GetAnnotations(int index);
	fluo::Annotations* GetAnnotations(const std::string &name);
	int GetAnnotationIndex(const std::string &name);
	fluo::Annotations* GetLastAnnotations();

	bool CheckNames(wxString &str);

	//wavelength to color
	void SetWavelengthColor(int c1, int c2, int c3, int c4);
	fluo::Color GetWavelengthColor(double wavelength);
	fluo::Color GetColor(int);

	//override voxel size
	void SetOverrideVox(bool val)
	{ m_override_vox = val; }
	bool GetOverrideVox()
	{ return m_override_vox; }

	//flags for pvxml settings
	void SetPvxmlFlipX(bool flip) {m_pvxml_flip_x = flip;}
	bool GetPvxmlFlipX() {return m_pvxml_flip_x;}
	void SetPvxmlFlipY(bool flip) {m_pvxml_flip_y = flip;}
	bool GetPvxmlFlipY() {return m_pvxml_flip_y;}
	void SetPvxmlSeqType(int value) { m_pvxml_seq_type = value; }
	int GetPvxmlSeqType() { return m_pvxml_seq_type; }

public:
	//default values
	//volume
	double m_vol_exb;	//extract_boundary
	double m_vol_gam;	//gamma
	double m_vol_of1;	//offset1
	double m_vol_of2;	//offset2
	double m_vol_lth;	//low_thresholding
	double m_vol_hth;	//high_thresholding
	double m_vol_lsh;	//low_shading
	double m_vol_hsh;	//high_shading
	double m_vol_alf;	//alpha
	double m_vol_spr;	//sample_rate
	double m_vol_xsp;	//x_spacing
	double m_vol_ysp;	//y_spacing
	double m_vol_zsp;	//z_spacing
	double m_vol_lum;	//luminance
	bool m_vol_cmi;		//colormap inversion
	int m_vol_cmp;		//colormap type (rainbow, warm, etc)
	int m_vol_cmm;		//colormap mode (enable)
	int m_vol_cmj;		//colormap projection
	double m_vol_lcm;	//colormap low value
	double m_vol_hcm;	//colormap high value
	bool m_vol_eap;		//enable alpha
	bool m_vol_esh;		//enable_shading
	bool m_vol_interp;	//enable interpolation
	bool m_vol_inv;		//enable inversion
	bool m_vol_mip;		//enable_mip
	bool m_vol_nrd;		//noise reduction
	bool m_vol_shw;		//enable shadow
	double m_vol_swi;	//shadow intensity
	bool m_vol_trp;		//hi transp
	int m_vol_com;		//component display

	bool m_vol_test_wiref;		//wireframe mode

	//wavelength to color table
	fluo::Color m_vol_wav[4];

private:
	std::vector<BaseReader*> m_reader_list;

	bool m_use_defaults;

	//slice sequence
	bool m_sliceSequence;
	//read channels
	bool m_channSequence;
	//digit order
	int m_digitOrder;//0:chann first; 1:slice first
	//series number
	int m_ser_num;
	//compression
	bool m_compression;
	//skip brick
	bool m_skip_brick;
	//time sequence identifier
	wxString m_timeId;
	//load volume mask
	bool m_load_mask;
	//project path
	wxString m_prj_path;
	//override voxel size
	bool m_override_vox;
	//flgs for pvxml flipping
	bool m_pvxml_flip_x;
	bool m_pvxml_flip_y;
	int m_pvxml_seq_type;
};

#endif//_DATAMANAGER_H_
