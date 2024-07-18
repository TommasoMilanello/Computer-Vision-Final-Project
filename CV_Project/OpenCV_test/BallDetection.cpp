#include "BallDetection.h"

std::vector<std::tuple<cv::Point, int, int>> detectBalls(const cv::Mat& segmented, const cv::Mat& mask) {
	cv::Mat gray;
	cv::cvtColor(segmented, gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1.2, 10, 30, 20, 5, 13);

	std::vector<std::tuple<cv::Point, int, int>> balls;
	if (!circles.empty()) {
		for (const auto& circle : circles) {
			cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
			int radius = cvRound(circle[2]);
			float factor = 0.9f;
			cv::Rect roi_rect(center.x - static_cast<int>(factor * radius), center.y - static_cast<int>(factor * radius),
				2 * static_cast<int>(factor * radius), 2 * static_cast<int>(factor * radius));
			roi_rect &= cv::Rect(0, 0, segmented.cols, segmented.rows);
			cv::Mat ballRoi = extractRoi(segmented, center, radius);
			if (!ballRoi.empty()) {
				int ball_type = classifyBall(ballRoi);
				balls.emplace_back(center, radius, ball_type);
			}
		}
	}
	return balls;
}

cv::Mat extractRoi(const cv::Mat& image, cv::Point center, int radius) {
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

int classifyBall(const cv::Mat& roi) {
	//cv::Mat hsvRoi;
	//cv::cvtColor(roi, hsvRoi, cv::COLOR_BGR2HSV);
	//cv::Scalar medianColor = mean(hsvRoi);

	//cv::Scalar whiteLower(0, 0, 168);
	//cv::Scalar whiteUpper(172, 111, 255);

	//cv::Scalar blackLower(0, 0, 0);
	//cv::Scalar blackUpper(180, 255, 50);

	//if (whiteLower[0] <= medianColor[0] && medianColor[0] <= whiteUpper[0] &&
	//	whiteLower[1] <= medianColor[1] && medianColor[1] <= whiteUpper[1] &&
	//	whiteLower[2] <= medianColor[2] && medianColor[2] <= whiteUpper[2]) {
	//	return 1;
	//}

	//if (blackLower[0] <= medianColor[0] && medianColor[0] <= blackUpper[0] &&
	//	blackLower[1] <= medianColor[1] && medianColor[1] <= blackUpper[1] &&
	//	blackLower[2] <= medianColor[2] && medianColor[2] <= blackUpper[2]) {
	//	return 2;
	//}

	//cv::Mat grayRoi, grayRoiFormat;
	//cvtColor(roi, grayRoi, cv::COLOR_BGR2GRAY);

	////test
	//grayRoi.convertTo(grayRoi, CV_32FC1);

	//cv::Mat fTransform;
	//dft(grayRoi, fTransform, cv::DFT_COMPLEX_OUTPUT);
	//fTransform = fTransform(cv::Rect(0, 0, fTransform.cols & -2, fTransform.rows & -2));
	//cv::Mat planes[] = { cv::Mat::zeros(fTransform.size(), CV_32F), cv::Mat::zeros(fTransform.size(), CV_32F) };
	//split(fTransform, planes);
	//magnitude(planes[0], planes[1], planes[0]);
	//cv::Mat mag = planes[0];
	//mag += cv::Scalar::all(1);
	//cv::log(mag, mag);

	//mag = mag(cv::Rect(0, 0, mag.cols & -2, mag.rows & -2));

	//normalize(mag, mag, 0, 1, cv::NORM_MINMAX);

	//double meanFrequency = mean(mag)[0];
	//return meanFrequency > 115 ? 4 : 3;

	// Convert the ROI to HSV color space
	cv::Mat hsv_roi;
	cv::cvtColor(roi, hsv_roi, cv::COLOR_BGR2HSV);

	// Calculate the median color
	std::vector<cv::Mat> channels;
	cv::split(hsv_roi, channels);
	std::vector<int> histSize = { 256 };
	float range[] = { 0, 256 };
	const float* histRange = { range };
	cv::Mat hist;

	std::vector<float> median_color(3);
	for (int i = 0; i < 3; ++i) {
		cv::calcHist(&channels[i], 1, 0, cv::Mat(), hist, 1, histSize.data(), &histRange, true, false);
		int mid = roi.total() / 2;
		int count = 0;
		for (int j = 0; j < histSize[0]; ++j) {
			count += hist.at<float>(j);
			if (count > mid) {
				median_color[i] = j;
				break;
			}
		}
	}

	cv::Scalar white_lower(0, 0, 168);
	cv::Scalar white_upper(172, 111, 255);

	cv::Scalar black_lower(0, 0, 0);
	cv::Scalar black_upper(180, 255, 50);

	cv::Scalar median_color_scalar(median_color[0], median_color[1], median_color[2]);

	if ((white_lower <= median_color_scalar).all() && (median_color_scalar <= white_upper).all()) {
		return 0;
	}

	if ((black_lower <= median_color_scalar).all() && (median_color_scalar <= black_upper).all()) {
		return 1;
	}

	// Convert the ROI to grayscale
	cv::Mat gray_roi;
	cv::cvtColor(roi, gray_roi, cv::COLOR_BGR2GRAY);

	// Perform FFT
	cv::Mat padded;
	int m = cv::getOptimalDFTSize(gray_roi.rows);
	int n = cv::getOptimalDFTSize(gray_roi.cols);
	cv::copyMakeBorder(gray_roi, padded, 0, m - gray_roi.rows, 0, n - gray_roi.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));
	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);

	cv::dft(complexI, complexI);
	cv::split(complexI, planes);
	cv::magnitude(planes[0], planes[1], planes[0]);
	cv::Mat magI = planes[0];
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);

	magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

	cv::Mat magI_shifted;
	magI_shifted = magI(cv::Rect(magI.cols / 2, magI.rows / 2, magI.cols / 2, magI.rows / 2)).clone();
	cv::hconcat(magI(cv::Rect(0, magI.rows / 2, magI.cols / 2, magI.rows / 2)), magI_shifted, magI_shifted);
	cv::vconcat(magI(cv::Rect(magI.cols / 2, 0, magI.cols / 2, magI.rows / 2)), magI_shifted, magI_shifted);
	magI_shifted = magI_shifted(cv::Rect(0, 0, magI.cols / 2, magI.rows / 2)).clone();

	cv::normalize(magI_shifted, magI_shifted, 0, 1, cv::NORM_MINMAX);

	double mean_frequency = cv::mean(magI_shifted)[0];
	mean_frequency = 20 * std::log(mean_frequency);

	if (mean_frequency > 115) {
		return 4;
	}
	else {
		return 3;
	}
}