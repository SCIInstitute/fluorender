/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2019 Scientific Computing and Imaging Institute,
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
#ifndef FL_Pca_h
#define FL_Pca_h

#include <FLIVR/Point.h>
#include <FLIVR/Vector.h>
#include <vector>

namespace FL
{
	class Pca
	{
	public:
		Pca()
		{}
		~Pca()
		{}

		void AddPoint(FLIVR::Point &point)
		{
			m_points.push_back(point);
		}
		void SetPoints(std::vector<FLIVR::Point> &points)
		{
			m_points.assign(points.begin(), points.end());
		}
		void ClearPoints()
		{
			m_points.clear();
		}
		void SetCovMat(std::vector<double> &cov);

		void Compute(bool upd_cov=true);

		FLIVR::Vector GetAxis(int index)
		{
			if (index >= 0 && index <= 2)
				return m_axis[index];
			else
				return FLIVR::Vector();
		}

	private:
		std::vector<FLIVR::Point> m_points;
		FLIVR::Vector m_axis[3];
		double m_values[3];
		double m_cov[3][3];//covariance matrix

	private:

	};
}
#endif//FL_Pca_h