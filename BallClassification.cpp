//AUTHOR: Ilyas Issa

#include "BallClassification.h"
#include "BallDetection.h"

std::vector<std::vector<int>> classifyBalls(const cv::Mat& image, std::vector<cv::Point> centerVector, std::vector<int> radiusVector) {
	std::vector<std::vector<int>> classifiedBalls;
	std::vector<int> whitePixelsInRoi, blackPixelsInRoi, blackList;
	int maxElWhite, maxElBlack;

	//counting white and black pixels in each ball
	for (int i = 0; i < centerVector.size(); ++i) {
		cv::Mat whitePixels, blackPixels;
		cv::Mat roi = extractRoi(image, centerVector[i], radiusVector[i]);
		
		cv::Mat hsvRoi;
		cv::cvtColor(roi, hsvRoi, cv::COLOR_BGR2HSV);
		cv::inRange(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[0], UPPER_BOUNDS_CLASSIFICATION[0], whitePixels);
		cv::inRange(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[1], UPPER_BOUNDS_CLASSIFICATION[1], blackPixels);
        cv::normalize(whitePixels, whitePixels, 0, 1, cv::NORM_MINMAX);
        cv::normalize(blackPixels, blackPixels, 0, 1, cv::NORM_MINMAX);
		whitePixelsInRoi.push_back(cv::sum(whitePixels)[0]);
		blackPixelsInRoi.push_back(cv::sum(blackPixels)[0]);
	}

	//select white and striped balls
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

	//select black and solid balls
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
