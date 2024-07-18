#ifndef BALLDETECTION_H
#define BALLDETECTION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BBox.h"

std::vector<std::tuple<cv::Point, int, int>> detectBalls(const cv::Mat& segmented, const cv::Mat& mask);

cv::Mat extractRoi(const cv::Mat& image, cv::Point center, int radius);

int classifyBall(const cv::Mat& roi);

#endif // BALLDETECTION_H