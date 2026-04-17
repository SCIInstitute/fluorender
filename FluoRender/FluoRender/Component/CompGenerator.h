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
#ifndef FL_CompGenerator_h
#define FL_CompGenerator_h

#include <Progress.h>
#include <vector>
#include <functional>
#include <chrono>
#include <memory>

class VolumeData;
namespace flrd
{
	typedef std::vector<std::string> CompCmdParams;
	//comand types: generate, clean, fixate
	typedef std::vector<CompCmdParams> CompCommand;
	typedef std::function<void(const std::string&)> CompGenFunc;
	class ComponentGenerator : public Progress
	{
	public:
		ComponentGenerator();
		~ComponentGenerator();

		bool GetAutoCompGen() const;

		void SetVolumeData(const std::shared_ptr<VolumeData>& vd);
		std::shared_ptr<VolumeData> GetVolumeData() const;

		void SetUseSel(bool val) { m_use_sel = val; }
		bool GetUseSel() const { return m_use_sel; }
		void SetUseMl(bool val) { m_use_ml = val; }
		bool GetUseMl() const { return m_use_ml; }
		void SetIter(int val) { m_iter = val; }
		int GetIter() const { return m_iter; }
		void SetThresh(double val) { m_thresh = val; }
		double GetThresh() const { return m_thresh; }
		void SetTFactor(double val) { m_tfactor = val; }
		double GetTFactor() const { return m_tfactor; }
		//distance
		void SetUseDistField(bool val) { m_use_dist_field = val; }
		bool GetUseDistField() const { return m_use_dist_field; }
		void SetDistStrength(double val) { m_dist_strength = val; }
		double GetDistStrength() const { return m_dist_strength; }
		void SetDistFilterSize(int val) { m_dist_filter_size = val; }
		int GetDistFilterSize() const { return m_dist_filter_size; }
		void SetMaxDist(int val) { m_max_dist = val; }
		int GetMaxDist() const { return m_max_dist; }
		void SetDistThresh(double val) { m_dist_thresh = val; }
		double GetDistThresh() const { return m_dist_thresh; }
		//diffusion
		void SetDiffusion(bool val) { m_diff = val; }
		bool GetDiffusion() const { return m_diff; }
		void SetFalloff(double val) { m_falloff = val; }
		double GetFalloff() const { return m_falloff; }
		//size
		void SetSize(bool val) { m_size = val; }
		bool GetSize() const { return m_size; }
		void SetSizeLimit(int val) { m_size_lm = val; }
		int GetSizeLimit() const { return m_size_lm; }
		//density
		void SetDensity(bool val) { m_density = val; }
		bool GetDensity() const { return m_density; }
		void SetDensityThresh(double val) { m_density_thresh = val; }
		double GetDensityThresh() const { return m_density_thresh; }
		void SetVarThresh(double val) { m_varth = val; }
		double GetVarThresh() const { return m_varth; }
		void SetDensityWinSize(int val) { m_density_window_size = val; }
		int GetDensityWinSize() const { return m_density_window_size; }
		void SetDensityStatSize(int val) { m_density_stats_size = val; }
		int GetDensityStatSize() const { return m_density_stats_size; }
		//fixate
		void SetFixate(bool val) { m_fixate = val; }
		bool GetFixate() const { return m_fixate; }
		void SetFixSize(int val) { m_fix_size = val; }
		int GetFixSize() const { return m_fix_size; }
		void SetGrowFixed(int val) { m_grow_fixed = val; }
		int GetGrowFixed() const { return m_grow_fixed; }
		//clean
		void SetClean(bool val) { m_clean = val; }
		bool GetClean() const { return m_clean; }
		void SetCleanIter(int val) { m_clean_iter = val; }
		int GetCleanIter() const { return m_clean_iter; }
		void SetCleanSize(int val) { m_clean_size_vl = val; }
		int GetCleanSize() const { return m_clean_size_vl; }
		//fill border
		void SetFillBorder(double val) { m_fill_border = val; }
		double GetFillBorder() const { return m_fill_border; }

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
		void LoadCmd(const std::wstring& filename);
		void SaveCmd(const std::wstring& filename);
		void AddCmd(const std::string& type);
		void ResetCmd();
		void PlayCmd(double tfactor);
		void SetRecordCmd(bool val);
		bool GetRecordCmd() const;
		size_t GetCmdNum() const;

		//if auto threshold is calculated
		bool GetAutoThreshold() const;

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

		//output
		std::wstring GetTitles() const { return m_titles; }
		std::wstring GetValues() const { return m_values; }

		//update progress
		CompGenFunc prework;
		CompGenFunc postwork;

	private:
		std::weak_ptr<VolumeData> m_vd;

		bool m_use_sel = true;//use mask instead of data
		bool m_use_ml = false;//use machine learning
		int m_iter = 30;//iteration
		mutable double m_thresh = 0.5;
		double m_tfactor = 1.0;
		//distance field
		bool m_use_dist_field = false;
		double m_dist_strength = 0.5;
		int m_dist_filter_size = 3;
		int m_max_dist = 30;
		double m_dist_thresh = 0.25;
		//diffusion
		bool m_diff = false;
		double m_falloff = 0.01;
		bool m_size = false;
		int m_size_lm = 100;
		//density
		bool m_density = false;
		double m_density_thresh = 1.0;
		double m_varth;//variance threshold
		int m_density_window_size = 5;
		int m_density_stats_size = 15;
		//fixate
		bool m_fixate = false;
		int m_fix_size = 50;
		int m_grow_fixed = 1;
		//clean
		bool m_clean = false;
		int m_clean_iter = 5;
		int m_clean_size_vl = 5;
		//fill borders
		double m_fill_border = 0.1;

		//record
		bool m_record_cmd = false;
		CompCommand m_command;

		//speed test
		bool m_test_speed = false;
		std::vector<std::chrono::high_resolution_clock::time_point> m_tps;
		std::wstring m_titles, m_values;

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
