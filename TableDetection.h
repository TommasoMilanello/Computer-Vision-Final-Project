//AUTHOR: Matteo Ruta

#ifndef TABLEDETECTION_H
#define TABLEDETECTION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

/**
 * Segments an image using a region-growing algorithm.
 *
 * @param src The source image in BGR format.
 * @param dst The destination image where the segmented result is stored.
 * @param ksize The size of the kernel used for segmentation.
 */
void segmentRegionGrowing(cv::Mat src, cv::Mat& dst, int ksize = 21);

/**
 * Detects lines in an image using the Hough Transform.
 *
 * @param src The source grayscale image (8-bit, single channel).
 * @param dst The destination image where the detected lines are drawn.
 * @param lines A vector to store the detected lines.
 */
void detectLinesWithHoughTransform(cv::Mat src, cv::Mat& dst, std::vector<cv::Vec3f>& lines);

/**
 * Computes the vertices of a rectangle from detected lines.
 *
 * @param lines The detected lines from which the rectangle vertices are computed.
 * @param vertices The resulting vertices of the rectangle.
 * @param verbose The verbosity level for debugging output.
 */
void getRectFromLines(std::vector<cv::Vec3f> lines, std::vector<cv::Point>& vertices, int verbose = 0);

#endif // TABLEDETECTION_H