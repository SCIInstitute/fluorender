/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2018 Scientific Computing and Imaging Institute,
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
#ifndef _VOLUMESELECTOR_H_
#define _VOLUMESELECTOR_H_

#include <FLIVR/Point.h>

class VRenderGLView;
class VolumeData;
class VolumeSelector
{
public:
	VolumeSelector();
	~VolumeSelector();

	void SetView(VRenderGLView* view) { m_view = view; }
	void SetVolume(VolumeData *vd) { m_vd = vd; }
	VolumeData* GetVolume() { return m_vd; }
	void Set2DMask(unsigned int mask) { m_2d_mask = mask; }
	void Set2DWeight(unsigned int weight1, unsigned int weight2)
	{
		m_2d_weight1 = weight1;
		m_2d_weight2 = weight2;
	}
	void SetProjection(double* mvmat, double *prjmat)
	{
		memcpy(m_mvmat, mvmat, 16 * sizeof(double));
		memcpy(m_prjmat, prjmat, 16 * sizeof(double));
	}
	void SetBrushIteration(int num) {m_iter_num = num;}
	int GetBrushIteration() {return m_iter_num;}
	//set/get brush properties
	void SetBrushIniThresh(double val) {m_ini_thresh = val;}
	double GetBrushIniThresh() {return m_ini_thresh;}
	void SetBrushGmFalloff(double val) {m_gm_falloff = val;}
	double GetBrushGmFalloff() {return m_gm_falloff;}
	void SetBrushSclFalloff(double val) {m_scl_falloff = val;}
	double GetBrushSclFalloff() {return m_scl_falloff;}
	void SetBrushSclTranslate(double val) {m_scl_translate = val;}
	double GetBrushSclTranslate() {return m_scl_translate;}
	void SetUseBrushSize2(bool val) {m_use_brush_size2 = val;}
	//w2d
	void SetW2d(double val) {m_w2d = val;}
	double GetW2d() {return m_w2d;}
	//edge detect
	void SetEdgeDetect(bool val) {m_edge_detect = val;}
	bool GetEdgeDetect() {return m_edge_detect;}
	//hidden removal
	void SetHiddenRemoval(bool val) {m_hidden_removal = val;}
	bool GetHiddenRemoval() {return m_hidden_removal;}
	//ortho
	void SetOrthographic(bool val) {m_ortho = val;}
	bool GetOrthographic() {return m_ortho;}
	//select group
	void SetSelectGroup(bool value) {m_select_multi = value?1:0;}
	bool GetSelectGroup() {return m_select_multi==1;}
	//update order
	void SetUpdateOrder(bool bval) { m_update_order = bval; }
	bool GetUpdateOrder() { return m_update_order; }

	//modes
	void SetMode(int mode) {m_mode = mode;}
	int GetMode() {return m_mode;}
	//set use 2d rendering results
	void SetPaintUse2d(bool use2d) {m_use2d = use2d;}
	bool GetPaintUse2d() {return m_use2d;}

	void Segment();
	void Select(double radius);
	void CompExportRandomColor(int hmode, VolumeData* vd_r, VolumeData* vd_g, VolumeData* vd_b, bool select, bool hide);
	VolumeData* GetResult(bool pop);
	//process current selection
	int ProcessSel(double thresh);
	int GetCenter(FLIVR::Point& p);
	int GetSize(double& s);

	//estimate threshold
	void SetEstimateThreshold(bool value)
	{m_estimate_threshold = value;}
	bool GetEstimateThreshold()
	{return m_estimate_threshold;}

private:
	VRenderGLView *m_view;
	VolumeData *m_vd;	//volume data for segmentation
	unsigned int m_2d_mask;	//2d mask from painting
	unsigned int m_2d_weight1;//2d weight map (after tone mapping)
	unsigned int m_2d_weight2;//2d weight map	(before tone mapping)
	double m_mvmat[16];	//modelview matrix
	double m_prjmat[16];//projection matrix
	int m_iter_num;		//iteration number for growing
	int m_mode;			//segmentation modes
						//1-select; 2-append; 3-erase; 4-diffuse; 5-flood; 6-clear; 7-all; 8-solid;
						//image processing modes
						//11-posterize
	bool m_use2d;

	bool m_update_order;

	//brush properties
	double m_ini_thresh;
	double m_gm_falloff;
	double m_scl_falloff;
	double m_scl_translate;
	int m_select_multi;	//0-only current; 1-select group;
	bool m_use_brush_size2;
	bool m_edge_detect;
	bool m_hidden_removal;
	bool m_ortho;
	//w2d
	double m_w2d;

	//exported volumes
	std::vector<VolumeData*> m_result_vols;

	//a random variable
	int m_randv;

	//process selection
	bool m_ps;
	FLIVR::Point m_ps_center;
	double m_ps_size;

	bool m_estimate_threshold;

private:
	double HueCalculation(int mode, unsigned int label);
};

#endif//_VOLUMESELECTOR_H_
