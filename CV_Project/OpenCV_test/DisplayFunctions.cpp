#include "DisplayFunctions.h"

void drawSegmentation(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes) {
	cv::Point2f center;
	float radius;

	dst = src.clone();

	cv::drawContours(dst, std::vector<std::vector<cv::Point>>{vertices}, -1, BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[0], cv::FILLED);

	for (auto& bbox : bboxes) {
		cv::minEnclosingCircle(bbox.asPoints(), center, radius);
		cv::circle(dst, center, radius, BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[bbox.getCategID()], cv::FILLED);
	}
}

void drawBallLocalization(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes, bool asBboxes) {
	dst = src.clone();

	cv::drawContours(dst, std::vector<std::vector<cv::Point>>{vertices}, -1, cv::Scalar(0, 255, 255), 2);

	for (auto& bbox : bboxes) {
		if (asBboxes) {
			cv::rectangle(dst, bbox.asRect(), BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[bbox.getCategID()]);
		}
		else {
			cv::circle(dst, bbox.getCenter(), bbox.getMaxRadius(), BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[bbox.getCategID()]);
		}
	}
}

void drawFrameWithMiniMap(const cv::Mat& src, cv::Mat& dst, const MiniMap& map) {
	dst = src.clone();

	map.drawMiniMapOnFrame(dst);
}