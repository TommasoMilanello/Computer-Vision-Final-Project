#ifndef SEGMENTATIONEVALUATION_H
#define SEGMENTATIONEVALUATION_H

#include <opencv2/highgui.hpp>

const int NUM_OF_CLASSES = 6;

float meanIoU(const cv::Mat& segmMask, const cv::Mat& groundTruth);

#endif // SEGMENTATIONEVALUATION_H