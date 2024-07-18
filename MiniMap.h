#ifndef MINIMAP_H
#define MINIMAP_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>

#include "BBox.h"

const std::string TABLE_SCHEME_PATH = "./map_background.png";
const float RESIZE_RATIO = 0.55;
const float RESIZE_PADDING = 0.01;
const int FIXED_RADIUS = 6;
const std::vector<cv::Point2f> MAIN_POINTS_WITH_IMAGE = {
	cv::Point(56, 48),
	cv::Point(634, 48),
	cv::Point(634, 351),
	cv::Point(56, 351),
	cv::Point(345, 200)
};

class MiniMap {
public:
	MiniMap();
	void drawMiniMapOnFrame(cv::Mat& frame) const;
	void computeHomography(const std::vector<cv::Point> corners, const cv::Point center, int verbose = 1);

	void initMiniMap(const std::vector<cv::Point> corners, const cv::Point center, const std::vector<BBox> bboxes, bool approxRadius = false);
	void updateMiniMap(const std::vector<BBox> newBboxes);

	void projectLastFrameOnMap();
private:
	cv::Mat Background;
	cv::Mat MapImg;
	std::vector<cv::Point2f> TableMainPoints;
	cv::Mat H;
	int radius;
	std::vector<std::vector<cv::Point>> ballCentersHistory;
	std::vector<int> ballCategID;
};

#endif // MINIMAP_H