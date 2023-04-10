/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2023 Scientific Computing and Imaging Institute,
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

#include "Camera2Ruler.h"
#include <Distance/RulerHandler.h>
#include <opencv2/opencv.hpp>
#include <wx/wfstream.h>
#include <wx/fileconf.h>

using namespace flrd;

Camera2Ruler::Camera2Ruler() :
	m_scale(100),
	m_nx(0),
	m_ny(0),
	m_start_list1(0),
	m_start_list2(0),
	m_end_list1(0),
	m_end_list2(0),
	m_del_list1(false),
	m_del_list2(false),
	m_list1(0),
	m_list2(0),
	m_list_out(0)
{

}

Camera2Ruler::~Camera2Ruler()
{
	if (m_del_list1)
	{
		m_list1->DeleteRulers();
		delete m_list1;
	}
	if (m_del_list2)
	{
		m_list2->DeleteRulers();
		delete m_list2;
	}
}

void Camera2Ruler::SetList(int i, RulerList* list)
{
	if (i == 1)
		m_list1 = list;
	else if (i == 2)
		m_list2 = list;
}

void Camera2Ruler::SetList(int i, const std::string& config)
{
	RulerList* list = 0;
	wxFileInputStream is(config);
	if (!is.IsOk())
		return;
	wxFileConfig fconfig(is);

	//movie panel
	int startf, endf;
	if (fconfig.Exists("/movie_panel"))
	{
		fconfig.SetPath("/movie_panel");
		fconfig.Read("start_frame", &startf, 0);
		fconfig.Read("end_frame", &endf, 0);
	}
	//views
	if (fconfig.Exists("/views"))
	{
		fconfig.SetPath("/views");
		int num = fconfig.Read("num", 0l);
		if (num < 1)
			return;

		if (fconfig.Exists("/views/0/rulers"))
		{
			fconfig.SetPath("/views/0/rulers");
			RulerHandler handler;
			list = new RulerList;
			handler.SetRulerList(list);
			handler.Read(fconfig, 0);
		}
	}

	if (list)
	{
		if (i == 1)
		{
			if (m_del_list1)
				delete m_list1;
			m_list1 = list;
			m_del_list1 = true;
			m_start_list1 = std::max(int(m_start_list1), startf);
			m_end_list1 = m_end_list1 ? std::min(int(m_end_list1), endf) : endf;
		}
		else if (i == 2)
		{
			if (m_del_list2)
				delete m_list2;
			m_list2 = list;
			m_del_list2 = true;
			m_start_list2 = std::max(int(m_start_list2), startf);
			m_end_list2 = m_end_list2 ? std::min(int(m_end_list2), endf) : endf;
		}
	}
}

void Camera2Ruler::Run()
{
	if (!m_list1 || !m_list2)
		return;

	std::vector<cv::Point2f> pp1, pp2;

	for (auto ruler : *m_list1)
	{
		for (int i = 0; i < ruler->GetNumPoint(); ++i)
		{
			for (size_t t = m_start_list1+1; t < m_end_list1; ++t)
			{
				ruler->SetWorkTime(t);
				fluo::Point p = ruler->GetPoint(i);
				cv::Point2f cvp = { float(p.x()), float(p.y()) };
				pp1.push_back(cvp);
			}
		}
	}
	for (auto ruler : *m_list2)
	{
		for (int i = 0; i < ruler->GetNumPoint(); ++i)
		{
			for (size_t t = m_start_list2+1; t < m_end_list2; ++t)
			{
				ruler->SetWorkTime(t);
				fluo::Point p = ruler->GetPoint(i);
				cv::Point2f cvp = { float(p.x()), float(p.y()) };
				pp2.push_back(cvp);
			}
		}
	}

	//trim
	if (pp1.size() > pp2.size())
		pp1.resize(pp2.size());
	else if (pp2.size() > pp1.size())
		pp2.resize(pp1.size());

	cv::Mat essential = cv::findEssentialMat(pp1, pp2);
	// recover relative camera pose from essential matrix
	cv::Mat rotation, translation;
	cv::recoverPose(essential, pp1, pp2, rotation, translation);
	// compose projection matrix from R,T
	cv::Mat p2(3, 4, CV_64F); // the 3x4 projection matrix
	rotation.copyTo(p2(cv::Rect(0, 0, 3, 3)));
	translation.copyTo(p2.colRange(3, 4));
	// compose generic projection matrix
	cv::Mat p1(3, 4, CV_64F, 0.); // the 3x4 projection matrix
	cv::Mat diag(cv::Mat::eye(3, 3, CV_64F));
	diag.copyTo(p1(cv::Rect(0, 0, 3, 3)));
	// Triangulation
	std::vector<cv::Vec3d> points3D;
	//cal.triangulate(projection1, projection2, points1u, points2u, points3D);
	for (size_t i = 0; i < pp1.size(); ++i)
	{
		cv::Vec2d u1(pp1[i].x, pp1[i].y);
		cv::Vec2d u2(pp2[i].x, pp2[i].y);
		cv::Matx43d A(
			u1(0) * p1.at<double>(2, 0) - p1.at<double>(0, 0),
			u1(0) * p1.at<double>(2, 1) - p1.at<double>(0, 1),
			u1(0) * p1.at<double>(2, 2) - p1.at<double>(0, 2),
			u1(1) * p1.at<double>(2, 0) - p1.at<double>(1, 0),
			u1(1) * p1.at<double>(2, 1) - p1.at<double>(1, 1),
			u1(1) * p1.at<double>(2, 2) - p1.at<double>(1, 2),
			u2(0) * p2.at<double>(2, 0) - p2.at<double>(0, 0),
			u2(0) * p2.at<double>(2, 1) - p2.at<double>(0, 1),
			u2(0) * p2.at<double>(2, 2) - p2.at<double>(0, 2),
			u2(1) * p2.at<double>(2, 0) - p2.at<double>(1, 0),
			u2(1) * p2.at<double>(2, 1) - p2.at<double>(1, 1),
			u2(1) * p2.at<double>(2, 2) - p2.at<double>(1, 2));
		cv::Matx41d B(
			p1.at<double>(0, 3) - u1(0) * p1.at<double>(2, 3),
			p1.at<double>(1, 3) - u1(1) * p1.at<double>(2, 3),
			p2.at<double>(0, 3) - u2(0) * p2.at<double>(2, 3),
			p2.at<double>(1, 3) - u2(1) * p2.at<double>(2, 3));
		// X contains the 3D coordinate of the reconstructed point
		cv::Vec3d X;
		// solve AX=B
		cv::solve(A, B, X, cv::DECOMP_SVD);
		points3D.push_back(X * m_scale);
	}

	if (m_list_out)
	{
		m_list_out->DeleteRulers();
		delete m_list_out;
	}
	m_list_out = new RulerList;
	//add 3d points
	size_t c = 0;
	for (auto ruler : *m_list1)
	{
		Ruler* r0 = new Ruler;
		int rn = ruler->GetNumPoint();
		if (rn > 1)
			r0->SetRulerType(1);
		else
			r0->SetRulerType(2);
		for (int i = 0; i < rn; ++i)
		{
			for (size_t t = m_start_list1 + 1; t < m_end_list1; ++t)
			{
				fluo::Point p(
					(points3D[c])(0),
					(points3D[c])(1),
					(points3D[c])(2));
				if (t == m_start_list1 + 1)
				{
					r0->SetWorkTime(t);
					r0->AddPoint(p);
				}
				else
				{
					pRulerPoint pp = r0->GetPRulerPoint(i);
					pp->SetPoint(p, t);
				}
				c++;
			}
		}
		m_list_out->push_back(r0);
	}
}

