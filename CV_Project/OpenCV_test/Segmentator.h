#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>

//debug purpose?
#include <iostream>
#include <string>

#include "Bbox.h"

class Segmentator {
public:
	bool segmentTable(cv::Mat src, cv::Mat& dst, int threshold = 10, int ksize = 5, bool withOpening = false);
	bool segmentOtsu(cv::Mat src, cv::Mat& dst);
	bool segmentRegionGrowing(cv::Mat src, cv::Mat& dst, int ksize = 21);

	void drawSegmentation(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes);
};