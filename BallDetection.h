//AUTHOR: Ilyas Issa

#ifndef BALLDETECTION_H
#define BALLDETECTION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "BBox.h"
#include "BallClassification.h"

/**
 * Detects balls in the segmented image using Hough Circle Transform and classifies them.
 * 
 * @param segmented The segmented image containing potential balls.
 * @param mask The mask used for detecting balls.
 * @return A vector of vectors containing classified ball information.
 */
std::vector<std::vector<int>> detectBalls(const cv::Mat& segmented, const cv::Mat& mask);

/**
 * Extracts a region of interest (ROI) from the image centered around a given point with a specific radius.
 * 
 * @param image The source image from which the ROI is extracted.
 * @param center The center point of the ROI.
 * @param radius The radius of the circular ROI.
 * @return The masked ROI from the image.
 */
cv::Mat extractRoi(const cv::Mat& image, cv::Point center, int radius);

#endif // BALLDETECTION_H