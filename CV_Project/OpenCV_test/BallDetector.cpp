#include "BallDetector.h"

void BallDetector::detectWhiteBall(const cv::Mat& src, cv::Mat& dst, const cv::Mat& tableArea) {
	//// TODO constant
	//cv::Scalar lowerBoundColor(20, 0, 153);
	//cv::Scalar upperBoundColor(90, 128, 255); // S: 102
	//cv::Mat srcHsv, segmented, preCanny = cv::Mat::zeros(src.rows, src.cols, CV_8UC1), postCanny;
	//std::vector<cv::Vec4f> circles;
	//cv::cvtColor(src, srcHsv, cv::COLOR_BGR2HSV);

	//cv::inRange(srcHsv, lowerBoundColor, upperBoundColor, segmented);
	//segmented.copyTo(preCanny, tableArea);

	////cv::Canny(preCanny, postCanny, 100, 200);

	//cv::HoughCircles(preCanny, circles, cv::HOUGH_GRADIENT, 2, 10, 200, 0.5, 5, 20);

	//if (!circles.empty()) {
	//	sort(circles.begin(), circles.end(), [](const cv::Vec4f& a, const cv::Vec4f& b) {
	//		return a[3] > b[3];
	//		});
	//	cv::cvtColor(preCanny, dst, cv::COLOR_GRAY2BGR);
	//	cv::circle(dst, cv::Point(circles[0][0], circles[0][1]), circles[0][2], cv::Scalar(0, 0, 255), 1);
	//}
	//else {
	//	std::cout << "No circles!!!" << std::endl;
	//}

	std::vector<cv::Vec4f> circles;
	cv::Mat srcGray, preCanny = cv::Mat::zeros(src.rows, src.cols, CV_8UC1), postCanny;
	cv::Mat laplacian(src.rows, src.cols, CV_16S), srcGraySigned(src.rows, src.cols, CV_16S);

	cv::cvtColor(src, srcGray, cv::COLOR_BGR2GRAY);

	//cv::GaussianBlur(srcGray, srcGray, cv::Size(7, 7), 1.5, 1.5);

	srcGray.copyTo(preCanny, tableArea);

	//cv::Canny(preCanny, postCanny, 100, 200);

	cv::HoughCircles(preCanny, circles, cv::HOUGH_GRADIENT_ALT, 1.5, 5, 250, 0.75, 5, 14);

	if (!circles.empty()) {
		sort(circles.begin(), circles.end(), [](const cv::Vec4f& a, const cv::Vec4f& b) {
			return a[3] > b[3];
			});
		cv::cvtColor(srcGray, dst, cv::COLOR_GRAY2BGR);
		for (int i = 0; i < circles.size(); ++i) {
			if (i < 16) {
				cv::circle(dst, cv::Point(circles[i][0], circles[i][1]), circles[i][2], cv::Scalar(0, 0, 255), 1);
			}
		}
	}
	else {
		std::cout << "No circles!!!" << std::endl;
	}
}

void BallDetector::templateMatching(const cv::Mat& src, cv::Mat& dst, cv::Mat& corr, const cv::Mat& templ, const cv::Mat& mask) {
	cv::Mat maskResized, corrNorm, labels, stats, centroids;
	double max, ratio = 0.80;
	cv::Point maxPoint, chosenPoint;

	cv::matchTemplate(src, templ, corr, cv::TM_CCOEFF_NORMED);

	cv::resize(mask, maskResized, corr.size());

	for (int col = 0; col < corr.cols; ++col) {
		for (int row = 0; row < corr.rows; ++row) {
			if (mask.at<uchar>(row, col) == 0) {
				corr.at<float>(row, col) = 0;
			}
		}
	}

	cv::erode(corr, corr, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));

	// the mask is not specified in the method since we alread turned off all the points out from the table
	cv::minMaxLoc(corr, NULL, &max, NULL, &maxPoint);

	cv::threshold(corr, corr, max * ratio, max, cv::THRESH_BINARY);

	cv::normalize(corr, corrNorm, 0, 255, cv::NORM_MINMAX, CV_8U);

	cv::connectedComponentsWithStats(corrNorm, labels, stats, centroids, 8);

	dst = src.clone();

	int maxArea = 0, maxAreaLabel = 1;
	for (int label = 1; label < stats.rows; ++label) {
		//std::cout << "Area: " << stats.at<int>(label, cv::CC_STAT_AREA) << std::endl;
		if (stats.at<int>(label, cv::CC_STAT_AREA) > maxArea) {
			maxArea = stats.at<int>(label, cv::CC_STAT_AREA);
			maxAreaLabel = label;
			std::cout << "MaxArea now: " << stats.at<int>(label, cv::CC_STAT_AREA) << ", label: " << label << std::endl;
		}
	}

	chosenPoint = cv::Point(centroids.at<double>(maxAreaLabel, 0), centroids.at<double>(maxAreaLabel, 1));

	cv::rectangle(dst, chosenPoint, chosenPoint + cv::Point(templ.cols, templ.rows), cv::Scalar(0, 255, 0));
}

void BallDetector::findBallSeeds(const cv::Mat& src, cv::Mat& dst, std::vector<Seed> seeds, cv::Mat& ballsSegm, const std::vector<cv::Point>& contour) {
	cv::Mat labels, stats, centroids, mask = cv::Mat::zeros(src.rows, src.cols, CV_8U);
	std::vector<int> validSizeLabels, validBoundLabels;
	ballsSegm = cv::Mat::zeros(src.rows, src.cols, CV_8U);
	double max;

	//inverting the segmented image
	cv::drawContours(mask, std::vector<std::vector<cv::Point>>{contour}, -1, 255, cv::FILLED);
	cv::bitwise_not(src, ballsSegm, mask);

	cv::connectedComponentsWithStats(ballsSegm, labels, stats, centroids);
	//std::cout << "Labels: " << stats.rows << std::endl;

	// turning off very small and very big regions
	for (int label = 1; label < stats.rows; ++label) {
		if (stats.at<int>(label, cv::CC_STAT_AREA) >= 51 && stats.at<int>(label, cv::CC_STAT_AREA) < 1000) { //area of a circle of 4 pixels && 30 / pi pixels
			validSizeLabels.push_back(label);
		}
	}

	// turning off regions that goes out from the mask
	for (int labelIndex = 0; labelIndex < validSizeLabels.size(); ++labelIndex) {
		cv::Rect region = cv::Rect(
			stats.at<int>(validSizeLabels[labelIndex], cv::CC_STAT_LEFT),
			stats.at<int>(validSizeLabels[labelIndex], cv::CC_STAT_TOP),
			stats.at<int>(validSizeLabels[labelIndex], cv::CC_STAT_WIDTH),
			stats.at<int>(validSizeLabels[labelIndex], cv::CC_STAT_HEIGHT)
		);

		bool outOfMask = false;
		for (int i = 0; i < 2 && !outOfMask; ++i) {
			for (int j = 0; j < 2 && !outOfMask; ++j) {
				int fromMask = cv::pointPolygonTest(contour, cv::Point(region.x + (i * region.width), region.y + (j * region.height)), false);
				outOfMask = fromMask < 0;
			}
		}

		if (!outOfMask) {
			validBoundLabels.push_back(validSizeLabels[labelIndex]);
		}
	}

	dst = cv::Mat::zeros(src.rows, src.cols, CV_8UC3);

	int x, y, left, top, width, height;
	double radius;
	for (auto& label : validBoundLabels) {
		x = cvRound(centroids.at<double>(label, 0));
		y = cvRound(centroids.at<double>(label, 1));
		left = stats.at<int>(label, cv::CC_STAT_LEFT);
		top = stats.at<int>(label, cv::CC_STAT_TOP);
		width = stats.at<int>(label, cv::CC_STAT_WIDTH);
		height = stats.at<int>(label, cv::CC_STAT_HEIGHT);
		radius = sqrt(static_cast<float>(stats.at<int>(label, cv::CC_STAT_AREA)) / CV_PI);
		//std::cout << "Radius: " << radius << std::endl;

		seeds.push_back(Seed(x, y, left, top, width, height, stats.at<int>(label, cv::CC_STAT_AREA)));
		//cv::circle(dst, cv::Point(x, y), cvRound(radius), cv::Scalar(0, 255, 0));
		cv::rectangle(dst, cv::Rect(left, top, width, height), cv::Scalar(0, 255, 0));
		dst.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 255, 0);
	}
}

void BallDetector::localizeBallsFromSeeds(const cv::Mat& src, cv::Mat& dst, std::vector<Seed> seeds, std::vector<BBox> bboxes) {
	// ASSUMPTION: radius of the ball between 7 and 18

	const int MIN_AREA = cvRound(7.0 * 7.0 * CV_PI); //useful now?
	const int MAX_AREA = cvRound(18.0 * 18.0 * CV_PI);

	for (int i = 0; i < seeds.size(); ++i) {
		if (seeds[i].AreaConnComp > MAX_AREA) {
			seeds.push_back(seeds[i].split());
		}
	}

	dst = src.clone();

	for (auto& seed : seeds) {
		cv::rectangle(dst, cv::Rect(seed.Left, seed.Top, seed.Width, seed.Height), cv::Scalar(0, 255, 0));
	}
}