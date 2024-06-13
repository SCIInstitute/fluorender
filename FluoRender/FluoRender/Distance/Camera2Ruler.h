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
#ifndef _CAMERA2RULER_H_
#define _CAMERA2RULER_H_

#include <Ruler.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace flrd
{
	class Camera2Ruler
	{
	public:
		Camera2Ruler();
		~Camera2Ruler();

		void SetList(int i, RulerList* list);
		void SetList(int i, const std::string& config);//read list from a project
		void SetRange(int i, size_t start, size_t end)
		{
			if (i == 1)
			{
				m_start_list1 = start;
				m_end_list1 = end;
			}
			else if (i == 2)
			{
				m_start_list2 = start;
				m_end_list2 = end;
			}
		}
		void SetImageSize(int nx, int ny)
		{
			m_nx = nx;
			m_ny = ny;
		}
		void SetNames(const std::vector<std::string>& names)
		{
			m_names = names;
		}
		void SetSlope(double dval)
		{
			m_slope = dval;
		}
		void SetPersp(bool val)
		{
			m_persp = val;
		}
		void SetAffine(bool val)
		{
			m_affine = val;
		}
		void SetMetric(bool val)
		{
			m_metric = val;
		}

		void Run();//compute 3D

		void Correct();

		RulerList* GetResult()
		{
			return m_list_out;
		}

	private:
		int m_nx;
		int m_ny;
		size_t m_start_list1;
		size_t m_start_list2;
		size_t m_end_list1;
		size_t m_end_list2;
		bool m_del_list1;
		bool m_del_list2;
		RulerList* m_list1;
		RulerList* m_list2;
		RulerList* m_list_out;
		double m_slope;
		std::vector<std::string> m_names;
		bool m_persp;
		bool m_affine;
		bool m_metric;
		cv::Mat m_h;//homogeneours matrix from affine correction

	private:
		bool get_affine(const cv::Mat& p1, const cv::Mat& p2);
		cv::Vec3d calib_affine(const cv::Vec3d& pp);
		bool calib_persp();
		bool calib_metric();
		cv::Point2f normalize(fluo::Point& p)
		{
			cv::Point2f cvp =
			{
				float(p.x() / m_nx - 0.5),
				float(p.y() / m_nx - 0.5 * double(m_ny) / double(m_nx))
			};
			return cvp;
		}
		cv::Point3f normalize_homo(fluo::Point& p)
		{
			cv::Point3f cvp =
			{
				float(p.x() / m_nx - 0.5),
				float(p.y() / m_nx - 0.5 * double(m_ny) / double(m_nx)),
				1
			};
			return cvp;
		}
		cv::Vec3d triangulate(
			const cv::Point2f& pp1, const cv::Point2f& pp2,
			const cv::Mat& p1, const cv::Mat& p2);
		cv::Vec4d get_plane(
			const cv::Vec4d& pp1,
			const cv::Vec4d& pp2,
			const cv::Vec4d& pp3);
	};
}

#endif//_CAMERA2RULER_H_