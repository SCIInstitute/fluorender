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
	Nrrd* nrrd = tex->get_nrrd(0);
	if (!nrrd) return -1.0;
	void* data = nrrd->data;
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
	int tmp_xx, tmp_yy, tmp_zz;
	fluo::Point nmp;
	double spcx, spcy, spcz;
	vd->GetSpacings(spcx, spcy, spcz);
	int resx, resy, resz;
	vd->GetResolution(resx, resy, resz, vd->GetLevel());
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
		mspc = sqrt(spcx*spcx + spcy * spcy + spcz * spcz) / vd->GetSampleRate();
	auto cb = vd->GetClippingBox();
	int counter = 0;//counter to determine if the ray casting has run
	if (bbox.intersect(mp1, vv, hit))
	{
		int brick_id = -1;
		flvr::TextureBrick* hit_brick = 0;
		unsigned long long vindex;
		int data_nx, data_ny, data_nz;
		if (vd->isBrxml())
		{
			data_nx = tex->nx();
			data_ny = tex->ny();
			data_nz = tex->nz();
		}

		while (true)
		{
			tmp_xx = int(hit.x() / spcx);
			tmp_yy = int(hit.y() / spcy);
			tmp_zz = int(hit.z() / spcz);
			if (mode == 1 &&
				tmp_xx == xx && tmp_yy == yy && tmp_zz == zz)
			{
				//same, skip
				hit += vv * mspc;
				continue;
			}
			else
			{
				xx = tmp_xx;
				yy = tmp_yy;
				zz = tmp_zz;
			}
			//out of bound, stop
			if (xx<0 || xx>resx ||
				yy<0 || yy>resy ||
				zz<0 || zz>resz)
				break;
			//normalize
			nmp.x(hit.x() / bbox.Max().x());
			nmp.y(hit.y() / bbox.Max().y());
			nmp.z(hit.z() / bbox.Max().z());
			if (cb.ContainsWorld(nmp))
			{
				xx = xx == resx ? resx - 1 : xx;
				yy = yy == resy ? resy - 1 : yy;
				zz = zz == resz ? resz - 1 : zz;

				//if it's multiresolution, get brick first
				if (vd->isBrxml())
				{
					vindex = (unsigned long long)data_nx*(unsigned long long)data_ny*
						(unsigned long long)zz + (unsigned long long)data_nx*
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
						ii = xx - hit_brick->ox();
						jj = yy - hit_brick->oy();
						kk = zz - hit_brick->oz();
						if (use_transf)
							value = vd->GetTransferedValue(ii, jj, kk, hit_brick);
						else
							value = vd->GetOriginalValue(ii, jj, kk, hit_brick);
					}
				}
				else
				{
					if (use_transf)
						value = vd->GetTransferedValue(xx, yy, zz);
					else
						value = vd->GetOriginalValue(xx, yy, zz);
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
	mp.scale(spcx, spcy, spcz);

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

double VolumePoint::GetPointVolumeBox(
	double mx, double my,//mouse coord on screen
	bool calc_mats,
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

	fluo::Transform mv;
	fluo::Transform p;
	glm::mat4 mv_temp;
	fluo::Transform *tform = vd->GetTexture()->transform();
	double mvmat[16];
	tform->get_trans(mvmat);

	if (calc_mats)
	{
		//projection
		//HandleProjection(nx, ny);
		//Transformation
		//HandleCamera();
		mv_temp = view->GetObjectMat();
	}
	else
		mv_temp = view->GetModelView();//m_mv_mat;

	glm::mat4 mv_mat2 = glm::mat4(
		mvmat[0], mvmat[4], mvmat[8], mvmat[12],
		mvmat[1], mvmat[5], mvmat[9], mvmat[13],
		mvmat[2], mvmat[6], mvmat[10], mvmat[14],
		mvmat[3], mvmat[7], mvmat[11], mvmat[15]);
	mv_temp = mv_temp * mv_mat2;
	glm::mat4 prj_mat = view->GetProjection();
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
	auto planes = cb.GetPlanesWorld();
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
			if (cb.ContainsWorld(pp))
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

double VolumePoint::GetPointVolumeBox2(
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

	//projection
	//HandleProjection(nx, ny);
	//Transformation
	//HandleCamera();
	glm::mat4 mv_temp = view->GetObjectMat();
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
	auto planes = cb.GetPlanesWorld();
	//for each plane, calculate the intersection point
	for (int i = 0; i < 6; i++)
	{
		auto plane = planes[i];
		fluo::Vector vec = plane.normal();
		fluo::Point pnt = plane.get_point();
		if (ray.planeIntersectParameter(vec, pnt, t))
		{
			pp = ray.parameter(t);

			if (cb.ContainsWorld(pp))
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
	fluo::Point* planep, bool calc_mats,
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

	glm::mat4 mv_temp;

	if (calc_mats)
	{
		//projection
		view->HandleProjection(nx, ny);
		//Transformation
		view->HandleCamera();
		mv_temp = view->GetObjectMat();
	}
	else
		mv_temp = view->GetModelView();//m_mv_mat;

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

