//AUTHOR: Matteo Ruta

#ifndef TABLEDETECTION_H
#define TABLEDETECTION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

void segmentRegionGrowing(cv::Mat src, cv::Mat& dst, int ksize = 21);

void detectLinesWithHoughTransform(cv::Mat src, cv::Mat& dst, std::vector<cv::Vec3f>& lines);

void getRectFromLines(std::vector<cv::Vec3f> lines, std::vector<cv::Point>& vertices, int verbose = 1);

#endif // TABLEDETECTION_H