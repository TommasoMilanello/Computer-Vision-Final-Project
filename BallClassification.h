//AUTHOR: Ilyas Issa

#ifndef BALLCLASSIFICATION_H
#define BALLCLASSIFICATION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

const std::vector<cv::Vec3b> LOWER_BOUNDS_CLASSIFICATION = {
	cv::Vec3b(0, 0, 153), //white 20, 10, 183
	cv::Vec3b(95, 224, 0) //black
};


const std::vector<cv::Vec3b> UPPER_BOUNDS_CLASSIFICATION = {
	cv::Vec3b(255, 102, 255), //white 90, 128, 255
	cv::Vec3b(125, 255, 38) //black
};

const int CLASSIFY_WHITE_THRESH = 101;
const int IGNORE_WHITE_THRESH = 26; // 28 seems good
const int CLASSIFY_BLACK_THRESH = 18;
const int IGNORE_BLACK_THRESH = 12;

/**
 * Classifies balls based on color analysis of regions of interest (ROIs) in an image.
 * 
 * @param image The source image containing the balls.
 * @param centerVector Vector of center points for each detected ball.
 * @param radiusVector Vector of radii for each detected ball.
 * @return A vector of vectors containing the classification results for each ball.
 */
std::vector<std::vector<int>> classifyBalls(const cv::Mat& image, std::vector<cv::Point> centerVector, std::vector<int> radiusVector);

/**
 * Extracts a region of interest (ROI) from the image centered around a given point with a specific radius.
 * 
 * @param image The source image from which the ROI is extracted.
 * @param center The center point of the ROI.
 * @param radius The radius of the circular ROI.
 * @return The masked ROI from the image.
 */
cv::Mat extractRoi2(const cv::Mat& image, cv::Point center, int radius);

#endif