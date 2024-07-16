#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>

#include <iostream>

#include "Bbox.h"
#include "Seed.h"

class BallDetector {
public:
	void detectWhiteBall(const cv::Mat& src, cv::Mat& dst, const cv::Mat& tableArea);
	void templateMatching(const cv::Mat& src, cv::Mat& dst, cv::Mat& corr, const cv::Mat& templ, const cv::Mat& mask);

	void findBallSeeds(const cv::Mat& src, cv::Mat& dst, std::vector<Seed> seeds, cv::Mat& ballsSegm, const std::vector<cv::Point>& contour);

	void localizeBallsFromSeeds(const cv::Mat& src, cv::Mat& dst, const std::vector<Seed> seeds, std::vector<BBox> bboxes);
};