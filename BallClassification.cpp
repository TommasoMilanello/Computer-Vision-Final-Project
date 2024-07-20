//AUTHOR: Ilyas Issa

#include "BallClassification.h"

std::vector<std::vector<int>> classifyBallAlt(const cv::Mat& image, std::vector<cv::Point> centerVector, std::vector<int> radiusVector) {
	std::vector<std::vector<int>> classifiedBalls;
	std::vector<int> whitePixelsInRoi, blackPixelsInRoi, blackList;
	int maxElWhite, maxElBlack;

	for (int i = 0; i < centerVector.size(); ++i) {
		cv::Mat whitePixels, blackPixels;
		cv::Mat roi = extractRoi2(image, centerVector[i], radiusVector[i]);
		
		cv::Mat hsvRoi;
		cv::cvtColor(roi, hsvRoi, cv::COLOR_BGR2HSV);
		cv::inRange(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[0], UPPER_BOUNDS_CLASSIFICATION[0], whitePixels);
		cv::inRange(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[1], UPPER_BOUNDS_CLASSIFICATION[1], blackPixels);
        cv::normalize(whitePixels, whitePixels, 0, 1, cv::NORM_MINMAX);
        cv::normalize(blackPixels, blackPixels, 0, 1, cv::NORM_MINMAX);
		whitePixelsInRoi.push_back(cv::sum(whitePixels)[0]);
		blackPixelsInRoi.push_back(cv::sum(blackPixels)[0]);
	}

	maxElWhite = std::distance(whitePixelsInRoi.begin(), std::max_element(whitePixelsInRoi.begin(), whitePixelsInRoi.end()));
	for (int i = 0; i < whitePixelsInRoi.size(); ++i) {
		if (i == maxElWhite && whitePixelsInRoi[i] >= CLASSIFY_WHITE_THRESH) {
			classifiedBalls.push_back({centerVector[i].x, centerVector[i].y, radiusVector[i], 1});
			blackList.push_back(i);
		}
		else if (whitePixelsInRoi[i] >= IGNORE_WHITE_THRESH) {
			classifiedBalls.push_back({centerVector[i].x, centerVector[i].y, radiusVector[i], 4});
			blackList.push_back(i);
		}
	}

	maxElBlack = std::distance(blackPixelsInRoi.begin(), std::max_element(blackPixelsInRoi.begin(), blackPixelsInRoi.end()));
	for (int i = 0; i < blackPixelsInRoi.size(); ++i) {
		if (i == maxElBlack && blackPixelsInRoi[i] >= CLASSIFY_BLACK_THRESH) {
			classifiedBalls.push_back({centerVector[i].x, centerVector[i].y, radiusVector[i], 2});
		}
		else if (std::find(blackList.begin(), blackList.end(), i) == blackList.end()) {
			classifiedBalls.push_back({centerVector[i].x, centerVector[i].y, radiusVector[i], 3});
		}
	}

	return classifiedBalls;
}

cv::Mat extractRoi2(const cv::Mat& image, cv::Point center, int radius) {
	double factor = 0.9;
	int r = static_cast<int>(radius * factor);
	int x1 = cv::max(center.x - r, 0);
	int y1 = cv::max(center.y - r, 0);
	int x2 = cv::min(center.x + r, image.cols);
	int y2 = cv::min(center.y + r, image.rows);

	cv::Mat roi = image(cv::Rect(x1, y1, x2 - x1, y2 - y1));

	cv::Mat mask = cv::Mat::zeros(roi.size(), roi.type());
	cv::circle(mask, cv::Point(r, r), r, cv::Scalar(255, 255, 255), cv::FILLED);

	cv::Mat maskedRoi;
	bitwise_and(roi, mask, maskedRoi);
	return maskedRoi;
}