﻿//  
//  For more information, please see: http://software.sci.utah.edu
//  
//  The MIT License
//  
//  Copyright (c) 2025 Scientific Computing and Imaging Institute,
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

#ifndef _FLTRANSFORM_H_
#define _FLTRANSFORM_H_

#include <string>
#include <iostream>
#include <iomanip>

namespace fluo
{
	class Vector;
	class Point;
	class Plane;
	class Transform;

	class Transform
	{
		double mat[4][4];
		mutable double imat[4][4];
		mutable int inverse_valid;
		void install_mat(double[4][4]);
		void build_permute(double m[4][4], int, int, int, int pre);
		void build_rotate(double m[4][4], double, const Vector&);
		void build_shear(double mat[4][4], const Vector&, const Plane&);
		void build_scale(double m[4][4], const Vector&);
		void build_translate(double m[4][4], const Vector&);
		void pre_mulmat(const double[4][4]);
		void post_mulmat(const double[4][4]);
		void invmat(double[4][4]);
		void switch_rows(double m[4][4], int row1, int row2) const;
		void sub_rows(double m[4][4], int row1, int row2, double mul) const;
		void load_identity(double[4][4]);
		void load_zero(double[4][4]);
	protected:

		double get_imat_val(int i, int j) const { return imat[i][j]; }
		void set_imat_val(int i, int j, double val) { imat[i][j] = val; }

	public:
		Transform();
		Transform(const Transform&);
		Transform& operator=(const Transform&);
		~Transform();
		Transform(const Point&, const Vector&, const Vector&, const Vector&);

		bool operator==(const Transform&) const;
		bool operator!=(const Transform&) const;

		double get_mat_val(int i, int j) const { return mat[i][j]; }
		void set_mat_val(int i, int j, double val) { mat[i][j] = val; }

		void load_basis(const Point&,const Vector&, const Vector&, const Vector&);
		void load_frame(const Vector&, const Vector&, const Vector&);

		void change_basis(Transform&);
		void post_trans(const Transform&);
		void pre_trans(const Transform&);

		void print(void);
		void printi(void);

		void pre_permute(int xmap, int ymap, int zmap);
		void post_permute(int xmap, int ymap, int zmap);
		void pre_scale(const Vector&);
		void post_scale(const Vector&);
		void pre_shear(const Vector&, const Plane&);
		void post_shear(const Vector&, const Plane&);
		void pre_rotate(double, const Vector& axis);
		void post_rotate(double, const Vector& axis);
		// Returns true if the rotation happened, false otherwise.
		bool rotate(const Vector& from, const Vector& to);
		void pre_translate(const Vector&);
		void post_translate(const Vector&);

		Point unproject(const Point& p) const;
		void unproject(const Point& p, Point& res) const;
		void unproject_inplace(Point& p) const;
		Vector unproject(const Vector& p) const;
		void unproject(const Vector& v, Vector& res) const;
		void unproject_inplace(Vector& v) const;

		Point transform(const Point& p) const;
		Vector transform(const Vector& v) const;
		void transform_inplace(Point& p) const;
		void transform_inplace(Vector& v) const;

		Point project(const Point& p) const;
		void project(const Point& p, Point& res) const;
		void project_inplace(Point& p) const;
		Vector project(const Vector& p) const;
		void project(const Vector& p, Vector& res) const;
		void project_inplace(Vector& p) const;
		Vector project_normal(const Vector&) const;
		void project_normal(const Vector&, Vector& res) const;
		void project_normal_inplace(Vector&) const;
		void get(double*) const;
		void get(float*) const;
		void get_trans(double*) const;
		void get_trans(float*) const;
		void set(double*);
		void set(float*);
		void set_trans(double*);
		void set_trans(float*);
		void load_identity();
		void perspective(const Point& eyep, const Point& lookat,
			const Vector& up, double fov,
			double znear, double zfar,
			int xres, int yres);
		void compute_imat() const;
		void invert();
		bool inv_valid() { return inverse_valid!=0; }
		void set_inv_valid(bool iv) { inverse_valid = iv; }

		friend std::ostream& operator<<(std::ostream& os, const Transform& t)
		{
			os << std::defaultfloat << std::setprecision(std::numeric_limits<double>::max_digits10);
			os << "[[" << t.mat[0][0] << ',' << t.mat[0][1] << ',' << t.mat[0][2] << ',' << t.mat[0][3] << "],";
			os <<  "[" << t.mat[1][0] << ',' << t.mat[1][1] << ',' << t.mat[1][2] << ',' << t.mat[1][3] << "],";
			os <<  "[" << t.mat[2][0] << ',' << t.mat[2][1] << ',' << t.mat[2][2] << ',' << t.mat[2][3] << "],";
			os <<  "[" << t.mat[3][0] << ',' << t.mat[3][1] << ',' << t.mat[3][2] << ',' << t.mat[3][3] << "]]";
			return os;
		}
		friend std::istream& operator >> (std::istream& is, Transform& t)
		{
			double m[4][4];
			char st;
			is >> st >> st >> m[0][0] >> st >> m[0][1] >> st >> m[0][2] >> st >> m[0][3] >> st >> st;
			is >>       st >> m[1][0] >> st >> m[1][1] >> st >> m[1][2] >> st >> m[1][3] >> st >> st;
			is >>       st >> m[2][0] >> st >> m[2][1] >> st >> m[2][2] >> st >> m[2][3] >> st >> st;
			is >>       st >> m[3][0] >> st >> m[3][1] >> st >> m[3][2] >> st >> m[3][3] >> st >> st;
			t.install_mat(m);
			return is;
		}
	};

	Point operator*(Transform &t, const Point &d);
	Vector operator*(Transform &t, const Vector &d);

} // End namespace fluo

#endif
