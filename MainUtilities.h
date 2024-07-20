//AUTHOR: Tommaso Milanello

#ifndef MAINUTILITIES_H
#define MAINUTILITIES_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/filesystem.hpp>
#include <iostream>
#include <fstream>

#include "BBox.h"

std::vector<cv::Mat> multipleImRead(const std::string& path, const std::string& pattern, bool asGray);

std::vector<std::vector<BBox>> multipleBBoxRead(const std::string& path, const std::string& pattern);

cv::Point computeCenterOfTableShape(std::vector<cv::Point> vertices);

cv::Point differenceVector(cv::Point p1, cv::Point p2);

std::vector<BBox> convertIntoBBoxes(std::vector<std::vector<int>> toBboxes);

#endif // MAINUTILITIES_H