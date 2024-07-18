#include "SegmentationEvaluation.h"

float meanIoU(const cv::Mat& segmMask, const cv::Mat& groundTruth) {
	cv::Mat binaryPrediction, binaryGroundTruth, intersMat, unionMat;
	float result = 0;
	for (int i = 0; i < NUM_OF_CLASSES; ++i) {
		cv::inRange(segmMask, i, i, binaryPrediction);
		cv::inRange(groundTruth, i, i, binaryGroundTruth);

		cv::bitwise_and(binaryPrediction, binaryGroundTruth, intersMat);
		cv::bitwise_or(binaryPrediction, binaryGroundTruth, unionMat);

		result += cv::sum(intersMat)[0] / cv::sum(unionMat)[0];
	}
	return result / NUM_OF_CLASSES;
}