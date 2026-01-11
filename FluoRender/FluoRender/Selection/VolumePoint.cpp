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

#include <VolumePoint.h>
#include <Global.h>
#include <CurrentObjects.h>
#include <VolumeData.h>
#include <RenderView.h>
#include <Texture.h>
#include <TextureBrick.h>
#include <VolumeRenderer.h>
#include <Point.h>
#include <glm/gtc/type_ptr.hpp>

using namespace flrd;

double VolumePoint::GetPointVolume(
	double mx, double my,//mouse coord on screen
	int mode, bool use_transf, double thresh,//params
	fluo::Point &mp, fluo::Point &ip)
{
	auto view = glbin_current.render_view.lock();
	auto vd = m_vd.lock();
	if (!vd)
		vd = glbin_current.vol_data.lock();
	if (!view || !vd)
		return -1.0;
	int nx = view->GetCanvasSize().w();
	int ny = view->GetCanvasSize().h();
	if (nx <= 0 || ny <= 0)
		return -1.0;

	flvr::Texture* tex = vd->GetTexture();
	if (!tex) return -1.0;
	auto comp = tex->get_nrrd(flvr::CompType::Data);
	if (!comp.data) return -1.0;
	void* data = comp.data->data;
	if (!data && vd->GetAllBrickNum() < 1) return -1.0;

	//projection
	view->HandleProjection(nx, ny);
	//Transformation
	view->HandleCamera();
	glm::mat4 mv_temp = view->GetInvtMat();
	glm::mat4 prj_mat = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(prj_mat));

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to object space
	fluo::Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	mp1 = mv.transform(mp1);
	fluo::Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	mp2 = mv.transform(mp2);

	//volume res
	int xx = -1;
	int yy = -1;
	int zz = -1;
	fluo::Vector temp_xyz;
	fluo::Point nmp;
	auto spc = vd->GetSpacing();
	auto res = vd->GetResolution(vd->GetLevel());
	//volume bounding box
	fluo::BBox bbox = vd->GetBounds();
	fluo::Vector vv = mp2 - mp1;
	vv.normalize();
	fluo::Point hit;
	double max_int = 0.0;
	double alpha = 0.0;
	double value = 0.0;
	double mspc = 1.0;
	if (vd->GetSampleRate() > 0.0)
		mspc = spc.length() / vd->GetSampleRate();
	auto cb = vd->GetClippingBox();
	int counter = 0;//counter to determine if the ray casting has run
	if (bbox.intersect(mp1, vv, hit))
	{
		int brick_id = -1;
		flvr::TextureBrick* hit_brick = 0;
		unsigned long long vindex;
		fluo::Vector data_res;
		if (vd->isBrxml())
		{
			data_res = tex->get_res();
		}

		while (true)
		{
			temp_xyz = fluo::Vector(hit) / spc;
			if (mode == 1 &&
				temp_xyz.intx() == xx && temp_xyz.inty() == yy && temp_xyz.intz() == zz)
			{
				//same, skip
				hit += vv * mspc;
				continue;
			}
			else
			{
				xx = temp_xyz.intx();
				yy = temp_xyz.inty();
				zz = temp_xyz.intz();
			}
			//out of bound, stop
			if (xx<0 || xx>res.intx() ||
				yy<0 || yy>res.inty() ||
				zz<0 || zz>res.intz())
				break;
			//normalize
			if (cb.ContainsWorld(hit))
			{
				xx = xx == res.intx() ? res.intx() - 1 : xx;
				yy = yy == res.inty() ? res.inty() - 1 : yy;
				zz = zz == res.intz() ? res.intz() - 1 : zz;

				//if it's multiresolution, get brick first
				if (vd->isBrxml())
				{
					vindex = (unsigned long long)data_res.get_size_xyz()*
						(unsigned long long)yy + (unsigned long long)xx;
					int id = tex->get_brick_id(vindex);
					if (id != brick_id)
					{
						//update hit brick
						hit_brick = tex->get_brick(id);
						brick_id = id;
					}
					if (hit_brick)
					{
						//coords in brick
						int ii, jj, kk;
						auto off_size = hit_brick->get_off_size();
						ii = xx - off_size.intx();
						jj = yy - off_size.inty();
						kk = zz - off_size.intz();
						if (use_transf)
							value = vd->GetTransferedValue(fluo::Point(ii, jj, kk), hit_brick);
						else
							value = vd->GetOriginalValue(fluo::Point(ii, jj, kk), hit_brick);
					}
				}
				else
				{
					if (use_transf)
						value = vd->GetTransferedValue(fluo::Point(xx, yy, zz));
					else
						value = vd->GetOriginalValue(fluo::Point(xx, yy, zz));
				}

				if (mode == 1)
				{
					if (value > max_int)
					{
						//mp = Point((xx + 0.5)*spcx, (yy + 0.5)*spcy, (zz + 0.5)*spcz);
						ip = fluo::Point(xx, yy, zz);
						max_int = value;
						counter++;
					}
				}
				else if (mode == 2)
				{
					//accumulate
					if (value > 0.0)
					{
						alpha = 1.0 - pow(fluo::Clamp(1.0 - value, 0.0, 1.0), vd->GetSampleRate());
						max_int += alpha * (1.0 - max_int);
						//mp = Point((xx + 0.5)*spcx, (yy + 0.5)*spcy, (zz + 0.5)*spcz);
						ip = fluo::Point(xx, yy, zz);
						counter++;
					}
					if (max_int > thresh || max_int >= 1.0)
						break;
				}
			}
			hit += vv * mspc;
		}
	}
	else
		return -1.0;

	if (counter == 0)
		return -1.0;

	mp = ip + fluo::Vector(0.5);
	mp.scale(spc);

	if (mode == 1)
	{
		if (max_int > 0.0)
			return (mp - mp1).length();
		else
			return -1.0;
	}
	else if (mode == 2)
	{
		if (max_int > thresh || max_int >= 1.0)
			return (mp - mp1).length();
		else
			return -1.0;
	}
	else
		return -1.0;
}

double VolumePoint::GetPointVolumeBoxOnePoint(
	double mx, double my,//mouse coord on screen
	fluo::Point &mp)
{
	auto view = glbin_current.render_view.lock();
	auto vd = m_vd.lock();
	if (!vd)
		vd = glbin_current.vol_data.lock();
	if (!view || !vd)
		return -1.0;
	int nx = view->GetCanvasSize().w();
	int ny = view->GetCanvasSize().h();
	if (nx <= 0 || ny <= 0)
		return -1.0;

	glm::mat4 mv_temp = view->GetModelView();
	fluo::Transform *tform = vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_temp = mv_temp * mv_mat2;
	glm::mat4 prj_mat = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(prj_mat));

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to object space
	fluo::Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	mp1 = mv.transform(mp1);
	fluo::Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	mp2 = mv.transform(mp2);
	fluo::Vector ray_d = mp1 - mp2;
	ray_d.normalize();
	fluo::Ray ray(mp1, ray_d);
	double mint = -1.0;
	double t;
	fluo::Point pp;//a point on plane

	auto cb = vd->GetClippingBox();
	auto planes = cb.GetPlanesUnit();
	//for each plane, calculate the intersection point
	for (int i = 0; i < 6; i++)
	{
		auto plane = planes[i];
		fluo::Vector vec = plane.normal();
		fluo::Point pnt = plane.get_point();
		if (ray.planeIntersectParameter(vec, pnt, t))
		{
			pp = ray.parameter(t);

			//determine if the point is inside the box
			if (cb.ContainsUnit(pp))
			{
				if (t > mint)
				{
					mp = pp;
					mint = t;
				}
			}
		}
	}

	mp = tform->transform(mp);

	return mint;
}

double VolumePoint::GetPointVolumeBoxTwoPoint(
	double mx, double my,//mouse coord on screen
	fluo::Point &p1, fluo::Point &p2)
{
	auto view = glbin_current.render_view.lock();
	auto vd = m_vd.lock();
	if (!vd)
		vd = glbin_current.vol_data.lock();
	if (!view || !vd)
		return -1.0;
	int nx = view->GetCanvasSize().w();
	int ny = view->GetCanvasSize().h();
	if (nx <= 0 || ny <= 0)
		return -1.0;

	glm::mat4 mv_temp = view->GetModelView();
	fluo::Transform *tform = vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);
	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_temp = mv_temp * mv_mat2;
	glm::mat4 prj_mat = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(prj_mat));

	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to object space
	fluo::Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	mp1 = mv.transform(mp1);
	fluo::Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	mp2 = mv.transform(mp2);
	fluo::Vector ray_d = mp1 - mp2;
	ray_d.normalize();
	fluo::Ray ray(mp1, ray_d);
	double mint = -1.0;
	double maxt = std::numeric_limits<double>::max();
	double t;
	fluo::Point pp;//a point on plane

	auto cb = vd->GetClippingBox();
	auto planes = cb.GetPlanesUnit();
	//for each plane, calculate the intersection point
	for (int i = 0; i < 6; i++)
	{
		auto plane = planes[i];
		fluo::Vector vec = plane.normal();
		fluo::Point pnt = plane.get_point();
		if (ray.planeIntersectParameter(vec, pnt, t))
		{
			pp = ray.parameter(t);

			if (cb.ContainsUnit(pp))
			{
				if (t > mint)
				{
					p1 = pp;
					mint = t;
				}
				if (t < maxt)
				{
					p2 = pp;
					maxt = t;
				}
			}
		}
	}

	p1 = tform->transform(p1);
	p2 = tform->transform(p2);

	return mint;
}

double VolumePoint::GetPointPlane(
	double mx, double my,//mouse coord on screen
	fluo::Point* planep,
	fluo::Point &mp)
{
	auto view = glbin_current.render_view.lock();
	auto vd = m_vd.lock();
	if (!vd)
		vd = glbin_current.vol_data.lock();
	if (!view || !vd)
		return -1.0;
	int nx = view->GetCanvasSize().w();
	int ny = view->GetCanvasSize().h();
	if (nx <= 0 || ny <= 0)
		return -1.0;

	glm::mat4 mv_temp = view->GetObjectMat();
	glm::mat4 prj_mat = view->GetProjection();
	fluo::Transform mv;
	fluo::Transform p;
	mv.set(glm::value_ptr(mv_temp));
	p.set(glm::value_ptr(prj_mat));

	fluo::Vector n(0.0, 0.0, 1.0);
	fluo::Point center(0.0, 0.0, -view->GetCenterEyeDist());
	if (planep)
	{
		center = *planep;
		center = mv.transform(center);
	}
	double x, y;
	x = mx * 2.0 / double(nx) - 1.0;
	y = 1.0 - my * 2.0 / double(ny);
	p.invert();
	mv.invert();
	//transform mp1 and mp2 to eye space
	fluo::Point mp1(x, y, 0.0);
	mp1 = p.transform(mp1);
	fluo::Point mp2(x, y, 1.0);
	mp2 = p.transform(mp2);
	fluo::Vector vec = mp2 - mp1;
	fluo::Ray ray(mp1, vec);
	double t = 0.0;
	if (ray.planeIntersectParameter(n, center, t))
		mp = ray.parameter(t);
	//transform mp to world space
	mp = mv.transform(mp);

	return (mp - mp1).length();
}

