#include "BallClassification.h"

void computeAndDisplayHistogram(const cv::Mat& hsvRoi, const cv::Vec3b& lowerBound, const cv::Vec3b& upperBound) {
    // Split the HSV image into three channels
    std::vector<cv::Mat> hsvChannels;
    cv::split(hsvRoi, hsvChannels);

    // Define the number of bins for the histogram
    int hBins = 50, sBins = 60, vBins = 60;
    int histSize[] = { hBins, sBins, vBins };

    // Define the ranges for the channels
    float hRanges[] = { 0, 180 }; // Hue range
    float sRanges[] = { 0, 256 }; // Saturation range
    float vRanges[] = { 0, 256 }; // Value range
    const float* ranges[] = { hRanges, sRanges, vRanges };

    // Compute histograms for each channel
    cv::Mat hHist, sHist, vHist;
    cv::calcHist(&hsvChannels[0], 1, 0, cv::Mat(), hHist, 1, &hBins, &ranges[0]);
    cv::calcHist(&hsvChannels[1], 1, 0, cv::Mat(), sHist, 1, &sBins, &ranges[1]);
    cv::calcHist(&hsvChannels[2], 1, 0, cv::Mat(), vHist, 1, &vBins, &ranges[2]);

    // Normalize the histograms to the range [0, histImage.rows]
    int histHeight = 400, histWidth = 512;
    cv::normalize(hHist, hHist, 0, histHeight, cv::NORM_MINMAX);
    cv::normalize(sHist, sHist, 0, histHeight, cv::NORM_MINMAX);
    cv::normalize(vHist, vHist, 0, histHeight, cv::NORM_MINMAX);

    // Create images to display the histograms
    cv::Mat hHistImage(histHeight, histWidth, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat sHistImage(histHeight, histWidth, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat vHistImage(histHeight, histWidth, CV_8UC3, cv::Scalar(0, 0, 0));

    // Draw the histograms
    int binWidthH = cvRound((double)histWidth / hBins);
    for (int i = 1; i < hBins; i++) {
        cv::line(hHistImage,
                 cv::Point(binWidthH * (i - 1), histHeight - cvRound(hHist.at<float>(i - 1))),
                 cv::Point(binWidthH * i, histHeight - cvRound(hHist.at<float>(i))),
                 cv::Scalar(255, 0, 0), 2, 8, 0);
    }

    int binWidthS = cvRound((double)histWidth / sBins);
    for (int i = 1; i < sBins; i++) {
        cv::line(sHistImage,
                 cv::Point(binWidthS * (i - 1), histHeight - cvRound(sHist.at<float>(i - 1))),
                 cv::Point(binWidthS * i, histHeight - cvRound(sHist.at<float>(i))),
                 cv::Scalar(0, 255, 0), 2, 8, 0);
    }

    int binWidthV = cvRound((double)histWidth / vBins);
    for (int i = 1; i < vBins; i++) {
        cv::line(vHistImage,
                 cv::Point(binWidthV * (i - 1), histHeight - cvRound(vHist.at<float>(i - 1))),
                 cv::Point(binWidthV * i, histHeight - cvRound(vHist.at<float>(i))),
                 cv::Scalar(0, 0, 255), 2, 8, 0);
    }

    // Highlight the specified region in the histogram
    int lowerH = (lowerBound[0] * hBins) / 180;
    int upperH = (upperBound[0] * hBins) / 180;
    int lowerS = (lowerBound[1] * sBins) / 256;
    int upperS = (upperBound[1] * sBins) / 256;
    int lowerV = (lowerBound[2] * vBins) / 256;
    int upperV = (upperBound[2] * vBins) / 256;

    // Draw rectangles to highlight the range
    cv::rectangle(hHistImage, cv::Point(binWidthH * lowerH, 0), cv::Point(binWidthH * upperH, histHeight), cv::Scalar(0, 255, 255), 2);
    cv::rectangle(sHistImage, cv::Point(binWidthS * lowerS, 0), cv::Point(binWidthS * upperS, histHeight), cv::Scalar(255, 255, 0), 2);
    cv::rectangle(vHistImage, cv::Point(binWidthV * lowerV, 0), cv::Point(binWidthV * upperV, histHeight), cv::Scalar(255, 0, 255), 2);

    // Display the histograms
    cv::imshow("Hue Histogram", hHistImage);
    cv::imshow("Saturation Histogram", sHistImage);
    cv::imshow("Value Histogram", vHistImage);
    cv::waitKey(0); // Wait for a key press to close the histograms
}

std::vector<std::vector<int>> classifyBallAlt(const cv::Mat& image, std::vector<cv::Point> centerVector, std::vector<int> radiusVector) {
	std::vector<std::vector<int>> classifiedBalls;
	std::vector<int> whitePixelsInRoi, blackPixelsInRoi, blackList;
	int maxElWhite, maxElBlack;

	for (int i = 0; i < centerVector.size(); ++i) {
		cv::Mat whitePixels, blackPixels;
		cv::Mat roi = extractRoi2(image, centerVector[i], radiusVector[i]);
		
		cv::Mat hsvRoi;
		cv::cvtColor(roi, hsvRoi, cv::COLOR_BGR2HSV);
        //computeAndDisplayHistogram(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[0], UPPER_BOUNDS_CLASSIFICATION[0]);

		cv::inRange(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[0], UPPER_BOUNDS_CLASSIFICATION[0], whitePixels);
		cv::inRange(hsvRoi, LOWER_BOUNDS_CLASSIFICATION[1], UPPER_BOUNDS_CLASSIFICATION[1], blackPixels);
        cv::normalize(whitePixels, whitePixels, 0, 1, cv::NORM_MINMAX);
        cv::normalize(blackPixels, blackPixels, 0, 1, cv::NORM_MINMAX);
		whitePixelsInRoi.push_back(cv::sum(whitePixels)[0]);
		blackPixelsInRoi.push_back(cv::sum(blackPixels)[0]);

        //cv::imshow("White" + std::to_string(i), whitePixels);
        std::cout << "White n = " << cv::sum(whitePixels) << std::endl;
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