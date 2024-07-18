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

	cv::Mat hsv_roi, gray;
	cv::cvtColor(roi, hsv_roi, cv::COLOR_BGR2HSV);

	// Compute the median color in HSV
	std::vector<cv::Mat> hsv_planes(3);
	cv::split(hsv_roi, hsv_planes);
	cv::Mat median_color;
	cv::medianBlur(hsv_planes[0], median_color, 3); // Use a kernel size of 3 for median blur
	cv::Scalar median = cv::mean(median_color);

	cv::Scalar white_lower(0, 0, 168);
	cv::Scalar white_upper(172, 111, 255);

	cv::Scalar black_lower(0, 0, 0);
	cv::Scalar black_upper(180, 255, 50);

	// Check if the median color falls within the white or black range
	if (cv::norm(median - white_lower) <= 255 && cv::norm(median - white_upper) <= 255) {
		return 1; // white
	}

	if (cv::norm(median - black_lower) <= 255 && cv::norm(median - black_upper) <= 255) {
		return 2; // black
	}

	// Convert to grayscale and apply threshold
	cv::cvtColor(roi, gray, cv::COLOR_BGR2GRAY);
	cv::Mat thresholded;
	cv::threshold(gray, thresholded, 130, 255, cv::THRESH_BINARY);

	int total_pixels = thresholded.total();
	int white_pixels = cv::countNonZero(thresholded);

	double white_proportion = static_cast<double>(white_pixels) / total_pixels;
	double proportion_threshold = 0.2;

	if (white_proportion > proportion_threshold) {
		return 4; // stripped
	}
	else {
		return 3; // solid
	}
}