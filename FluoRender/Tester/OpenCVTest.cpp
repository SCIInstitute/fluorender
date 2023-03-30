#include "tests.h"
#include "asserts.h"
#include <opencv2/opencv.hpp>
#include <vector>

void OpenCVTest0()
{
	std::vector<cv::Point2f> pp1, pp2;

	pp1.push_back({ 0, 0 });
	pp1.push_back({ 1, 0 });
	pp1.push_back({ 0, 1 });
	pp1.push_back({ 1, 1 });

	pp2.push_back({ 2, 0 });
	pp2.push_back({ 3, 0 });
	pp2.push_back({ 2, 1 });
	pp2.push_back({ 3, 1 });

	cv::Mat mat1 = cv::findFundamentalMat(
		pp1, pp2, cv::RANSAC, 3.0, 0.999
	);

	cv::Mat p1 = cv::Mat(cv::Point3d(0, 0, 1));
	cv::Mat p2 = cv::Mat(cv::Point3d(3, 0, 1));

	cv::Mat distance = p1.t() * mat1 * p2.t();

	std::cout << distance << std::endl;
}

