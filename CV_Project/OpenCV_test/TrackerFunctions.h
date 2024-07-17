#include <opencv2/tracking.hpp>

#include "Bbox.h"

BBox trackBall(cv::Mat& frame, BBox bbox, cv::Ptr<cv::Tracker> tracker);//we might need std::Ptr
