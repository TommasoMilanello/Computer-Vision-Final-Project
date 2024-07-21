//AUTHOR: Tommaso Milanello

#include "EvaluationMetrics.h"


float meanIoU(const cv::Mat& segmMask, const cv::Mat& groundTruth) {
	cv::Mat binaryPrediction, binaryGroundTruth, intersMat, unionMat;
	std::vector<float> partials(6);
	float result = 0;

	//get IoU for each class
	for (int i = 0; i < NUM_OF_CLASSES; ++i) {
		cv::inRange(segmMask, cv::Scalar(i), cv::Scalar(i), binaryPrediction);
		cv::inRange(groundTruth, cv::Scalar(i), cv::Scalar(i), binaryGroundTruth);

		cv::bitwise_and(binaryPrediction, binaryGroundTruth, intersMat);
		cv::bitwise_or(binaryPrediction, binaryGroundTruth, unionMat);

		partials[i] = (cv::sum(intersMat)[0] / cv::sum(unionMat)[0]);
		result += partials[i];
	}
	result /= NUM_OF_CLASSES;
	return result;
}

float mAP(std::vector<BBox> bBoxes, std::vector<BBox> groundTruth) {
	int currentClass;
	float  sumOfPrecisions, sumOfAveragePrecisions, precision, recall, totalGroundTruth;
	std::vector<std::vector<cv::Point2f>> recallPrecisionVectorPerClass(NUM_OF_CLASSES - 1);
	std::vector<float> APVector(NUM_OF_CLASSES - 1, 0);
	std::vector<int> TPvector(NUM_OF_CLASSES - 1, 0); 
	std::vector<int> FPVector(NUM_OF_CLASSES - 1, 0);

	totalGroundTruth = groundTruth.size();
	sumOfAveragePrecisions = 0;
	
	//updating the correct AP values w.r.t. each BBox class
	for (auto& bbox : bBoxes) {
		currentClass = bbox.getCategID();
		
		if(isTP(bbox, groundTruth)){
			TPvector[currentClass]++;
		}
		else {
			FPVector[currentClass]++;
		}
		recall =static_cast<float> (TPvector[currentClass] / totalGroundTruth);
		precision = static_cast<float> (TPvector[currentClass]) / (TPvector[currentClass] + FPVector[currentClass]);
		recallPrecisionVectorPerClass[currentClass].push_back(cv::Point2f(recall, precision));
	}

	//computation of exact AP values (using 11-interpolation)
	for (int i = 1; i < NUM_OF_CLASSES - 1; i++) {		
		sumOfPrecisions = 0;
		for(auto& precision : interpolate(recallPrecisionVectorPerClass[i])){
			sumOfPrecisions += precision;
		}
		sumOfAveragePrecisions += sumOfPrecisions / 11;
		}

	return sumOfAveragePrecisions / (NUM_OF_CLASSES - 2);
}

std::vector<float> interpolate(std::vector<cv::Point2f> recallPrecisionVector) {
	std::vector<float> result;
	float maxPrecision = 0;
	for(float recall = 0; recall <= 1; recall += 0.1) {
		maxPrecision = 0;
		for (auto& pair : recallPrecisionVector) {
			if(pair.x > recall && pair.y > maxPrecision) {
				maxPrecision = pair.y;
			}
		}
		result.push_back(maxPrecision);
	}

	return result;	
}

bool isTP(const BBox bbox, const std::vector<BBox> groundTruth) {
	float maxIntersection = 0;
	int maxIntersectionIndex = -1;

	for(int i = 0; i < groundTruth.size(); i++) {
		if((bbox.asRect() & groundTruth[i].asRect()).area() > maxIntersection) {
			maxIntersection = static_cast<float> ((bbox.asRect() & groundTruth[i].asRect()).area()); 
			maxIntersectionIndex = i;
		}
	}
	float unionArea = (bbox.asRect() | groundTruth[maxIntersectionIndex].asRect()).area();
	if(maxIntersectionIndex == -1)
		return false;
	return maxIntersection / unionArea >= 0.5;
}