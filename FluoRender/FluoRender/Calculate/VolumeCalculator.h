﻿/*
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
#ifndef _VOLUMECALCULATOR_H_
#define _VOLUMECALCULATOR_H_

#include <Progress.h>
#include <vector>
#include <string>
#include <memory>

class VolumeData;
namespace flrd
{
	class VolumeSelector;
	class VolumeCalculator : public Progress
	{
	public:
		VolumeCalculator();
		~VolumeCalculator();

		void SetVolumeA(const std::shared_ptr<VolumeData>& vd);
		void SetVolumeB(const std::shared_ptr<VolumeData>& vd);

		void SetThreshold(double thresh)
		{ m_threshold = thresh; }

		std::shared_ptr<VolumeData> GetVolumeA();
		std::shared_ptr<VolumeData> GetVolumeB();
		std::shared_ptr<VolumeData> GetResult(bool pop);

		//1-sub;2-add;3-div;4-and;5-new;6-new inv;7-clear
		void CalculateGroup(int type, const std::wstring &prev_group = L"", bool add = true);
		void CalculateSingle(int type, const std::wstring &prev_group, bool add);
		void Calculate(int type);

	private:
		std::vector<std::shared_ptr<VolumeData>> m_vd_r;//result volume data (stack)

		std::weak_ptr<VolumeData> m_vd_a;	//volume data A
		std::weak_ptr<VolumeData> m_vd_b;	//volume data B

		int m_type;	//calculation type
					//1:substraction;
					//2:addition;
					//3:division;
					//4:intersection
					//5:apply mask (single volume multiplication)
					//6:apply mask inverted (multiplication with mask's complement in volume)
					//7:apply mask inverted, then replace volume a
					//8:intersection with masks if available
					//9:fill holes

		double m_threshold;

	private:
		void CreateVolumeResult1();//create the resulting volume from one input
		void CreateVolumeResult2();//create the resulting volume from two inputs

		//fill holes
		void FillHoles(double thresh);
	};
}
#endif//_VOLUMECALCULATOR_H_