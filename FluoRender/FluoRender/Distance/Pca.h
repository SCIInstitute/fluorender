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
#ifndef FL_Pca_h
#define FL_Pca_h

#include <Point.h>
#include <Vector.h>
#include <BPoint.h>
#include <cstring>
#include <vector>

namespace flrd
{
	class Pca
	{
	public:
		Pca(int mode = 0):
			m_mode(mode)
		{
			ClearPoints();
		}
		~Pca()
		{}

		void Add(Pca &pca)
		{
			if (m_mode == 0)
			{
				m_mean += pca.m_mean;
				m_cov[0][0] += pca.m_cov[0][0];
				m_cov[0][1] += pca.m_cov[0][1];
				m_cov[0][2] += pca.m_cov[0][2];
				m_cov[1][1] += pca.m_cov[1][1];
				m_cov[1][2] += pca.m_cov[1][2];
				m_cov[2][2] += pca.m_cov[2][2];
				m_num += pca.m_num;
			}
			else if (m_mode == 2)
				m_points.insert(m_points.end(), pca.m_points.begin(), pca.m_points.end());
			m_bpoint.extend(pca.m_bpoint);
		}
		void AddPoint(const fluo::Point &point)
		{
			if (m_mode == 0)
			{
				m_mean = fluo::Point(m_mean + point);
				m_cov[0][0] += point(0) * point(0);
				m_cov[0][1] += point(0) * point(1);
				m_cov[0][2] += point(0) * point(2);
				m_cov[1][1] += point(1) * point(1);
				m_cov[1][2] += point(1) * point(2);
				m_cov[2][2] += point(2) * point(2);
				m_num++;
			}
			else if (m_mode == 2)
				m_points.push_back(point);
			m_bpoint.extend(point);
		}
		void AddPointScale(const fluo::Point &point, const fluo::Vector& scale)
		{
			fluo::Point sp(point);
			sp.scale(scale);
			AddPoint(sp);
		}
		void SetPoints(std::vector<fluo::Point> &points)
		{
			m_points.assign(points.begin(), points.end());
			m_mode = 2;
		}
		std::vector<fluo::Point> &GetPoints()
		{
			return m_points;
		}
		void SetCovMat(std::vector<double> &cov)
		{
			size_t size = cov.size();
			if (size < 6)
				return;
			if (size > 9)
				size = 9;
			std::memcpy(m_cov, &cov[0], size * sizeof(double));
			if (size < 9)
			{
				m_cov[2][2] = m_cov[1][2];
				m_cov[2][1] = m_cov[1][1];
				m_cov[2][0] = m_cov[0][2];
				m_cov[1][2] = m_cov[1][1];
				m_cov[1][1] = m_cov[1][0];
				m_cov[1][0] = m_cov[0][1];
			}
			m_mode = 1;
		}

		void ClearPoints()
		{
			m_num = 0;
			m_mean = fluo::Point();
			std::memset(m_cov, 0, sizeof(double) * 9);
			m_points.clear();
		}
		void Compute();

		fluo::Vector GetAxis(int index)
		{
			if (index >= 0 && index <= 2)
				return m_axis[index];
			else
				return fluo::Vector();
		}
		fluo::Vector GetLengths()
		{
			return fluo::Vector(m_values[0], m_values[1], m_values[2]);
		}

	private:
		int m_mode;//0-incremental cov; 1-external cov; 2-store points
		fluo::BPoint m_bpoint;
		std::vector<fluo::Point> m_points;
		fluo::Vector m_axis[3];
		double m_values[3];
		int m_num;
		fluo::Point m_mean;
		double m_cov[3][3];//covariance matrix

	private:

	};
}
#endif//FL_Pca_h