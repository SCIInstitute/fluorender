//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2004 Scientific Computing and Imaging Institute,
//  University of Utah.
//  
//  
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
//  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//  

#include <math.h>

#include <FLIVR/TextureBrick.h>
#include <FLIVR/Utils.h>
#include <utility>
#include <iostream>

using namespace std;

namespace FLIVR
{
	TextureBrick::TextureBrick (Nrrd* n0, Nrrd* n1,
		int nx, int ny, int nz, int nc, int* nb,
		int ox, int oy, int oz,
		int mx, int my, int mz,
		const BBox& bbox, const BBox& tbox)
		: nx_(nx), ny_(ny), nz_(nz), nc_(nc), ox_(ox), oy_(oy), oz_(oz),
		mx_(mx), my_(my), mz_(mz), bbox_(bbox), tbox_(tbox)
	{
		for (int i=0; i<TEXTURE_MAX_COMPONENTS; i++)
		{
			data_[i] = 0;
			nb_[i] = 0;
			ntype_[i] = TYPE_NONE;
		}

		for (int c=0; c<nc_; c++)
		{
			nb_[c] = nb[c];
		}
		if (nc_==1)
		{
			ntype_[0] = TYPE_INT;
		}
		else if (nc_>1)
		{
			ntype_[0] = TYPE_INT_GRAD;
			ntype_[1] = TYPE_GM;
		}

		compute_edge_rays(bbox_);
		compute_edge_rays_tex(tbox_);

		data_[0] = n0;
		data_[1] = n1;

		nmask_ = -1;

		//if it's been drawn in a full update loop
		for (int i=0; i<TEXTURE_RENDER_MODES; i++)
			drawn_[i] = false;

		//priority
		priority_ = 0;
	}

	TextureBrick::~TextureBrick()
	{
		// Creator of the brick owns the nrrds.  
		// This object never deletes that memory.
		data_[0] = 0;
		data_[1] = 0;
	}

	/* The cube is numbered in the following way 

	Corners:

		 2________6        y
		/|        |        |
	   / |       /|        |
	  /  |      / |        |
	 /   0_____/__4        |
	3---------7   /        |_________ x
	|  /      |  /         /
	| /       | /         /
	|/        |/         /
	1_________5         /
						z

	Edges:

		 ,____1___,        y
		/|        |        |
	   / 0       /|        |
	  9  |     10 2        |
	 /   |__3__/__|        |
	/_____5___/   /        |_________ x
	|  /      | 11         /
	4 8       6 /         /
	|/        |/         /
	|____7____/         /
						z
	*/

	void TextureBrick::compute_edge_rays(BBox &bbox)
	{
		// set up vertices
		Point corner[8];
		corner[0] = bbox.min();
		corner[1] = Point(bbox.min().x(), bbox.min().y(), bbox.max().z());
		corner[2] = Point(bbox.min().x(), bbox.max().y(), bbox.min().z());
		corner[3] = Point(bbox.min().x(), bbox.max().y(), bbox.max().z());
		corner[4] = Point(bbox.max().x(), bbox.min().y(), bbox.min().z());
		corner[5] = Point(bbox.max().x(), bbox.min().y(), bbox.max().z());
		corner[6] = Point(bbox.max().x(), bbox.max().y(), bbox.min().z());
		corner[7] = bbox.max();

		// set up edges
		edge_[0] = Ray(corner[0], corner[2] - corner[0]);
		edge_[1] = Ray(corner[2], corner[6] - corner[2]);
		edge_[2] = Ray(corner[4], corner[6] - corner[4]);
		edge_[3] = Ray(corner[0], corner[4] - corner[0]);
		edge_[4] = Ray(corner[1], corner[3] - corner[1]);
		edge_[5] = Ray(corner[3], corner[7] - corner[3]);
		edge_[6] = Ray(corner[5], corner[7] - corner[5]);
		edge_[7] = Ray(corner[1], corner[5] - corner[1]);
		edge_[8] = Ray(corner[0], corner[1] - corner[0]);
		edge_[9] = Ray(corner[2], corner[3] - corner[2]);
		edge_[10] = Ray(corner[6], corner[7] - corner[6]);
		edge_[11] = Ray(corner[4], corner[5] - corner[4]);
	}

	void TextureBrick::compute_edge_rays_tex(BBox &bbox)
	{
		// set up vertices
		Point corner[8];
		corner[0] = bbox.min();
		corner[1] = Point(bbox.min().x(), bbox.min().y(), bbox.max().z());
		corner[2] = Point(bbox.min().x(), bbox.max().y(), bbox.min().z());
		corner[3] = Point(bbox.min().x(), bbox.max().y(), bbox.max().z());
		corner[4] = Point(bbox.max().x(), bbox.min().y(), bbox.min().z());
		corner[5] = Point(bbox.max().x(), bbox.min().y(), bbox.max().z());
		corner[6] = Point(bbox.max().x(), bbox.max().y(), bbox.min().z());
		corner[7] = bbox.max();

		// set up edges
		tex_edge_[0] = Ray(corner[0], corner[2] - corner[0]);
		tex_edge_[1] = Ray(corner[2], corner[6] - corner[2]);
		tex_edge_[2] = Ray(corner[4], corner[6] - corner[4]);
		tex_edge_[3] = Ray(corner[0], corner[4] - corner[0]);
		tex_edge_[4] = Ray(corner[1], corner[3] - corner[1]);
		tex_edge_[5] = Ray(corner[3], corner[7] - corner[3]);
		tex_edge_[6] = Ray(corner[5], corner[7] - corner[5]);
		tex_edge_[7] = Ray(corner[1], corner[5] - corner[1]);
		tex_edge_[8] = Ray(corner[0], corner[1] - corner[0]);
		tex_edge_[9] = Ray(corner[2], corner[3] - corner[2]);
		tex_edge_[10] = Ray(corner[6], corner[7] - corner[6]);
		tex_edge_[11] = Ray(corner[4], corner[5] - corner[4]);
	}

	// compute polygon of edge plane intersections
	void TextureBrick::compute_polygon(Ray& view, double t,
		vector<double>& vertex, vector<double>& texcoord,
		vector<int>& size)
	{
		compute_polygons(view, t, t, 1.0, vertex, texcoord, size);
	}

	void TextureBrick::compute_polygons(Ray& view, double dt,
		vector<double>& vertex, vector<double>& texcoord,
		vector<int>& size)
	{
		Point corner[8];
		corner[0] = bbox_.min();
		corner[1] = Point(bbox_.min().x(), bbox_.min().y(), bbox_.max().z());
		corner[2] = Point(bbox_.min().x(), bbox_.max().y(), bbox_.min().z());
		corner[3] = Point(bbox_.min().x(), bbox_.max().y(), bbox_.max().z());
		corner[4] = Point(bbox_.max().x(), bbox_.min().y(), bbox_.min().z());
		corner[5] = Point(bbox_.max().x(), bbox_.min().y(), bbox_.max().z());
		corner[6] = Point(bbox_.max().x(), bbox_.max().y(), bbox_.min().z());
		corner[7] = bbox_.max();

		double tmin = Dot(corner[0] - view.origin(), view.direction());
		double tmax = tmin;
		int maxi = 0;
		for (int i=1; i<8; i++)
		{
			double t = Dot(corner[i] - view.origin(), view.direction());
			tmin = Min(t, tmin);
			if (t > tmax) { maxi = i; tmax = t; }
		}

		// Make all of the slices consistent by offsetting them to a fixed
		// position in space (the origin).  This way they are consistent
		// between bricks and don't change with camera zoom.
		double tanchor = Dot(corner[maxi], view.direction());
		double tanchor0 = floor(tanchor/dt)*dt;
		double tanchordiff = tanchor - tanchor0;
		tmax -= tanchordiff;

		compute_polygons(view, tmin, tmax, dt, vertex, texcoord, size);
	}

	// compute polygon list of edge plane intersections
	//
	// This is never called externally and could be private.
	//
	// The representation returned is not efficient, but it appears a
	// typical rendering only contains about 1k triangles.
	void TextureBrick::compute_polygons(Ray& view,
		double tmin, double tmax, double dt,
		vector<double>& vertex, vector<double>& texcoord,
		vector<int>& size)
	{
		Vector vv[12], tt[12]; // temp storage for vertices and texcoords

		int k = 0, degree = 0;

		// find up and right vectors
		Vector vdir = view.direction();
		view_vector_ = vdir;
		Vector up;
		Vector right;
		switch(MinIndex(fabs(vdir.x()),
			fabs(vdir.y()),
			fabs(vdir.z())))
		{
		case 0:
			up.x(0.0); up.y(-vdir.z()); up.z(vdir.y());
			break;
		case 1:
			up.x(-vdir.z()); up.y(0.0); up.z(vdir.x());
			break;
		case 2:
			up.x(-vdir.y()); up.y(vdir.x()); up.z(0.0);
			break;
		}
		up.normalize();
		right = Cross(vdir, up);
		for (double t = tmax; t >= tmin; t -= dt)
		{
			// we compute polys back to front
			// find intersections
			degree = 0;
			for (size_t j=0; j<12; j++)
			{
				double u;

				bool intersects = edge_[j].planeIntersectParameter
					(-view.direction(), view.parameter(t), u);
				if (intersects && u >= 0.0 && u <= 1.0)
				{
					Point p;
					p = edge_[j].parameter(u);
					vv[degree] = (Vector)p;
					p = tex_edge_[j].parameter(u);
					tt[degree] = (Vector)p;
					degree++;
				}
			}

			if (degree < 3 || degree >6) continue;

			if (degree > 3)
			{
				// compute centroids
				Vector vc(0.0, 0.0, 0.0), tc(0.0, 0.0, 0.0);
				for (int j=0; j<degree; j++)
				{
					vc += vv[j]; tc += tt[j];
				}
				vc /= (double)degree; tc /= (double)degree;

				// sort vertices
				int idx[6];
				double pa[6];
				for (int i=0; i<degree; i++)
				{
					double vx = Dot(vv[i] - vc, right);
					double vy = Dot(vv[i] - vc, up);

					// compute pseudo-angle
					pa[i] = vy / (fabs(vx) + fabs(vy));
					if (vx < 0.0) pa[i] = 2.0 - pa[i];
					else if (vy < 0.0) pa[i] = 4.0 + pa[i];
					// init idx
					idx[i] = i;
				}
				Sort(pa, idx, degree);

				// output polygon
				for (int j=0; j<degree; j++)
				{
					vertex.push_back(vv[idx[j]].x());
					vertex.push_back(vv[idx[j]].y());
					vertex.push_back(vv[idx[j]].z());
					texcoord.push_back(tt[idx[j]].x());
					texcoord.push_back(tt[idx[j]].y());
					texcoord.push_back(tt[idx[j]].z());
				}
			}
			else if (degree == 3)
			{
				// output a single triangle
				for (int j=0; j<degree; j++)
				{
					vertex.push_back(vv[j].x());
					vertex.push_back(vv[j].y());
					vertex.push_back(vv[j].z());
					texcoord.push_back(tt[j].x());
					texcoord.push_back(tt[j].y());
					texcoord.push_back(tt[j].z());
				}
			}
			k += degree;
			size.push_back(degree);
		}
	}

	int TextureBrick::sx()
	{
		if (data_[0]->dim == 3)
			return (int)data_[0]->axis[0].size;
		else
			return (int)data_[0]->axis[1].size;
	}

	int TextureBrick::sy()
	{
		if (data_[0]->dim == 3)
			return (int)data_[0]->axis[1].size;
		else
			return (int)data_[0]->axis[2].size;
	}

	GLenum TextureBrick::tex_type_aux(Nrrd* n)
	{
		// GL_BITMAP!?
		if (n->type == nrrdTypeChar)   return GL_BYTE;
		if (n->type == nrrdTypeUChar)  return GL_UNSIGNED_BYTE;
		if (n->type == nrrdTypeShort)  return GL_SHORT;
		if (n->type == nrrdTypeUShort) return GL_UNSIGNED_SHORT;
		if (n->type == nrrdTypeInt)    return GL_INT;
		if (n->type == nrrdTypeUInt)   return GL_UNSIGNED_INT;
		if (n->type == nrrdTypeFloat)  return GL_FLOAT;
		return GL_NONE;
	}

	size_t TextureBrick::tex_type_size(GLenum t)
	{
		if (t == GL_BYTE)           { return sizeof(GLbyte); }
		if (t == GL_UNSIGNED_BYTE)  { return sizeof(GLubyte); }
		if (t == GL_SHORT)          { return sizeof(GLshort); }
		if (t == GL_UNSIGNED_SHORT) { return sizeof(GLushort); }
		if (t == GL_INT)            { return sizeof(GLint); }
		if (t == GL_UNSIGNED_INT)   { return sizeof(GLuint); }
		if (t == GL_FLOAT)          { return sizeof(GLfloat); }
		return 0;
	}

	GLenum TextureBrick::tex_type(int c)
	{
		if (c < nc_)
			return tex_type_aux(data_[c]);
		else if (c == nmask_)
			return GL_UNSIGNED_BYTE;
		else if (c == nlabel_)
			return GL_UNSIGNED_INT;
		else
			return GL_NONE;
	}

	void *TextureBrick::tex_data(int c)
	{
		if (c >= 0 && data_[c])
		{
			unsigned char *ptr = (unsigned char *)(data_[c]->data);
			long long offset = long long(oz()) *
				long long(sx()) *
				long long(sy()) +
				long long(oy()) *
				long long(sx()) +
				long long(ox());
			return ptr + offset * tex_type_size(tex_type(c));
		}
		else
			return NULL;
	}

	void TextureBrick::set_priority()
	{
		if (!data_[0])
		{
			priority_ = 0;
			return;
		}
		size_t vs = tex_type_size(tex_type(0));
		size_t sx = data_[0]->axis[0].size;
		size_t sy = data_[0]->axis[1].size;
		if (vs == 1)
		{
			unsigned char max = 0;
			unsigned char *ptr = (unsigned char *)(data_[0]->data);
			for (int i=0; i<nx_; i++)
			for (int j=0; j<ny_; j++)
			for (int k=0; k<nz_; k++)
			{
				long long index = long long(sx)* long long(sy) *
					long long(oz_+k) + long long(sx) *
					long long(oy_+j) + long long(ox_+i);
				max = ptr[index]>max?ptr[index]:max;
			}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
		}
		else if (vs == 2)
		{
			unsigned short max = 0;
			unsigned short *ptr = (unsigned short *)(data_[0]->data);
			for (int i=0; i<nx_; i++)
			for (int j=0; j<ny_; j++)
			for (int k=0; k<nz_; k++)
			{
				long long index = long long(sx)* long long(sy) *
					long long(oz_+k) + long long(sx) *
					long long(oy_+j) + long long(ox_+i);
				max = ptr[index]>max?ptr[index]:max;
			}
			if (max == 0)
				priority_ = 1;
			else
				priority_ = 0;
		}
	}

} // end namespace FLIVR
