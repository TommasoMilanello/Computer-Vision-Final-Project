#include "TableDetection.h"

void segmentRegionGrowing(cv::Mat src, cv::Mat& dst, int ksize) {

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
}

void detectLinesWithHoughTransform(cv::Mat src, cv::Mat& dst, std::vector<cv::Vec3f>& lines) {
	bool itWorked = false;

	if (src.type() != CV_8U) {
		std::cout << "The input image should be a grayscale image (8-bit, single channel)" << std::endl;
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
}

void getRectFromLines(std::vector<cv::Vec3f> lines, std::vector<cv::Point>& vertices, int verbose) {

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
}