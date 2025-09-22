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

#include <opencv2/opencv.hpp>
#include <Camera2Ruler.h>
#include <Global.h>
#include <CurrentObjects.h>
#include <Ruler.h>
#include <RulerHandler.h>
#include <Ray.h>
#include <Plane.h>
#include <Point.h>

using namespace flrd;

Camera2Ruler::Camera2Ruler() :
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
	m_list_out(0),
	m_slope(0),
	m_persp(true),
	m_affine(false),
	m_metric(true)
{
	m_h = std::make_unique<cv::Mat>(cv::Mat::eye(4, 4, CV_64F));
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

void Camera2Ruler::SetList(int i, int startf, int endf)
{
	RulerList* list = glbin_current.GetRulerList();
	if (!list)
		return;

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

void Camera2Ruler::Run()
{
	if (!m_list1 || !m_list2)
		return;

	std::vector<cv::Point2f> pp1, pp2;

	//connvert ruler points
	for (auto ruler : *m_list1)
	{
		if (!ruler)
			continue;
		if (std::find(m_names.begin(), m_names.end(),
			ruler->GetName()) == m_names.end())
			continue;//ignore non-calib points
		for (int i = 0; i < ruler->GetNumPoint(); ++i)
		{
			ruler->SetWorkTime(0);
			fluo::Point p = ruler->GetPoint(i);
			pp1.push_back(normalize(p));
		}
	}
	for (auto ruler : *m_list2)
	{
		if (!ruler)
			continue;
		if (std::find(m_names.begin(), m_names.end(),
			ruler->GetName()) == m_names.end())
			continue;//ignore non-calib points
		for (int i = 0; i < ruler->GetNumPoint(); ++i)
		{
			ruler->SetWorkTime(0);
			fluo::Point p = ruler->GetPoint(i);
			pp2.push_back(normalize(p));
		}
	}

	//trim
	if (pp1.size() > pp2.size())
		pp1.resize(pp2.size());
	else if (pp2.size() > pp1.size())
		pp2.resize(pp1.size());

	cv::Mat f = cv::findFundamentalMat(pp1, pp2, cv::FM_LMEDS);
	// recover relative camera pose from essential matrix
	cv::Mat rotation, translation;
	cv::recoverPose(f, pp1, pp2, rotation, translation);
	// compose projection matrix from R,T
	cv::Mat p2(3, 4, CV_64F); // the 3x4 projection matrix
	rotation.copyTo(p2(cv::Rect(0, 0, 3, 3)));
	translation.copyTo(p2.colRange(3, 4));
	// compose generic projection matrix
	cv::Mat p1(3, 4, CV_64F, 0.); // the 3x4 projection matrix
	cv::Mat diag(cv::Mat::eye(3, 3, CV_64F));
	diag.copyTo(p1(cv::Rect(0, 0, 3, 3)));

	if (m_affine)
	{
		//calib
		get_affine(p1, p2);
	}

	//rebuild points
	pp1.clear();
	pp2.clear();
	for (auto ruler : *m_list1)
	{
		if (!ruler)
			continue;
		bool use_t = true;
		if (std::find(m_names.begin(), m_names.end(),
			ruler->GetName()) != m_names.end())
			use_t = false;
		for (int i = 0; i < ruler->GetNumPoint(); ++i)
		{
			if (use_t)
			{
				for (size_t t = m_start_list1 + 1; t < m_end_list1; ++t)
				{
					ruler->SetWorkTime(t);
					fluo::Point p = ruler->GetPoint(i);
					pp1.push_back(normalize(p));
				}
			}
			else
			{
				ruler->SetWorkTime(0);
				fluo::Point p = ruler->GetPoint(i);
				pp1.push_back(normalize(p));
			}
		}
	}
	for (auto ruler : *m_list2)
	{
		if (!ruler)
			continue;
		bool use_t = true;
		if (std::find(m_names.begin(), m_names.end(),
			ruler->GetName()) != m_names.end())
			use_t = false;
		for (int i = 0; i < ruler->GetNumPoint(); ++i)
		{
			if (use_t)
			{
				for (size_t t = m_start_list2 + 1; t < m_end_list2; ++t)
				{
					ruler->SetWorkTime(t);
					fluo::Point p = ruler->GetPoint(i);
					pp2.push_back(normalize(p));
				}
			}
			else
			{
				ruler->SetWorkTime(0);
				fluo::Point p = ruler->GetPoint(i);
				pp2.push_back(normalize(p));
			}
		}
	}
	// Triangulation
	std::vector<cv::Vec3d> points3D;
	for (size_t i = 0; i < pp1.size(); ++i)
	{
		cv::Vec3d X = triangulate(pp1[i], pp2[i], p1, p2);
		if (m_affine)
			X = calib_affine(X);
		points3D.push_back(X);
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
		if (!ruler)
			continue;
		bool use_t = true;
		if (std::find(m_names.begin(), m_names.end(),
			ruler->GetName()) != m_names.end())
			use_t = false;
		Ruler* r0 = new Ruler;
		r0->SetName(ruler->GetName());
		r0->SetColor(ruler->GetColor());
		int rn = ruler->GetNumPoint();
		if (rn > 1)
			r0->SetRulerMode(RulerMode::Polyline);
		else
			r0->SetRulerMode(RulerMode::Locator);
		for (int i = 0; i < rn; ++i)
		{
			if (use_t)
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
			else
			{
				fluo::Point p(
					(points3D[c])(0),
					(points3D[c])(1),
					(points3D[c])(2));
				r0->SetWorkTime(0);
				r0->AddPoint(p);
				c++;
			}
		}
		m_list_out->push_back(r0);
	}
}

void Camera2Ruler::Correct()
{
	if (!m_list_out)
		return;

	size_t n = m_names.size();
	if (n > 5)
	{
		if (m_persp)
			calib_persp();
		if (m_metric)
			calib_metric();
	}
}

bool Camera2Ruler::get_affine(const cv::Mat& p1, const cv::Mat& p2)
{
	//name order x1, x2, y1, y2, z1, z2
	//each pointing to the axis direction from p0 to p1
	if (m_names.size() < 6)
		return false;
	if (!m_list1 || !m_list2)
		return false;
	if (m_list1->empty() || m_list2->empty())
		return false;

	std::vector<Ruler*> rulers;//6
	//reused for img1 and img2
	std::vector<cv::Point3f> pp;//12
	std::vector<cv::Point3f> lines;//6
	//
	std::vector<cv::Point3f> intp1;//3, img1
	std::vector<cv::Point3f> intp2;//3, img2
	std::vector<cv::Vec4d> intp4;//3, 3d
	fluo::Point p;

	//img1
	//get rulers
	for (size_t i = 0; i < 6; ++i)
	{
		rulers.push_back(m_list1->GetRuler(m_names[i]));
		if (!rulers[i])
			return false;
		if (rulers[i]->GetNumPoint() < 2)
			return false;
	}
	//get points
	for (auto r : rulers)
	{
		r->SetWorkTime(0);
		p = r->GetPoint(0);
		pp.push_back(normalize_homo(p));
		p = r->GetPoint(1);
		pp.push_back(normalize_homo(p));
	}
	//get lines
	for (size_t i = 0; i < pp.size(); i+=2)
	{
		lines.push_back(pp[i].cross(pp[i + 1]));
	}
	//get intersections in 2d
	for (size_t i = 0; i < lines.size(); i += 2)
	{
		intp1.push_back(lines[i].cross(lines[i + 1]));
	}

	rulers.clear();
	pp.clear();
	lines.clear();
	//img2
	//get rulers
	for (size_t i = 0; i < 6; ++i)
	{
		rulers.push_back(m_list2->GetRuler(m_names[i]));
		if (!rulers[i])
			return false;
		if (rulers[i]->GetNumPoint() < 2)
			return false;
	}
	//get points
	for (auto r : rulers)
	{
		r->SetWorkTime(0);
		p = r->GetPoint(0);
		pp.push_back(normalize_homo(p));
		p = r->GetPoint(1);
		pp.push_back(normalize_homo(p));
	}
	//get lines
	for (size_t i = 0; i < pp.size(); i += 2)
	{
		lines.push_back(pp[i].cross(pp[i + 1]));
	}
	//get intersections in 2d
	for (size_t i = 0; i < lines.size(); i += 2)
	{
		intp2.push_back(lines[i].cross(lines[i + 1]));
	}

	//get intersections in 3d
	for (size_t i = 0; i < intp1.size(); ++i)
	{
		cv::Point2f pp1(intp1[i].x / intp1[i].z, intp1[i].y / intp1[i].z);
		cv::Point2f pp2(intp2[i].x / intp2[i].z, intp2[i].y / intp2[i].z);
		cv::Vec3d intp = triangulate(pp1, pp2, p1, p2);
		intp4.push_back(cv::Vec4d(intp(0), intp(1), intp(2), 1));
	}
	//get plane in 3d
	cv::Vec4d plane = get_plane(
		intp4[0], intp4[1], intp4[2]);
	//get homogeneous matrix
	m_h = std::make_unique<cv::Mat>(cv::Mat::eye(4, 4, CV_64F));
	m_h->at<double>(3, 0) = plane(0);
	m_h->at<double>(3, 1) = plane(1);
	m_h->at<double>(3, 2) = plane(2);
	m_h->at<double>(3, 3) = plane(3);
	return true;
}

cv::Vec3d Camera2Ruler::calib_affine(const cv::Vec3d& pp)
{
	cv::Vec4d rhp(
		m_h->at<double>(0, 0) * pp(0) + m_h->at<double>(0, 1) * pp(1) + m_h->at<double>(0, 2) * pp(2) + m_h->at<double>(0, 3),
		m_h->at<double>(1, 0) * pp(0) + m_h->at<double>(1, 1) * pp(1) + m_h->at<double>(1, 2) * pp(2) + m_h->at<double>(1, 3),
		m_h->at<double>(2, 0) * pp(0) + m_h->at<double>(2, 1) * pp(1) + m_h->at<double>(2, 2) * pp(2) + m_h->at<double>(2, 3),
		m_h->at<double>(3, 0) * pp(0) + m_h->at<double>(3, 1) * pp(1) + m_h->at<double>(3, 2) * pp(2) + m_h->at<double>(3, 3)
		);
	return cv::Vec3d(rhp(0) / rhp(3), rhp(1) / rhp(3), rhp(2) / rhp(3));
}

bool Camera2Ruler::calib_persp()
{
	if (!m_list_out)
		return false;
	if (m_list_out->empty())
		return false;

	Ruler* r1 = m_list_out->GetRuler(m_names[0]);
	Ruler* r2 = m_list_out->GetRuler(m_names[1]);
	if (!r1 || !r2)
		return false;
	if (r1->GetNumPoint() < 2 ||
		r2->GetNumPoint() < 2)
		return false;

	double d0, d1, s0, l0;
	fluo::Point p0 = r1->GetPoint(0);
	fluo::Point p1 = r1->GetPoint(1);
	fluo::Ray ray(p0, p1 - p0);
	ray.normalize();
	d0 = ray.distance(r2->GetPoint(0));
	d1 = ray.distance(r2->GetPoint(1));
	s0 = d0 / d1 - 1;
	l0 = (p1 - p0).length();

	//scale based on v0 and s in cylindrical system
	for (auto r : *m_list_out)
	{
		if (!r)
			continue;
		for (int i = 0; i < r->GetNumPoint(); ++i)
		{
			RulerPoint* rp = r->GetRulerPoint(i);
			if (!rp)
				continue;
			for (size_t tpi = 0; tpi < rp->GetTimeNum(); ++tpi)
			{
				size_t t = 0;
				fluo::Point p;
				if (rp->GetTimeAndPoint(tpi, t, p))
				{
					double l = ray.length(p);
					double s = s0 * l / l0 + 1;
					fluo::Vector v = p - p0;
					p = p0 + s * v;
					rp->SetPoint(p, t);
				}
			}
		}
	}
	return true;
}

bool Camera2Ruler::calib_metric()
{
	//name order x1, x2, y1, y2, z1, z2
	//each pointing to the axis direction from p0 to p1
	if (m_names.size() < 6)
		return false;
	if (!m_list1)
		return false;
	if (m_list_out->empty())
		return false;
	if (!m_list_out)
		return false;
	if (m_list_out->empty())
		return false;

	//find scale from first ruler in names
	Ruler* r1 = m_list1->GetRuler(m_names[0]);
	Ruler* ro = m_list_out->GetRuler(m_names[0]);
	double length = r1->GetLength();
	double scale = length / ro->GetLength();

	std::vector<Ruler*> rulers;
	for (size_t i = 0; i < 6; ++i)
	{
		rulers.push_back(m_list_out->GetRuler(m_names[i]));
		if (!rulers[i])
			return false;
		if (rulers[i]->GetNumPoint() < 2)
			return false;
	}
	std::vector<fluo::Point> pp;//12
	for (auto r : rulers)
	{
		pp.push_back(r->GetPoint(0));
		pp.push_back(r->GetPoint(1));
	}

	//find plane from yz
	fluo::Point p0;
	for (size_t i = 4; i < 12; ++i)
		p0 += pp[i];
	p0 = p0 / 8;
	std::vector<fluo::Vector> pv;
	pv.push_back(pp[7] - pp[4]);//0
	pv.push_back(pp[6] - pp[5]);//1
	pv.push_back(pp[11] - pp[8]);//2
	pv.push_back(pp[9] - pp[10]);//3
	pv.push_back(fluo::Cross(pv[0], pv[1]));//4
	pv.push_back(fluo::Cross(pv[2], pv[3]));//5
	fluo::Vector v0 = pv[4] + pv[5];
	v0.normalize();
	fluo::Plane pl(p0, v0);
	//x axis
	fluo::Ray axisx(pp[0], pp[1] - pp[0]);
	axisx.normalize();
	//intersection plane axisx
	fluo::Point o;
	pl.Intersect(axisx.origin(), axisx.direction(), o);
	//find each axis
	fluo::Vector x = pp[0] + pp[2] - pp[1] - pp[3];
	x.normalize();
	fluo::Vector y = pp[5] + pp[7] - pp[4] - pp[6];
	y.normalize();
	fluo::Vector z = pp[9] + pp[11] - pp[8] - pp[10];
	z.normalize();
	//build transform
	fluo::Transform tf(o, x, y, z);
	//second transform
	x = fluo::Vector(std::cos(fluo::d2r(m_slope)), std::sin(fluo::d2r(m_slope)), 0);
	//x = fluo::Vector(1, 0, 0);
	y = fluo::Vector(0, 1, 0);
	z = fluo::Vector(0, 0, 1);
	o = fluo::Point((m_nx - length) / 2, m_ny / 2, 0);
	fluo::Transform tf2(o, x, y, z);
	tf2.post_scale(fluo::Vector(scale));

	//correct points
	for (auto r : *m_list_out)
	{
		if (!r)
			continue;
		for (int i = 0; i < r->GetNumPoint(); ++i)
		{
			RulerPoint* rp = r->GetRulerPoint(i);
			if (!rp)
				continue;
			for (size_t tpi = 0; tpi < rp->GetTimeNum(); ++tpi)
			{
				size_t t = 0;
				fluo::Point p;
				if (rp->GetTimeAndPoint(tpi, t, p))
				{
					tf.unproject_inplace(p);
					tf2.project_inplace(p);
					rp->SetPoint(p, t);
				}
			}
		}
	}
	return true;
}

cv::Point2f Camera2Ruler::normalize(fluo::Point& p)
{
	cv::Point2f cvp =
	{
		float(p.x() / m_nx - 0.5),
		float(p.y() / m_nx - 0.5 * double(m_ny) / double(m_nx))
	};
	return cvp;
}

cv::Point3f Camera2Ruler::normalize_homo(fluo::Point& p)
{
	cv::Point3f cvp =
	{
		float(p.x() / m_nx - 0.5),
		float(p.y() / m_nx - 0.5 * double(m_ny) / double(m_nx)),
		1
	};
	return cvp;
}

cv::Vec3d Camera2Ruler::triangulate(
	const cv::Point2f& pp1, const cv::Point2f& pp2,
	const cv::Mat& p1, const cv::Mat& p2)
{
	cv::Vec2d u1(pp1.x, pp1.y);
	cv::Vec2d u2(pp2.x, pp2.y);
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
	return X;
}

cv::Vec4d Camera2Ruler::get_plane(
	const cv::Vec4d& pp1,
	const cv::Vec4d& pp2,
	const cv::Vec4d& pp3)
{
	//D234
	cv::Matx33d d123(
		pp1(1), pp2(1), pp3(1),
		pp1(2), pp2(2), pp3(2),
		pp1(3), pp2(3), pp3(3));
	//D134
	cv::Matx33d d023(
		pp1(0), pp2(0), pp3(0),
		pp1(2), pp2(2), pp3(2),
		pp1(3), pp2(3), pp3(3));
	//D124
	cv::Matx33d d013(
		pp1(0), pp2(0), pp3(0),
		pp1(1), pp2(1), pp3(1),
		pp1(3), pp2(3), pp3(3));
	//D123
	cv::Matx33d d012(
		pp1(0), pp2(0), pp3(0),
		pp1(1), pp2(1), pp3(1),
		pp1(2), pp2(2), pp3(2));
	cv::Vec4d X(
		cv::determinant(d123),
		-cv::determinant(d023),
		cv::determinant(d013),
		-cv::determinant(d012)
	);
	return X;
}