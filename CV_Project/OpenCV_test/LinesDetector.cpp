#include "LinesDetector.h"

bool LinesDetector::detectLinesWithHoughTransform(cv::Mat src, cv::Mat& dst, std::vector<cv::Vec3f>& lines) {
	bool itWorked = false;

	if (src.type() != CV_8U) {
		std::cout << "The input image a grayscale image (8-bit, single channel)" << std::endl;
	}
	else {
		cv::Mat laplacian_output, canny_output, colored_output;
		cv::Mat tempSrc;
		//cv::namedWindow("Laplacian debug");
		//cv::namedWindow("Otsu debug");
		//cv::namedWindow("FindLines debug");
		//cv::namedWindow("ShowTable");
		cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);
		src.convertTo(tempSrc, CV_16S);

		//cv::Laplacian(src, laplacian_output, CV_16S);
		///*int newPixelValue;
		//for (int r = 0; r < src.rows; ++r) {
		//	for (int c = 0; c < src.cols; ++c) {
		//		newPixelValue = src.at<uchar>(r, c) + laplacian_output.at<signed char>(r, c);
		//		if (newPixelValue < 0) newPixelValue = 0;
		//		if (newPixelValue > 255) newPixelValue = 255;
		//		temp_src.at<uchar>(r, c) = newPixelValue;
		//	}
		//}*/
		//tempSrc += laplacian_output;
		//cv::convertScaleAbs(tempSrc, tempSrc);
		//cv::imshow("Laplacian debug", tempSrc);

		cv::Canny(src, canny_output, 70, 200);
		//cv::cvtColor(canny_output, dst, cv::COLOR_GRAY2BGR);
		//cv::imshow("FindLines debug", canny_output);

		cv::HoughLines(canny_output, lines, 2, CV_PI / 90, 0);

		sort(lines.begin(), lines.end(), [](const cv::Vec3f& a, const cv::Vec3f& b) {
			return a[2] > b[2];
			});
		lines.resize(4);

		// Draw lines on the image
		for (size_t i = 0; i < 4; i++) {
			float rho = lines[i][0], theta = lines[i][1];
			cv::Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a * rho, y0 = b * rho;
			pt1.x = cvRound(x0 + 1000 * (-b));
			pt1.y = cvRound(y0 + 1000 * (a));
			pt2.x = cvRound(x0 - 1000 * (-b));
			pt2.y = cvRound(y0 - 1000 * (a));
			line(dst, pt1, pt2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
		}

		//cv::imshow("FindLines debug", dst);
		//cv::imshow("ShowTable", dst);

		//cv::waitKey(0);
	}

	return itWorked;
}

bool LinesDetector::detectCirclesWithHoughTransform(cv::Mat src, cv::Mat dst) {
	bool itWorked = false;

	if (src.type() != CV_8U) {
		std::cout << "The input image a grayscale image (8-bit, single channel)" << std::endl;
	}
	else {
		cv::Mat canny_output;
		std::vector<cv::Vec3f> circles;
		cv::namedWindow("canny");
		dst = src.clone();
		cv::cvtColor(dst, dst, cv::COLOR_GRAY2BGR);

		cv::Canny(src, canny_output, 100, 50);
		cv::imshow("canny", canny_output);

		HoughCircles(src, circles, cv::HOUGH_GRADIENT, 2, src.rows / 10, 100, 70, 0, 100);

		for (size_t i = 0; i < circles.size(); i++)
		{
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			// draw the circle center
			circle(dst, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
			// draw the circle outline
			circle(dst, center, radius, cv::Scalar(255, 0, 0), 1, 8, 0);
		}
		cv::namedWindow("circles");
		cv::imshow("circles", dst);

		cv::waitKey(0);
	}

	return itWorked;
}

bool LinesDetector::detectAnglesWithHarris(cv::Mat src, cv::Mat dst) {
	bool itWorked = false;

	if (src.type() != CV_8U) {
		std::cout << "The input image a grayscale image (8-bit, single channel)" << std::endl;
	}
	else {
		cv::Mat floatSrc, corners;
		cv::namedWindow("corners");
		//cv::namedWindow("dst and cornres");
		src.convertTo(floatSrc, CV_32F);
		cv::cvtColor(src, dst, cv::COLOR_GRAY2BGR);

		cv::cornerHarris(floatSrc, corners, 2, 3, 0.2);

		double minVal, maxVal;
		cv::minMaxLoc(corners, &minVal, &maxVal);

		cv::threshold(corners, corners, maxVal*0.7, 255, cv::THRESH_BINARY);
		cv::imshow("corners", corners);

		cv::waitKey(0);
	}

	return itWorked;
}

bool LinesDetector::getRectFromLines(std::vector<cv::Vec3f> lines, std::vector<cv::Point>& vertices, int verbose) {

	/*std::vector<cv::Vec2f> params;

	for (int i = 0; i < lines.size(); ++i) {
		float rho = lines[i][0], theta = lines[i][1];
		float m = -cos(theta) / sin(theta), q = rho / sin(theta);
		params.push_back(cv::Vec2f(m, q));
	}*/

	//sort(params.begin(), params.end());
	std::vector<cv::Vec3f> nearSeed;
	std::vector<cv::Vec3f> oppositeSeed;

	//we choose as seed the line with min abs(rho)
	//in most cases, it should be the top line
	std::sort(lines.begin(), lines.end(), [](const cv::Vec3f& a, const cv::Vec3f& b) {
		return abs(a[0]) < abs(b[0]);
		});
	float seedPhase = lines[0][1];
	nearSeed.push_back(lines[0]);

	vertices.clear();
	/*sort(lines.begin(), lines.end(), [](const cv::Vec3f& a, const cv::Vec3f& b) {
		return abs(a[0]) > abs(b[0]);
		});*/

	// partition the lines based on their relative phases
	float minDist = CV_PI, rowDist;
	int chosenOne = 0;
	for (int i = 1; i < 4; ++i) {
		rowDist = abs(lines[i][1] - seedPhase);
		if (std::min((double)rowDist, CV_PI - rowDist) < minDist) {
			minDist = rowDist;
			chosenOne = i;
		}
	}
	nearSeed.push_back(lines[chosenOne]);
	for (int i = 0; i < 4; ++i) {
		if (i != 0 && i != chosenOne) {
			oppositeSeed.push_back(lines[i]);
		}
	}

	if (verbose > 0) {
		std::cout << "seed phase: " << seedPhase << std::endl;
		for (auto line : nearSeed) {
			std::cout << line << std::endl;
		}
		for (auto line : oppositeSeed) {
			std::cout << line << std::endl;
		}
	}

	float r1, r2, th1, th2, det;
	for (auto firstLine : nearSeed) {
		for (auto secondLine : oppositeSeed) {
			if (verbose > 1) {
				std::cout << "Intersection of lines: " << firstLine << " X " << secondLine << std::endl;
			}

			r1 = firstLine[0];
			th1 = firstLine[1];
			r2 = secondLine[0];
			th2 = secondLine[1];
			// security check -> it should not happen
			if (firstLine[1] != secondLine[1]) {
				// computing the intersection point
				det = sin(firstLine[1] - secondLine[1]);

				vertices.push_back(cv::Point(cvRound(((r2 * sin(th1)) - (r1 * sin(th2))) / det), cvRound(((r1 * cos(th2)) - (r2 * cos(th1))) / det)));
			}
			else {
				std::cout << "----ERROR!!!!!----\nDivision by 0 detected while computing intersections of lines!" << std::endl;
			}
		}
		std::reverse(oppositeSeed.begin(), oppositeSeed.end());
	}


	//sort(lines.begin(), lines.end(), [](const cv::Vec3f& a, const cv::Vec3f& b) {
	//	return abs(a[0]) > abs(b[0]);
	//	});
	//for (auto line : lines) {
	//	std::cout << line << std::endl;
	//}

	//// storing the most distant line in therms of rho
	//cv::Vec3f A = lines[0];
	//cv::Vec3f B = lines[lines.size() - 1];
	////std::cout << "Fino a qui, tutto liscio!" << std::endl;
	//cv::Vec3f secondLine;

	//float r1, r2, th1, th2, det;
	//for (auto firstLine : {A, B}) {
	//	for (int i = 1; i < 2; ++i) {
	//		secondLine = lines[i];

	//		r1 = firstLine[0];
	//		th1 = firstLine[1];
	//		r2 = secondLine[0];
	//		th2 = secondLine[1];
	//		// security check -> it should not happen
	//		if (firstLine[1] != secondLine[1]) {
	//			// computing the intersection point
	//			det = sin(firstLine[1] - secondLine[1]);

	//			vertices.push_back(cv::Point(cvRound(((r2 * sin(th1)) - (r1 * sin(th2))) / det), cvRound(((r1 * cos(th2)) - (r2 * cos(th1))) / det)));
	//		}
	//		else {
	//			std::cout << "----ERROR!!!!!----\nDivision by 0 detected while computing intersections of lines!" << std::endl;
	//		}
	//	}
	//}

	// WRONG
	//float r1, r2, th1, th2, det;
	//for (int firstLine = 0; firstLine < 2; ++firstLine) {
	//	for (int secondLine = 2; secondLine < 4; ++secondLine) {
	//		r1 = lines[firstLine][0];
	//		th1 = lines[firstLine][1];
	//		r2 = lines[secondLine - 2][0];
	//		th2 = lines[secondLine - 2][1];
	//		// security check -> it should not happen
	//		if (lines[firstLine][1] != lines[secondLine][1]) {
	//			// computing the intersection point
	//			det = sin(lines[firstLine][1] - lines[secondLine][1]);

	//			vertices.push_back(cv::Point(cvRound(((r2 * sin(th1)) - (r1 * sin(th2))) / det), cvRound(((r1 * cos(th2)) - (r2 * cos(th1))) / det)));
	//		}
	//		else {
	//			std::cout << "----ERROR!!!!!----\nDivision by 0 detected while computing intersections of lines!" << std::endl;
	//		}
	//	}
	//}

	if (verbose > 0) {
		std::cout << "vertices: " << vertices.size() << std::endl;
	}

	return false;
}