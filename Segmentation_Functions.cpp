#include "Segmentation_Functions.h"
#include <iostream>
#include <map>


//idea: a pool table has a uniform color which is very similarly illuminated, the HSV space let's us focus only on the hue value and by taking a decent size kernel
//in the middle of the frame we sould manage to segment the table
cv::Mat segmentTable(const cv::Mat& frame)
{
	cv::Mat segmented = frame.clone();
	cv::Mat HSV;
	cv::cvtColor(frame, HSV, cv::COLOR_BGR2HSV);
	std::vector<cv::Vec3b> pixels;
	for (int i = HSV.rows / 2 - 20; i < HSV.rows / 2 + 20; i++) {
		for (int j = HSV.cols / 2 - 20; j < HSV.cols / 2 + 20; j++) {
			pixels.push_back(HSV.at<cv::Vec3b>(i, j));
		}
	}
	int hue = 0;
	int saturation = 0;
	int value = 0;
	for (auto pixel : pixels) {
		hue += static_cast<int>(pixel[0]);
		saturation += static_cast<int>(pixel[1]);
		value += static_cast<int>(pixel[2]);
	}
	hue /= pixels.size();
	saturation /= pixels.size();
	value /= pixels.size();

	// standard thresholds:
	//int t_h = 10;
	//int t_s = 70;
	//int t_v = 144;

	int t_h = 10;
	int t_s = 70;
	int t_v = 144;

	for (int i = 0; i < frame.rows; i++) {
		for (int j = 0; j < frame.cols; j++) {
			if ((std::abs(HSV.at<cv::Vec3b>(i, j)[0] - hue) <= t_h && std::abs(HSV.at<cv::Vec3b>(i, j)[1] - saturation) <= t_s && std::abs(HSV.at<cv::Vec3b>(i, j)[2] - value) <= t_v))
				segmented.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255);
			else
				segmented.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 0);
		}
	}
	return segmented;
}

void findHoughLines(cv::Mat& src)
{
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);

	//cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0,0);

    // Store the edges 
    cv::Mat edges;
    // Find the edges in the image using canny detector
    cv::Canny(gray, edges, 120, 240); //Canny(gray, edges, 60, 240);
    imshow("Edges", edges);
    // Create a vector to store lines of the image
    std::vector<cv::Vec3f> lines;
    // Apply Hough Transform
    int thresh = 0;
    cv::HoughLines(edges, lines, 2, CV_PI / 90, thresh);	//cv::HoughLines(edges, lines, 2, CV_PI / 90, thresh);
	sort(lines.begin(), lines.end(), [](const cv::Vec3f& a, const cv::Vec3f& b) {
		return a[2] > b[2];
		});
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
		line(src, pt1, pt2, cv::Scalar(0, 0, 255), 1, cv::LINE_AA);
	}
    /*for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec3f l = lines[i];
        cv::line(*src, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    }*/
}

std::vector<cv::Mat> multipleImRead(const std::string& path, const std::string& pattern)
{
	std::vector<cv::Mat> images;
	std::vector<cv::String> filenames;

	cv::utils::fs::glob(
		path, pattern, filenames
	);
	if (filenames.empty())
	{
		std::cout << "Image folder path not valid";
	}
	else
	{
		for (int i = 0; i < filenames.size(); i++)
		{
			images.push_back(cv::imread(filenames[i]));
		}
	}
	return images;
}
