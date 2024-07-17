#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BBox.h"
#include "MiniMap.h"

void drawSegmentation(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes);

void drawBallLocalization(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes, bool asBboxes = true);

void drawFrameWithMiniMap(const cv::Mat& src, cv::Mat& dst, MiniMap& map);

//cv::Mat drawResults(cv::Mat& frame, std::vector<BBox> balls, const std::vector<cv::Point>& tableContour);
