#include "MiniMap.h"

// utility function
float euclideanDistance(cv::Point p1, cv::Point p2) {
	float diffX = abs(p1.x - p2.x);
	float diffY = abs(p1.y - p2.y);
	return sqrt((diffX * diffX) + (diffY * diffY));
}

template<typename T> void shiftToBegin(std::vector<T>& vector, int index) {
	for (; index > 0; --index) {
		vector.push_back(vector[index - 1]);
		vector.erase(vector.begin() + index - 1);
	}
}

float avgMaxRadius(std::vector<BBox> bboxes) {
	float result = 0;
	for (auto& bbox : bboxes) {
		result += bbox.getMaxRadius();
	}
	return result / bboxes.size();
}

MiniMap::MiniMap(int nrows, int ncols) {
	//this->MapImg.create(nrows, ncols, CV_8UC3);

	//// fix corners of the projected table
	//int displacmentRows = cvRound(nrows * PADDING / 2);
	//int displacmentCols = cvRound(ncols * PADDING / 2);
	//// the indications in the comments refers to an horizontal table, as in the minimap
	//this->TableMainPoints.push_back(cv::Point2f(displacmentCols, displacmentRows));					// top-left
	//this->TableMainPoints.push_back(cv::Point2f(ncols - displacmentCols, displacmentRows));			// top-right
	//this->TableMainPoints.push_back(cv::Point2f(ncols - displacmentCols, nrows - displacmentRows));	// bottom-right
	//this->TableMainPoints.push_back(cv::Point2f(displacmentCols, nrows - displacmentRows)); 		// bottom-left
	//this->TableMainPoints.push_back(cv::Point2f(
	//	displacmentCols + cvRound((float)ncols * (1 - PADDING) / 2),
	//	displacmentRows + cvRound((float)nrows * (1 - PADDING) / 2)
	//)); 		// center

	//cv::rectangle(this->MapImg, this->TableMainPoints[0], this->TableMainPoints[2], cv::Scalar(0, 0, 0));
	//cv::circle(this->MapImg, this->TableMainPoints[4], 10, cv::Scalar(0, 0, 0), 1);


	this->MapImg = cv::imread(TABLE_SCHEME_PATH);
	this->TableMainPoints = MiniMap::MAIN_POINTS_WITH_IMAGE;
}

void MiniMap::drawMiniMapOnFrame(cv::Mat& frame) const {
	cv::Rect miniMapOnFrameShape;
	cv::Mat resizedMap;

	cv::resize(this->MapImg, resizedMap, cv::Size(), this->RESIZE_RATIO, this->RESIZE_RATIO);

	miniMapOnFrameShape = cv::Rect(
		cvRound(frame.cols * RESIZE_PADDING),
		cvRound((frame.rows * (1 - RESIZE_PADDING)) - resizedMap.rows),
		resizedMap.cols,
		resizedMap.rows
	);
	//std::cout << "Rect: " << miniMapOnFrameShape << std::endl;

	resizedMap.copyTo(frame(miniMapOnFrameShape));
}

void MiniMap::computeHomography(const std::vector<cv::Point> corners, const cv::Point center, int verbose) {
	std::vector<cv::Point> orderedCorners;

	// II method
	float minDistFromOrigin = INFINITY;
	int closestToOriginCornerIndex = 0;
	for (int i = 0; i < 4; ++i) {
		float distFromOrigin = sqrt((corners[i].x * corners[i].x) + (corners[i].y * corners[i].y));
		if (distFromOrigin < minDistFromOrigin) {
			minDistFromOrigin = distFromOrigin;
			closestToOriginCornerIndex = i;
		}
	}

	if (verbose > 1) {
		std::cout << "closestToOriginCornerIndex: " << closestToOriginCornerIndex << " - " << corners[closestToOriginCornerIndex] << std::endl;
	}

	// in the II method, we start working with orderedCorners immediately
	// => we exchange the positions of the corners later if needed
	for (int i = 0; i < 4; ++i) {
		orderedCorners.push_back(corners[(closestToOriginCornerIndex + i) % 4]);
	}

	// check the alignment
	std::vector<float> distCornersFromCenter;
	int farthesCornerFromCenterIndex;
	int farthestDist = 0;
	for (int i = 0; i < 4; ++i) {
		distCornersFromCenter.push_back(euclideanDistance(orderedCorners[i], center));
		if (distCornersFromCenter[i] > farthestDist) {
			farthestDist = distCornersFromCenter[i];
			farthesCornerFromCenterIndex = i;
		}
	}
	shiftToBegin(distCornersFromCenter, farthesCornerFromCenterIndex);
	std::vector<int> longEdgeExtremeIndexes = (distCornersFromCenter[1] <= distCornersFromCenter[3]) ? std::vector<int>{0, 1} : std::vector<int>{ 3, 0 };
	// mess
	bool isHorizontal = abs(
		orderedCorners[longEdgeExtremeIndexes[0]].x - orderedCorners[longEdgeExtremeIndexes[1]].x
	) > abs(
		orderedCorners[longEdgeExtremeIndexes[0]].y - orderedCorners[longEdgeExtremeIndexes[1]].y
	);

	if (verbose > 1) {
		std::cout << "Aligmnent: " << (isHorizontal ? "Horizontal" : "Vertical") << std::endl;
		std::cout << "Boundaries of the longest edge: " << orderedCorners[longEdgeExtremeIndexes[0]] << ", " << orderedCorners[longEdgeExtremeIndexes[1]] << std::endl;
		std::cout << "Distanceds from the center: " << std::endl;
		for (auto dist : distCornersFromCenter) {
			std::cout << dist << std::endl;
		}
	}

	// shifting the vector until the corner to be matched with the top-left one in the minimap is on the first position
	// only needed for a vertical rectangole and just once 
	if (!isHorizontal) {
		//this is the equivalent to do:
		//    A B C D --> B C D A
		//so, "moving the pointer forward of 1"

		shiftToBegin(orderedCorners, 1);
	}

	orderedCorners.push_back(center);
	//end II method

	if (verbose > 0) {
		std::cout << "Ordered corners:";
		for (auto point : orderedCorners) {
			std::cout << std::endl << point;
		}
		std::cout << " (center)" << std::endl;
	}

	this->H = cv::findHomography(orderedCorners, this->TableMainPoints);

	// debug for the corners
	cv::Mat pointAsMat, projectedPointAsMat;
	cv::Point projectedPoint;
	for (int i = 0; i < orderedCorners.size(); ++i) {
		pointAsMat = (cv::Mat_<double>(3, 1) << orderedCorners[i].x,
			orderedCorners[i].y,
			1);
		projectedPointAsMat = this->H * pointAsMat;
		projectedPointAsMat /= projectedPointAsMat.at<double>(2);
		projectedPoint = cv::Point(
			cvRound(projectedPointAsMat.at<double>(0, 0)),
			cvRound(projectedPointAsMat.at<double>(1, 0))
		);

		//std::cout << "Projected Point: " << projectedPoint << std::endl;
		cv::circle(this->MapImg, projectedPoint, 2, cv::Scalar(0, 128, 128), 2);
	}
}

void MiniMap::initMiniMap(const std::vector<cv::Point> corners, const cv::Point center, const std::vector<BBox> bboxes) {
	this->computeHomography(corners, center);

	this->ballCenters.clear();
	this->ballCategID.clear();
	for (auto& bbox : bboxes) {
		this->ballCenters.push_back(bbox.getCenter());
		this->ballCategID.push_back(bbox.getCategID());
	}

	this->projectOnMap(bboxes, center);
}

void MiniMap::updateMiniMap(const std::vector<BBox> newBboxes, const cv::Point center) {
	for (int i = 0; i < newBboxes.size(); ++i) {
		//TODO
	}
}

void MiniMap::projectOnMap(std::vector<BBox> bboxes, cv::Point center) {
	cv::Mat pointAsMat, projectedPointAsMat, radiusAsMat, projectedRadiusAsMat;
	cv::Point projectedPoint;
	float radius, projectedRadius;

	// we approx the radius on the minimap picking the max avg raidus of the ball
	// then we apply it on an ipothetic ball in the center and then apply the homography to translate the dimension
	radius = avgMaxRadius(bboxes);
	radiusAsMat = (cv::Mat_<double>(3, 1) << radius + center.x,
		center.y,
		1);
	projectedRadiusAsMat = this->H * radiusAsMat;
	projectedRadiusAsMat /= projectedRadiusAsMat.at<double>(2);
	projectedRadius = euclideanDistance(cv::Point(
		projectedRadiusAsMat.at<double>(0, 0),
		projectedRadiusAsMat.at<double>(1, 0)
	), this->TableMainPoints[4]);
	//std::cout << "Radius: " << projectedRadius << std::endl;

	for (int i = 0; i < ballCenters.size(); ++i) {
		pointAsMat = (cv::Mat_<double>(3, 1) << this->ballCenters[i].x,
			this->ballCenters[i].y,
			1);
		//std::cout << "Point size: " << pointAsMat.size() << std::endl;
		projectedPointAsMat = this->H * pointAsMat;
		projectedPointAsMat /= projectedPointAsMat.at<double>(2);
		projectedPoint = cv::Point(
			cvRound(projectedPointAsMat.at<double>(0, 0)),
			cvRound(projectedPointAsMat.at<double>(1, 0))
		);

		//std::cout << "Projected Point: " << projectedPoint << std::endl;
		cv::circle(this->MapImg, projectedPoint, cvRound(projectedRadius), OBJECT_COLORS_BASED_ON_CATEG_ID[this->ballCategID[i]], cv::FILLED);
		cv::circle(this->MapImg, projectedPoint, cvRound(projectedRadius), cv::Scalar(0, 0, 0), 1);
	}
}