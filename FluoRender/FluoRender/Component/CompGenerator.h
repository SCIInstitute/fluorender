/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2024 Scientific Computing and Imaging Institute,
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
#ifndef FL_CompGenerator_h
#define FL_CompGenerator_h

#include <DataManager.h>
#include <KernelProgram.h>
#include <VolKernel.h>
#include <vector>
#include <functional>
#include <chrono>

using namespace std;

class VolumeData;
namespace flrd
{
	typedef std::vector<std::string> CompCmdParams;
	//comand types: generate, clean, fixate
	typedef std::vector<CompCmdParams> CompCommand;
	typedef std::function<void(const std::string&)> CompGenFunc;
	class ComponentGenerator
	{
	public:
		ComponentGenerator();
		~ComponentGenerator();

		void SetVolumeData(VolumeData* vd)
		{
			m_vd = vd;
		}

		void SetUseSel(bool val) { m_use_sel = val; }
		bool GetUseSel() { return m_use_sel; }
		void SetUseMl(bool val) { m_use_ml = val; }
		bool GetUseMl() { return m_use_ml; }
		void SetIter(int val) { m_iter = val; }
		int GetIter() { return m_iter; }
		void SetThresh(double val) { m_thresh = val; }
		double GetThresh() { return m_thresh; }
		void SetTFactor(double val) { m_tfactor = val; }
		double GetTFactor() { return m_tfactor; }
		//distance
		void SetUseDistField(bool val) { m_use_dist_field = val; }
		bool GetUseDistField() { return m_use_dist_field; }
		void SetDistStrength(double val) { m_dist_strength = val; }
		double GetDistStrength() { return m_dist_strength; }
		void SetDistFilterSize(int val) { m_dist_filter_size = val; }
		int GetDistFilterSize() { return m_dist_filter_size; }
		void SetMaxDist(int val) { m_max_dist = val; }
		int GetMaxDist() { return m_max_dist; }
		void SetDistThresh(double val) { m_dist_thresh = val; }
		double GetDistThresh() { return m_dist_thresh; }
		//diffusion
		void SetDiffusion(bool val) { m_diff = val; }
		bool GetDiffusion() { return m_diff; }
		void SetFalloff(double val) { m_falloff = val; }
		double GetFalloff() { return m_falloff; }
		//size
		void SetSize(bool val) { m_size = val; }
		bool GetSize() { return m_size; }
		void SetSizeLimit(int val) { m_size_lm = val; }
		int GetSizeLimit() { return m_size_lm; }
		//density
		void SetDensity(bool val) { m_density = val; }
		bool GetDensity() { return m_density; }
		void SetDensityThresh(double val) { m_density_thresh = val; }
		double GetDensityThresh() { return m_density_thresh; }
		void SetVarThresh(double val) { m_varth = val; }
		double GetVarThresh() { return m_varth; }
		void SetDensityWinSize(int val) { m_density_window_size = val; }
		int GetDensityWinSize() { return m_density_window_size; }
		void SetDensityStatSize(int val) { m_density_stats_size = val; }
		int GetDensityStatSize() { return m_density_stats_size; }
		//fixate
		void SetFixate(bool val) { m_fixate = val; }
		bool GetFixate() { return m_fixate; }
		void SetFixSize(int val) { m_fix_size = val; }
		int GetFixSize() { return m_fix_size; }
		void SetGrowFixed(int val) { m_grow_fixed = val; }
		int GetGrowFixed() { return m_grow_fixed; }
		//clean
		void SetClean(bool val) { m_clean = val; }
		bool GetClean() { return m_clean; }
		void SetCleanIter(int val) { m_clean_iter = val; }
		int GetCleanIter() { return m_clean_iter; }
		void SetCleanSize(int val) { m_clean_size_vl = val; }
		int GetCleanSize() { return m_clean_size_vl; }
		//fill border
		void SetFillBorder(double val) { m_fill_border = val; }
		double GetFillBorder() { return m_fill_border; }

		//high-level functions
		void Compute();
		void GenerateComp(bool command = true);
		void Fixate(bool command = true);
		void Clean(bool command = true);
		//learning functions
		void ApplyRecord();

		//segmentation functions
		void ShuffleID();
		void SetIDBit(int);

		void Grow();
		void DensityField();
		void DistGrow();
		void DistDensityField();
		void CleanNoise();
		void ClearBorders();
		void FillBorders();

		//learning functions
		void AddEntry();
		void AddCleanEntry();
		void GenerateDB();

		//command
		void LoadCmd(const wxString& filename);
		void SaveCmd(const wxString& filename);
		void AddCmd(const std::string& type);
		void ResetCmd();
		void PlayCmd(double tfactor);
		void SetRecordCmd(bool val);
		bool GetRecordCmd();
		int GetCmdNum();

		//unused
		//void OrderID_2D();
		//void OrderID_3D();
		//void ShuffleID_2D();
		//void Grow3DSized(bool, int, float, float,
		//	int, float, int);
		//void MatchSlices(bool backwards,
		//	unsigned int,
		//	float, float, float);
		//void DistField(int iter, float th, int dsize, float sscale);

		//update progress
		CompGenFunc prework;
		CompGenFunc postwork;

	private:
		VolumeData *m_vd;

		bool m_use_sel;//use mask instead of data
		bool m_use_ml;//use machine learning
		int m_iter;//iteration
		double m_thresh;
		double m_tfactor;
		//distance field
		bool m_use_dist_field;
		double m_dist_strength;
		int m_dist_filter_size;
		int m_max_dist;
		double m_dist_thresh;
		//diffusion
		bool m_diff;
		double m_falloff;
		bool m_size;
		int m_size_lm;
		//density
		bool m_density;
		double m_density_thresh;
		double m_varth;//variance threshold
		int m_density_window_size;
		int m_density_stats_size;
		//fixate
		bool m_fixate;
		int m_fix_size;
		int m_grow_fixed;
		//clean
		bool m_clean;
		int m_clean_iter;
		int m_clean_size_vl;
		//fill borders
		double m_fill_border;

		//record
		bool m_record_cmd;
		CompCommand m_command;

		//speed test
		bool m_test_speed;
		std::vector<std::chrono::high_resolution_clock::time_point> m_tps;
		wxString m_titles, m_values;

	private:
		bool CheckBricks();
		unsigned int reverse_bit(unsigned int val, unsigned int len);
		//speed test
		void StartTimer(const std::string& str);
		void StopTimer(const std::string& str);
	};

	inline unsigned int ComponentGenerator::reverse_bit(unsigned int val, unsigned int len)
	{
		unsigned int res = val;
		int s = len - 1;

		for (val >>= 1; val; val >>= 1)
		{
			res <<= 1;
			res |= val & 1;
			s--;
		}
		res <<= s;
		res <<= 32 - len;
		res >>= 32 - len;
		return res;
	}

}
#endif//FL_CompGenerator_h
