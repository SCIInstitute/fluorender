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
#include <Algorithm>

using namespace FL;

void Pca::Compute()
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
}