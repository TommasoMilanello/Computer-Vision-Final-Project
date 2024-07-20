//AUTHOR: Tommaso Milanello

#ifndef SEGMENTATIONEVALUATION_H
#define SEGMENTATIONEVALUATION_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>

#include "BBox.h"

const int NUM_OF_CLASSES = 6;

float meanIoU(const cv::Mat& segmMask, const cv::Mat& groundTruth);

/**
 * Calculates the mean Average Precision of the bounding boxes.
 * @returns Mean Average Precision
 * @param bBoxes bounding boxes found by the algorithm
 * @param groundTruth ground truth bounding boxes
*/
float mAP(std::vector<BBox> bBoxes, std::vector<BBox> groundTruth);

std::vector<float> interpolate(std::vector<cv::Point2f> recallPrecisionVector);

bool isTP(const BBox bbox, const std::vector<BBox> groundTruth);

#endif // SEGMENTATIONEVALUATION_H