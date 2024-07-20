#ifndef BALLCLASSIFICATION_H
#define BALLCLASSIFICATION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

const std::vector<cv::Vec3b> LOWER_BOUNDS_CLASSIFICATION = {
	cv::Vec3b(0, 0, 153), //white 20, 10, 183
	cv::Vec3b(95, 224, 0) //black
};
//cv::Scalar lowerBoundColor(20, 0, 153);
//cv::Scalar upperBoundColor(90, 128, 255); // S: 102

const std::vector<cv::Vec3b> UPPER_BOUNDS_CLASSIFICATION = {
	cv::Vec3b(255, 102, 255), //white 90, 128, 255
	cv::Vec3b(125, 255, 38) //black
};

const int CLASSIFY_WHITE_THRESH = 101;
const int IGNORE_WHITE_THRESH = 26; // 28 seems good
const int CLASSIFY_BLACK_THRESH = 18;
const int IGNORE_BLACK_THRESH = 12;

std::vector<std::vector<int>> classifyBallAlt(const cv::Mat& image, std::vector<cv::Point> centerVector, std::vector<int> radiusVector);

cv::Mat extractRoi2(const cv::Mat& image, cv::Point center, int radius);

#endif