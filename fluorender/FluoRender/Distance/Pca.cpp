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
#include <Distance/Pca.h>
#include <utility.h>
#include <Algorithm>

using namespace FL;

/*void Pca::Compute()
{
	int N = m_points.size();
	if (N < 3)
		return;
	FLIVR::Point mean;
	//get point vector
	Eigen::MatrixXd points(N, 3);
	for (int i = 0; i < N; ++i)
	{
		points(i, 0) = m_points[i].x();
		points(i, 1) = m_points[i].y();
		points(i, 2) = m_points[i].z();
		mean += m_points[i];
	}
	//mean value
	mean /= N;
	//centered matrix
	for (int i = 0; i < N; ++i)
	{
		points(i, 0) -= mean.x();
		points(i, 1) -= mean.y();
		points(i, 2) -= mean.z();
	}
	//covariance matrix
	Eigen::MatrixXd cov(N, N);
	cov = points.transpose() * points;
	//eigen
	Eigen::EigenSolver<Eigen::MatrixXd> solver(cov);
	Eigen::MatrixXd eigenVectors = solver.eigenvectors().real();
	Eigen::VectorXd eigenValues = solver.eigenvalues().real();
	//eigenVectors.normalize();
	//output
	std::vector<double> values(3);
	values.push_back(eigenValues[0]);
	values.push_back(eigenValues[1]);
	values.push_back(eigenValues[2]);
	//sort large to small
	std::sort(values.begin(), values.end(), std::greater<double>());
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (values[i] == eigenValues(j))
			{
				m_values[i] = values[i];
				m_axis[i] = FLIVR::Vector(
					eigenVectors(j, 0),
					eigenVectors(j, 1),
					eigenVectors(j, 2));
				break;
			}
		}
	}
}*/

void Pca::Compute()
{
	int N = m_points.size();
	if (N < 2)
		return;

	std::vector<FLIVR::Point> points;
	//mean
	FLIVR::Point mean;
	for (int i = 0; i < N; ++i)
	{
		points.push_back(m_points[i]);
		mean += m_points[i];
	}
	mean /= N;
	//centered matrix
	for (int i = 0; i < N; ++i)
		points[i] -= mean;
	//covariance matrix
	double cov[3][3] = {
		0, 0, 0,
		0, 0, 0,
		0, 0, 0 };

	for (int i = 0; i < 3; ++i)
	for (int j = 0; j < 3; ++j)
	{
		for (int n = 0; n < N; ++n)
			cov[i][j] += (points[n])(i) * (points[n])(j);
	}
	//eigen
	//simple method for real symmetric cov
	double eig[3] = { 0,0,0 };
	double p1 =
		cov[0][1] * cov[0][1] +
		cov[0][2] * cov[0][2] +
		cov[1][2] * cov[1][2];
	if (fabs(p1) < EPS)
	{
		//cov is diagonal
		eig[0] = cov[0][0];
		eig[1] = cov[1][1];
		eig[2] = cov[2][2];
	}
	else
	{
		double q = (cov[0][0] + cov[1][1] + cov[2][2]) / 3.0;
		double p2 =
			(cov[0][0] - q) * (cov[0][0] - q) +
			(cov[1][1] - q) * (cov[1][1] - q) +
			(cov[2][2] - q) * (cov[2][2] - q) + 2.0 * p1;
		double p = std::sqrt(p2 / 6.0);
		double matb[3][3] = {
			1.0 / p * (cov[0][0] - q), 1.0 / p * cov[0][1], 1.0 / p * cov[0][2],
			1.0 / p * cov[1][0], 1.0 / p * (cov[1][1] - q), 1.0 / p * cov[1][2],
			1.0 / p * cov[2][0], 1.0 / p * cov[2][1], 1.0 / p * (cov[2][2] - q) };
		double r =
			matb[0][0] * (matb[1][1] * matb[2][2] - matb[1][2] * matb[2][1]) -
			matb[0][1] * (matb[1][0] * matb[2][2] - matb[1][2] * matb[2][0]) +
			matb[0][2] * (matb[1][0] * matb[2][1] - matb[1][1] * matb[2][0]);
		r /= 2.0;

		double phi;
		if (r <= -1)
			phi = PI / 3;
		else if (r >= 1)
			phi = 0.0;
		else
			phi = std::acos(r) / 3.0;

		eig[0] = q + 2.0 * p * std::cos(phi);
		eig[2] = q + 2.0 * p * std::cos(phi + (2.0 * PI / 3.0));
		eig[1] = 3.0 * q - eig[0] - eig[2];
	}
	//sort
	std::sort(eig, eig + 3, std::greater<double>());
	//eigenvectors
	double mat1[3][3] = {
		cov[0][0] - eig[0], cov[0][1], cov[0][2],
		cov[1][0], cov[1][1] - eig[0], cov[1][2],
		cov[2][0], cov[2][1], cov[2][2] - eig[0] };
	double mat2[3][3] = {
		cov[0][0] - eig[1], cov[0][1], cov[0][2],
		cov[1][0], cov[1][1] - eig[1], cov[1][2],
		cov[2][0], cov[2][1], cov[2][2] - eig[1] };
	double mat3[3][3] = {
		cov[0][0] - eig[2], cov[0][1], cov[0][2],
		cov[1][0], cov[1][1] - eig[2], cov[1][2],
		cov[2][0], cov[2][1], cov[2][2] - eig[2] };
	//ev1
	double eigv1[3][3] = {
		mat2[0][0] * mat3[0][0] + mat2[0][1] * mat3[1][0] + mat2[0][2] * mat3[2][0],
		mat2[0][0] * mat3[0][1] + mat2[0][1] * mat3[1][1] + mat2[0][2] * mat3[2][1],
		mat2[0][0] * mat3[0][2] + mat2[0][1] * mat3[1][2] + mat2[0][2] * mat3[2][2],
		mat2[1][0] * mat3[0][0] + mat2[1][1] * mat3[1][0] + mat2[1][2] * mat3[2][0],
		mat2[1][0] * mat3[0][1] + mat2[1][1] * mat3[1][1] + mat2[1][2] * mat3[2][1],
		mat2[1][0] * mat3[0][2] + mat2[1][1] * mat3[1][2] + mat2[1][2] * mat3[2][2],
		mat2[2][0] * mat3[0][0] + mat2[2][1] * mat3[1][0] + mat2[2][2] * mat3[2][0],
		mat2[2][0] * mat3[0][1] + mat2[2][1] * mat3[1][1] + mat2[2][2] * mat3[2][1],
		mat2[2][0] * mat3[0][2] + mat2[2][1] * mat3[1][2] + mat2[2][2] * mat3[2][2] };
	double eigv2[3][3] = {
		mat1[0][0] * mat3[0][0] + mat1[0][1] * mat3[1][0] + mat1[0][2] * mat3[2][0],
		mat1[0][0] * mat3[0][1] + mat1[0][1] * mat3[1][1] + mat1[0][2] * mat3[2][1],
		mat1[0][0] * mat3[0][2] + mat1[0][1] * mat3[1][2] + mat1[0][2] * mat3[2][2],
		mat1[1][0] * mat3[0][0] + mat1[1][1] * mat3[1][0] + mat1[1][2] * mat3[2][0],
		mat1[1][0] * mat3[0][1] + mat1[1][1] * mat3[1][1] + mat1[1][2] * mat3[2][1],
		mat1[1][0] * mat3[0][2] + mat1[1][1] * mat3[1][2] + mat1[1][2] * mat3[2][2],
		mat1[2][0] * mat3[0][0] + mat1[2][1] * mat3[1][0] + mat1[2][2] * mat3[2][0],
		mat1[2][0] * mat3[0][1] + mat1[2][1] * mat3[1][1] + mat1[2][2] * mat3[2][1],
		mat1[2][0] * mat3[0][2] + mat1[2][1] * mat3[1][2] + mat1[2][2] * mat3[2][2] };
	//extract
	m_values[0] = eig[0];
	m_values[1] = eig[1];
	m_values[2] = eig[2];
	int col = 0;
	do
	{
		m_axis[0] = FLIVR::Vector(eigv1[0][col], eigv1[1][col], eigv1[2][col]);
		col++;
	} while (m_axis[0].normalize() <= EPS);
	col = 0;
	do
	{
		m_axis[1] = FLIVR::Vector(eigv2[0][col], eigv2[1][col], eigv2[2][col]);
		col++;
	} while (m_axis[1].normalize() <= EPS);
	m_axis[2] = Cross(m_axis[0], m_axis[1]);
	m_axis[2].normalize();
}