//AUTHOR: Ilyas Issa

#include "DisplayFunctions.h"

void drawSegmentation(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes) {
	dst = src.clone();

	cv::drawContours(dst, std::vector<std::vector<cv::Point>>{vertices}, -1, BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[PLAYING_FIELD_CATEG], cv::FILLED);

	for (auto& bbox : bboxes) {
		cv::circle(dst, bbox.getCenter(), bbox.getMaxRadius(), BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[bbox.getCategID()], cv::FILLED);
	}
}

void drawSegmentationMask(const cv::Mat& src, cv::Mat& dst, const cv::Mat& segmented, const std::vector<BBox> bboxes) {
	dst = cv::Mat::zeros(src.rows, src.cols, CV_8U);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(segmented, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
	cv::drawContours(dst, contours, -1, PLAYING_FIELD_CATEG, cv::FILLED);

	for (auto& bbox : bboxes) {
		cv::circle(dst, bbox.getCenter(), bbox.getMaxRadius(), bbox.getCategID(), cv::FILLED);
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

void drawFrameWithMiniMap(const cv::Mat& src, cv::Mat& dst, const std::vector<BBox> bboxes, const MiniMap& map) {
	dst = src.clone();
	map.drawMiniMapOnFrame(dst);
}