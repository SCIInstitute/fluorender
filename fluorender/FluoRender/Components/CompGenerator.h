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
#ifndef FL_CompGenerator_h
#define FL_CompGenerator_h

#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/signals2.hpp>
#include "DataManager.h"
#include <FLIVR/KernelProgram.h>
#include <FLIVR/VolKernel.h>

using namespace std;

class VolumeData;
namespace flrd
{
	typedef std::vector<std::string> CompCmdParams;
	//comand types: generate, clean, fixate
	typedef std::vector<CompCmdParams> CompCommand;
	class ComponentGenerator
	{
	public:
		ComponentGenerator(VolumeData* vd);
		~ComponentGenerator();

		void SetUseMask(bool use_mask)
		{ m_use_mask = use_mask; }
		bool GetUseMask()
		{ return m_use_mask; }

		//segmentation functions
		void ShuffleID();
		void SetIDBit(int);
		void Grow(bool, int, float, float, float);
		void DensityField(int dsize, int wsize,
			bool diffuse, int iter, float tran, float falloff,
			float density, float sscale);
		void DistGrow(bool, int,
			float, float, int, int, float, float, float);
		void DistDensityField(
			bool diffuse, int iter, float tran, float falloff,
			int dsize1, int max_dist, float dist_thresh, float dist_strength,
			int dsize2, int wsize, float density, float sscale);
		void Cleanup(int, unsigned int);
		void ClearBorders();
		void FillBorders(float);

		//unused
		void OrderID_2D();
		void OrderID_3D();
		void ShuffleID_2D();
		void Grow3DSized(bool, int, float, float,
			int, float, int);
		void MatchSlices(bool backwards,
			unsigned int,
			float, float, float);
		void DistField(int iter, float th, int dsize, float sscale);

		//update progress
		boost::signals2::signal<void ()> m_sig_progress;

	private:
		VolumeData *m_vd;
		bool m_use_mask;//use mask instead of data

	private:
		bool CheckBricks();
		unsigned int reverse_bit(unsigned int val, unsigned int len);
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
