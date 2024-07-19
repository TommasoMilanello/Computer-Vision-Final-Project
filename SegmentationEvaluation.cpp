#include "SegmentationEvaluation.h"

float meanIoU(const cv::Mat& segmMask, const cv::Mat& groundTruth) {
	cv::Mat binaryPrediction, binaryGroundTruth, intersMat, unionMat;
	float result = 0;
	//std::cout << "Is 8-bit? " << ((groundTruth.type() == CV_8UC1) ? "Yes" : "No") << std::endl;
	for (int i = 0; i < NUM_OF_CLASSES; ++i) {
		cv::inRange(segmMask, cv::Scalar(i), cv::Scalar(i), binaryPrediction);
		cv::inRange(groundTruth, cv::Scalar(i), cv::Scalar(i), binaryGroundTruth);

		cv::bitwise_and(binaryPrediction, binaryGroundTruth, intersMat);
		cv::bitwise_or(binaryPrediction, binaryGroundTruth, unionMat);

		//test
		// cv::Mat thresh;
		// cv::threshold(groundTruth, thresh, 4, 255, cv::THRESH_BINARY);
		cv::imshow("pred", binaryPrediction);
        cv::imshow("truth", binaryGroundTruth);
		cv::waitKey(0);

		result += (cv::sum(intersMat)[0] / cv::sum(unionMat)[0]);
		std::cout << "Result: " << result << std::endl;
	}
	return result / NUM_OF_CLASSES;
}