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
#ifndef _VOLUMESELECTOR_H_
#define _VOLUMESELECTOR_H_

#include <Vector.h>
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>

class VolumeData;
namespace flrd
{
	enum class SelectMode : int
	{
		None,			//0-no selection
		SingleSelect,	//1-select;
		Append,			//2-append;
		Eraser,			//3-unselect;
		Diffuse,		//4-diffuse;
		Flood,			//5-flood;
		Clear,			//6-unselect all;
		SelectAll,		//7-select all;
		Solid,			//8-solid;
		Grow,			//9-grow from point;
		Segment,		//10-select and gen comps
		Mesh			//11-select and gen mesh
	};

	struct BrushRadiusSet
	{
		SelectMode type;//brush type
		double radius1;//radius 1
		double radius2;//radius 2
		bool use_radius2;//use radius 2
		int iter_num;//iteration number
	};

	class VolumeSelector
	{
	public:
		VolumeSelector();
		~VolumeSelector();

		bool GetAutoPaintSize();

		void SetVolume(const std::shared_ptr<VolumeData>& vd) { m_vd = vd; }
		//modes
		void SetSelectMode(SelectMode mode);
		SelectMode GetSelectMode() { return m_mode; }
		//init mask
		void SetInitMask(int val) { m_init_mask = val; }
		int GetInitMask() { return m_init_mask; }
		//
		void Set2DMask(unsigned int mask) { m_2d_mask = mask; }
		void Set2DWeight(unsigned int weight1, unsigned int weight2)
		{
			m_2d_weight1 = weight1;
			m_2d_weight2 = weight2;
		}
		void SetProjection(double* mvmat, double *prjmat)
		{
			std::memcpy(m_mvmat, mvmat, 16 * sizeof(double));
			std::memcpy(m_prjmat, prjmat, 16 * sizeof(double));
		}
		void SetBrushIteration(int num)
		{
			m_iter_num = num;
			UpdateBrushRadiusSet();
		}
		int GetBrushIteration() { return m_iter_num; }
		//set/get brush properties
		void SetBrushIniThresh(double val) { m_ini_thresh = val; }
		double GetBrushIniThresh() { return m_ini_thresh; }
		void SetBrushGmFalloff(double val) { m_gm_falloff = val; }
		double GetBrushGmFalloff() { return m_gm_falloff; }
		void SetBrushSclFalloff(double val) { m_scl_falloff = val; }
		double GetBrushSclFalloff() { return m_scl_falloff; }
		void SetBrushSclTranslate(double val) { m_scl_translate = val; }
		double GetBrushSclTranslate() { return m_scl_translate; }
		//w2d
		void SetW2d(double val) { m_w2d = val; }
		double GetW2d() { return m_w2d; }
		//edge detect
		void SetEdgeDetect(bool val) { m_edge_detect = val; }
		bool GetEdgeDetect() { return m_edge_detect; }
		//hidden removal
		void SetHiddenRemoval(bool val) { m_hidden_removal = val; }
		bool GetHiddenRemoval() { return m_hidden_removal; }
		//ortho
		void SetOrthographic(bool val) { m_ortho = val; }
		bool GetOrthographic() { return m_ortho; }
		//select group
		void SetSelectGroup(bool value) { m_select_multi = value; }
		bool GetSelectGroup() { return m_select_multi; }
		//iteration number
		int GetIter() { return m_iter; }
		//update order
		void SetUpdateOrder(bool bval) { m_update_order = bval; }
		bool GetUpdateOrder() { return m_update_order; }
		//brush properties
		//use pressure
		void SetBrushUsePres(bool pres) { m_use_press = pres; }
		bool GetBrushUsePres() { return m_use_press; }
		void SetBrushPressPeak(double val) { m_press_peak = val; }
		double GetBrushPressPeak() { return m_press_peak; }
		void SetBrushPnMax(double val) { m_press_nmax = val; }
		double GetBrushPnMax() { return m_press_nmax; }
		void SetBrushPtMax(double val) { m_press_tmax = val; }
		double GetBrushPtMax() { return m_press_tmax; }
		void SetPressure(int np, int tp)
		{
			//compute pressure, normalized to [0, 1]
			if (m_press_nmax > 1.0)
				m_pressure = np / m_press_nmax;
			if (m_pressure > m_press_peak)
				m_press_peak = m_pressure;
			//wheel of air brush
			if (m_press_tmax > 1.0)
				m_air_press = tp / m_press_tmax;
		}
		double GetPressure() { return m_pressure; }
		double GetNormPress()
		{
			double np = m_use_press && m_pressure > 0.0 ?
				1.0 + (m_pressure - 0.5)*0.4 : 1.0;
			np += m_air_press - 0.5;
			np = std::max(np, 0.0);
			return np;
		}
		//set brush size
		void SetUseBrushSize2(bool val)
		{
			m_use_brush_radius2 = val;
			if (!val) m_brush_radius2 = m_brush_radius1;
		}
		bool GetUseBrushSize2() { return m_use_brush_radius2; }
		void SetBrushSize(double size1, double size2)
		{
			if (size1 > 0.0)
				m_brush_radius1 = size1;
			if (size2 > 0.0)
				m_brush_radius2 = size2;
			if (!m_use_brush_radius2)
				m_brush_radius2 = m_brush_radius1;
		}
		double GetBrushSize1() { return m_brush_radius1; }
		double GetBrushSize2() { return m_brush_radius2; }
		//set brush spacing
		void SetBrushSpacing(double spacing)
		{
			if (spacing > 0.0)
				m_brush_spacing = spacing;
		}
		double GetBrushSpacing() { return m_brush_spacing; }
		//set brush size relation
		void SetBrushSizeData(bool val) { m_brush_size_data = val; }
		bool GetBrushSizeData() { return m_brush_size_data; }
		//change display
		void ChangeBrushSize(int value, bool ctrl)
		{
			if (!value) return;

			if (m_mode == SelectMode::Solid || !m_use_brush_radius2)
			{
				double delta = value * m_brush_radius1 / 1000.0;
				m_brush_radius1 += delta;
				m_brush_radius1 = std::max(m_brush_radius1, 1.0);
				m_brush_radius2 = m_brush_radius1;
			}
			else
			{
				if (ctrl)
				{
					double delta = value * m_brush_radius1 / 1000.0;
					m_brush_radius1 += delta;
					m_brush_radius1 = std::max(m_brush_radius1, 1.0);
					m_brush_radius2 = std::max(m_brush_radius2, m_brush_radius1);
				}
				else
				{
					double delta = value * m_brush_radius2 / 2000.0;
					m_brush_radius2 += delta;
					m_brush_radius2 = std::max(1.0, m_brush_radius2);
					m_brush_radius1 = std::min(m_brush_radius2, m_brush_radius1);
				}
			}

			UpdateBrushRadiusSet();
		}
		//brush sets
		void GetBrushRadiusSet(std::vector<BrushRadiusSet>& sets)
		{
			sets.assign(m_brush_radius_sets.begin(), m_brush_radius_sets.end());
		}
		void SetBrushRadiusSet(const std::vector<BrushRadiusSet>& sets)
		{
			m_brush_radius_sets.assign(sets.begin(), sets.end());
			if (!m_brush_radius_sets.empty() &&
				m_brush_sets_index >= 0 &&
				m_brush_sets_index < m_brush_radius_sets.size())
			{
				BrushRadiusSet& radius_set = m_brush_radius_sets.at(m_brush_sets_index);
				m_brush_radius1 = radius_set.radius1;
				m_brush_radius2 = radius_set.radius2;
				m_use_brush_radius2 = radius_set.use_radius2;
				m_iter_num = radius_set.iter_num;
			}
		}
		void UpdateBrushRadiusSet()
		{
			SelectMode mode = m_mode;
			if (mode == SelectMode::SingleSelect ||
				mode == SelectMode::Segment ||
				mode == SelectMode::Mesh)
				mode = SelectMode::Append;
			for (auto& it : m_brush_radius_sets)
			{
				if (it.type == mode)
				{
					it.radius1 = m_brush_radius1;
					it.radius2 = m_brush_radius2;
					it.use_radius2 = m_use_brush_radius2;
					it.iter_num = m_iter_num;
				}
			}
		}
		void ChangeBrushSetsIndex();
		//set use 2d rendering results
		void SetPaintUse2d(bool use2d) { m_use2d = use2d; }
		bool GetPaintUse2d() { return m_use2d; }
		//estimate threshold
		void SetEstimateThreshold(bool value) { m_estimate_threshold = value; }
		bool GetEstimateThreshold() { return m_estimate_threshold; }
		//th udpate
		bool GetThUpdate();
		//if auto threshold is calculated
		bool GetAutoThreshold();

		//segment volumes in current view
		void Segment(bool push_mask, bool est_th = true, int mx = 0, int my = 0);
		void Select(bool push_mask, bool est_th, double radius);
		void Clear();//erase selection
		void Erase();//extract a new volume excluding the selection
		void Extract();//extract a new volume of the selection
		void CompExportRandomColor(int hmode,
			std::shared_ptr<VolumeData>& vd_r,
			std::shared_ptr<VolumeData>& vd_g,
			std::shared_ptr<VolumeData>& vd_b,
			bool select, bool hide);
		std::shared_ptr<VolumeData> GetResult(bool pop);

		void PushMask();
		void PopMask();
		void UndoMask();
		void RedoMask();
		//mask operations
		void CopyMask(bool copy_data);
		void SetCopyMaskVolume(const std::shared_ptr<VolumeData>& vd) { m_vd_copy = vd; }
		std::shared_ptr<VolumeData> GetCopyMaskVolume() { return m_vd_copy; }
		void PasteMask(int op);

		//mouse position
		void ResetMousePos()
		{
			m_mx = m_my = m_mx0 = m_my0 = -1;
		}
		bool GetMouseVec(int mx, int my, fluo::Vector &mvec);

		//speed test
		bool m_test_speed;
		double GetSpanSec() { return m_span_sec; }

	private:
		std::shared_ptr<VolumeData> m_vd;	//volume data for segmentation
		std::shared_ptr<VolumeData> m_vd_copy;//for copying mask source
		bool m_copy_data;//copy data or mask

		unsigned int m_2d_mask;	//2d mask from painting
		unsigned int m_2d_weight1;//2d weight map (after tone mapping)
		unsigned int m_2d_weight2;//2d weight map	(before tone mapping)
		double m_mvmat[16];	//modelview matrix
		double m_prjmat[16];//projection matrix
		int m_iter_num;		//iteration number for growing
		SelectMode m_mode;	//selection modes
		int m_init_mask;	//0; 1-init only; 2-diffuse only; 3-init & diffuse
		bool m_use2d;

		int m_iter;
		bool m_update_order;

		//brush properties
		double m_ini_thresh;
		double m_gm_falloff;
		double m_scl_falloff;
		double m_scl_translate;
		bool m_select_multi;	//0-only current; 1-select group;
		bool m_edge_detect;
		bool m_hidden_removal;
		bool m_ortho;
		//w2d
		double m_w2d;
		//paint brush use pressure
		bool m_use_press;
		bool m_on_press;
		double m_pressure;
		double m_press_peak;
		double m_press_nmax;
		double m_press_tmax;
		//air brush
		double m_air_press;
		//paint stroke radius
		double m_brush_radius1;
		double m_brush_radius2;
		bool m_use_brush_radius2;
		//radius settings for individual brush types
		std::vector<BrushRadiusSet> m_brush_radius_sets;
		int m_brush_sets_index;
		//paint stroke spacing
		double m_brush_spacing;
		//brush size relation
		bool m_brush_size_data;

		//exported volumes
		std::vector<std::shared_ptr<VolumeData>> m_result_vols;

		//a random variable
		int m_randv;

		bool m_estimate_threshold;

		glm::mat4 m_mv_mat;
		glm::mat4 m_prj_mat;

		int m_mx, m_my, m_mx0, m_my0;
		fluo::Vector m_mvec;

		std::chrono::high_resolution_clock::time_point m_t1, m_t2;
		double m_span_sec;

	private:
		double HueCalculation(int mode, unsigned int label);
		void segment(bool push_mask, bool est_th, int mx, int my);
	};
}
#endif//_VOLUMESELECTOR_H_
