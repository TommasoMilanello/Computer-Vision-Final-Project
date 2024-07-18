#ifndef TRACKERFUNCTIONS_H
#define TRACKERFUNCTIONS_H

#include <opencv2/tracking.hpp>

#include "Bbox.h"

void trackBalls(cv::Mat& frame, std::vector<BBox>& bBoxes, std::vector<cv::Ptr<cv::Tracker>> trackers);

#endif // TRACKERFUNCTIONS_H