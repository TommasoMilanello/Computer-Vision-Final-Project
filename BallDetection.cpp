#include "BallDetection.h"

std::vector<std::vector<int>> detectBalls(const cv::Mat& segmented, const cv::Mat& mask) {
	cv::Mat gray;
	cv::cvtColor(segmented, gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1.2, 10, 30, 20, 5, 13);

	std::vector<std::vector<int>> balls;
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
				balls.push_back({center.x, center.y, radius, ball_type});
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
    cv::Mat hsv_roi;
    cvtColor(roi, hsv_roi, cv::COLOR_BGR2HSV);
    
	// Reshape and sort to find the median color
    cv::Mat reshaped_hsv_roi = hsv_roi.reshape(1, hsv_roi.total());
    cv::Mat sorted_hsv_roi;
    cv::sort(reshaped_hsv_roi, sorted_hsv_roi, cv::SORT_EVERY_COLUMN + cv::SORT_ASCENDING);
    cv::Scalar median_color;
    median_color[0] = sorted_hsv_roi.at<cv::Vec3b>(sorted_hsv_roi.rows / 2, 0)[0];
    median_color[1] = sorted_hsv_roi.at<cv::Vec3b>(sorted_hsv_roi.rows / 2, 1)[1];
    median_color[2] = sorted_hsv_roi.at<cv::Vec3b>(sorted_hsv_roi.rows / 2, 2)[2];

    cv::Scalar white_lower(0, 0, 168);
    cv::Scalar white_upper(172, 111, 255);
    cv::Scalar black_lower(0, 0, 0);
    cv::Scalar black_upper(180, 255, 50);

    bool is_white = (median_color[0] >= white_lower[0] && median_color[0] <= white_upper[0]) &&
                    (median_color[1] >= white_lower[1] && median_color[1] <= white_upper[1]) &&
                    (median_color[2] >= white_lower[2] && median_color[2] <= white_upper[2]);

    bool is_black = (median_color[0] >= black_lower[0] && median_color[0] <= black_upper[0]) &&
                    (median_color[1] >= black_lower[1] && median_color[1] <= black_upper[1]) &&
                    (median_color[2] >= black_lower[2] && median_color[2] <= black_upper[2]);

    if (is_white) {
        return 1;  // Cue Ball: white
    }

    if (is_black) {
        return 2;  // 8-Ball: black
    }

    cv::Mat gray_roi;
    cvtColor(roi, gray_roi, cv::COLOR_BGR2GRAY);
	gray_roi.convertTo(gray_roi, CV_32F);
    cv::Mat f_transform;
    dft(gray_roi, f_transform, cv::DFT_COMPLEX_OUTPUT);

    cv::Mat planes[] = {cv::Mat::zeros(f_transform.size(), CV_32F), cv::Mat::zeros(f_transform.size(), CV_32F)};
    split(f_transform, planes);
    magnitude(planes[0], planes[1], planes[0]);

    cv::Mat magnitude_spectrum = planes[0];
    magnitude_spectrum += cv::Scalar::all(1);
    log(magnitude_spectrum, magnitude_spectrum);

    cv::Scalar mean_frequency = mean(magnitude_spectrum);

    if (mean_frequency[0] > 115) {
        return 3;  // Solid Balls: red
    } else {
        return 4;  // Striped Balls: green
    }
}