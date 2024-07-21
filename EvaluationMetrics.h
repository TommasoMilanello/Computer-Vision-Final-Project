//AUTHOR: Tommaso Milanello

#ifndef SEGMENTATIONEVALUATION_H
#define SEGMENTATIONEVALUATION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "BBox.h"

const int NUM_OF_CLASSES = 6;

/**
 * Computes the mean Intersection over Union (IoU) between a segmentation mask and ground truth.
 *
 * @param segmMask The segmentation mask (cv::Mat).
 * @param groundTruth The ground truth mask (cv::Mat).
 * @return The mean IoU value as a float.
 */
float meanIoU(const cv::Mat& segmMask, const cv::Mat& groundTruth);

/**
 * Computes the mean Average Precision (mAP) for a set of bounding boxes.
 *
 * @param bBoxes A vector of predicted BBox objects.
 * @param groundTruth A vector of ground truth BBox objects.
 * @return The mean Average Precision as a float.
 */
float mAP(std::vector<BBox> bBoxes, std::vector<BBox> groundTruth);

/**
 * Interpolates the precision values at different recall levels for a given class.
 *
 * @param recallPrecisionVector A vector of cv::Point2f where x is recall and y is precision.
 * @return A vector of interpolated precision values as floats.
 */
std::vector<float> interpolate(std::vector<cv::Point2f> recallPrecisionVector);

/**
 * Determines if a bounding box is a true positive based on the Intersection over Union (IoU) with ground truth boxes.
 *
 * @param bbox The predicted bounding box (BBox).
 * @param groundTruth A vector of ground truth bounding boxes (BBox).
 * @return True if the bounding box is a true positive, false otherwise.
 */
bool isTP(const BBox bbox, const std::vector<BBox> groundTruth);

#endif // SEGMENTATIONEVALUATION_H