#include "Segmentator.h"

bool Segmentator::segmentTable(cv::Mat src, cv::Mat& dst, int threshold, int ksize, bool withOpening) {
	bool itWorked = false;

	// get the center coords
	int centerX = ceil((float)src.rows / 2), centerY = ceil((float)src.cols / 2);
	int kernelRadius = floor((float)ksize / 2);

	cv::Vec3f sum(0, 0, 0);
	for (int deltaRow = - kernelRadius; deltaRow < kernelRadius; ++deltaRow) {
		for (int deltaCol = - kernelRadius; deltaCol < kernelRadius; ++deltaCol) {
			sum += src.at<cv::Vec3b>(centerX + deltaRow, centerY + deltaCol);
		}
	}
	// compute the avg
	sum /= (ksize * ksize);
	cv::Vec3b avgColor(ceil(sum[0]), ceil(sum[1]), ceil(sum[2]));

	//debug
	std::cout << "AvgColor: " << avgColor << ", type:" << ((std::string("unsigned char").compare(typeid(avgColor[0]).name()) == 0) ? "" : " not") << " Vec3b" << std::endl;

	cv::Vec3b thresholdVec = cv::Vec3b::all(threshold);
	cv::inRange(src, avgColor - thresholdVec, avgColor + thresholdVec, dst);

	if (withOpening) {
		cv::erode(dst, dst, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)));
		cv::dilate(dst, dst, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)));
	}

	// it worked?
	double min, max;
	cv::minMaxLoc(dst, &min, &max);
	itWorked = (max != 0);

	if (itWorked) {
		cv::namedWindow("Segm");
		cv::imshow("Segm", dst);

		cv::namedWindow("debug");
		cv::Mat debug(200, 200, CV_8UC3);
		debug.setTo(avgColor);
		cv::imshow("debug", debug);
	}

	cv::waitKey(0);

	return itWorked;
}

bool Segmentator::segmentOtsu(cv::Mat src, cv::Mat& dst) {

	cv::threshold(src, dst, 0, 255, cv::THRESH_BINARY + cv::THRESH_OTSU);
	cv::imshow("Otsu debug", dst);

	return false; // itWorked
}

bool Segmentator::segmentRegionGrowing(cv::Mat src, cv::Mat& dst, int ksize) {

	int centerX = ceil((float)src.cols / 2), centerY = ceil((float)src.rows / 2);
	int kernelRadius = floor((float)ksize / 2);
	cv::Mat hsv;
	cv::cvtColor(src, hsv, cv::COLOR_BGR2HSV);
	cv::Vec3b centerColor = hsv.at<cv::Vec3b>(centerY, centerX);
	cv::Mat mask(src.rows, src.cols, CV_8U);
	cv::Mat segmented(src.rows, src.cols, CV_8U);
	segmented.setTo(0);
	std::vector<cv::Point> queueSeeds;
	queueSeeds.push_back(cv::Point(centerX, centerY));
	segmented.at<uchar>(centerY, centerX) = 255;

	//cv::Vec3f sum(0, 0, 0);
	//for (int deltaRow = -kernelRadius; deltaRow <= kernelRadius; ++deltaRow) {
	//	for (int deltaCol = -kernelRadius; deltaCol <= kernelRadius; ++deltaCol) {
	//		sum += src.at<cv::Vec3b>(centerX + deltaRow, centerY + deltaCol);
	//	}
	//}
	//// compute the avg
	//sum /= (ksize * ksize);
	//cv::Vec3b avgColor(ceil(sum[0]), ceil(sum[1]), ceil(sum[2]));

	cv::Vec3b lowerBound(centerColor[0] - 8, 119, 65); // center - 8, 127, 65
	// game1_clip1 - lower s: 119
	// game3_clip1 - lower s: 104 (for a correct cut for the first only also 119 should be fine)
	cv::Vec3b upperBound(centerColor[0] + 8, 255, 230); // center + 8, 255, 230

	cv::inRange(hsv, lowerBound, upperBound, mask);
	//cv::namedWindow("mask");
	//cv::imshow("mask", mask);
	
	//region growing
	while (!queueSeeds.empty()) {
		//std::cout << "I'm a new seed!!!!!!!" << std::endl;

		cv::Point actualSeed = queueSeeds[0];
		// 8-connected points are scanned
		for (int r = -1; r <= 1; ++r) {
			for (int c = -1; c <= 1; ++c) {
				cv::Point nearPoint(actualSeed.x + c, actualSeed.y + r);
				if (segmented.at<uchar>(nearPoint.y, nearPoint.x) != 255 && mask.at<uchar>(nearPoint.y, nearPoint.x) == 255) {
					segmented.at<uchar>(nearPoint.y, nearPoint.x) = 255;
					queueSeeds.push_back(nearPoint);
				}
			}
		}

		// remove the last-checked seed
		queueSeeds.erase(queueSeeds.begin());
	}

	dst = segmented;

	return false;
}

void Segmentator::drawSegmentation(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes) {
	cv::Point2f center;
	float radius;

	dst = src.clone();

	cv::drawContours(dst, std::vector<std::vector<cv::Point>>{vertices}, -1, BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[0], cv::FILLED);

	for (auto& bbox : bboxes) {
		cv::minEnclosingCircle(bbox.asPoints(), center, radius);
		cv::circle(dst, center, radius, BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[bbox.getCategID()], cv::FILLED);
	}
}