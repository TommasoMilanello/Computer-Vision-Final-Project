#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <algorithm>

class LinesDetector {
public:
	bool detectLinesWithHoughTransform(cv::Mat src, cv::Mat& dst, std::vector<cv::Vec3f>& lines);
	bool detectCirclesWithHoughTransform(cv::Mat src, cv::Mat dst);
	bool detectAnglesWithHarris(cv::Mat src, cv::Mat dst);

	bool getRectFromLines(std::vector<cv::Vec3f> lines, std::vector<cv::Point>& vertices, int verbose = 1);
};