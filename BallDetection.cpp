//AUTHOR: Ilyas Issa

#include "BallDetection.h"

std::vector<std::vector<int>> detectBalls(const cv::Mat& segmented, const cv::Mat& mask) {
	cv::Mat gray;
	cv::cvtColor(segmented, gray, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);

    //find circles with circular Hough transform
	std::vector<cv::Vec3f> circles;
	cv::HoughCircles(gray, circles, cv::HOUGH_GRADIENT, 1.2, 10, 30, 20, 5, 13);

    std::vector<cv::Point> centerVector;    
    std::vector<int> radiusVector;          
	if (!circles.empty()) {
		for (const auto& circle : circles) {
			cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
			int radius = cvRound(circle[2]);
			float factor = 0.9f;
			cv::Rect roi_rect(center.x - static_cast<int>(factor * radius), center.y - static_cast<int>(factor * radius),
				2 * static_cast<int>(factor * radius), 2 * static_cast<int>(factor * radius));
			roi_rect &= cv::Rect(0, 0, segmented.cols, segmented.rows);

            centerVector.push_back(center); 
            radiusVector.push_back(radius); 
		}
	}
    return classifyBalls(segmented, centerVector, radiusVector); 
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