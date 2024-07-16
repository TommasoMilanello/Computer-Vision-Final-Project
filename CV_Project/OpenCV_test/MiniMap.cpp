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
	this->MapImg.create(nrows, ncols, CV_8UC3);

	// fix corners of the projected table
	int displacmentRows = cvRound(nrows * PADDING / 2);
	int displacmentCols = cvRound(ncols * PADDING / 2);
	// the indications in the comments refers to an horizontal table, as in the minimap
	this->TableMainPoints.push_back(cv::Point2f(displacmentCols, displacmentRows));					// top-left
	this->TableMainPoints.push_back(cv::Point2f(ncols - displacmentCols, displacmentRows));			// top-right
	this->TableMainPoints.push_back(cv::Point2f(ncols - displacmentCols, nrows - displacmentRows));	// bottom-right
	this->TableMainPoints.push_back(cv::Point2f(displacmentCols, nrows - displacmentRows)); 		// bottom-left
	this->TableMainPoints.push_back(cv::Point2f(
		displacmentCols + cvRound((float)ncols * (1 - PADDING) / 2),
		displacmentRows + cvRound((float)nrows * (1 - PADDING) / 2)
	)); 		// center

	cv::rectangle(this->MapImg, this->TableMainPoints[0], this->TableMainPoints[2], cv::Scalar(0, 0, 0));
	cv::circle(this->MapImg, this->TableMainPoints[4], 10, cv::Scalar(0, 0, 0), 1);
	
	
	/*this->MapImg = cv::imread(TABLE_SCHEME_PATH);
	this->TableMainPoints = MiniMap::MAIN_POINTS_WITH_IMAGE;	*/
}

void MiniMap::drawMiniMap(std::string windowName) {
	cv::namedWindow(windowName);
	cv::imshow(windowName, this->MapImg);
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

void MiniMap::projectOnMap(std::vector<BBox> bboxes) {
	std::vector<cv::Point> ballCenters;
	std::vector<int> categId;
	cv::Mat pointAsMat, projectedPointAsMat, radiusAsMat, projectedRadiusAsMat;
	cv::Point projectedPoint;
	float radius, projectedRadius;

	// we approx the radius on the minimap picking the max avg raidus of the ball
	// then we apply it on an ipothetic ball in the center and then apply the homography to translate the dimension
	radius = avgMaxRadius(bboxes);
	radiusAsMat = (cv::Mat_<double>(3, 1) << radius + this->TableMainPoints[4].x,
		this->TableMainPoints[4].y,
		1);
	std::cout << "Point: " << radiusAsMat << std::endl;
	projectedRadiusAsMat = this->H * radiusAsMat;
	projectedRadiusAsMat /= projectedRadiusAsMat.at<double>(2);
	projectedRadius = static_cast<float>(projectedRadiusAsMat.at<double>(0, 0) - this->TableMainPoints[4].x);

	for (auto& bbox : bboxes) {
		ballCenters.push_back(bbox.getCenter());
		categId.push_back(bbox.getCategID());
	}

	for (int i = 0; i < ballCenters.size(); ++i) {
		pointAsMat = (cv::Mat_<double>(3, 1) << ballCenters[i].x,
												ballCenters[i].y,
												1);
		//std::cout << "Point size: " << pointAsMat.size() << std::endl;
		projectedPointAsMat = this->H * pointAsMat;
		projectedPointAsMat /= projectedPointAsMat.at<double>(2);
		projectedPoint = cv::Point(
			cvRound(projectedPointAsMat.at<double>(0, 0)),
			cvRound(projectedPointAsMat.at<double>(1, 0))
		);

		//std::cout << "Projected Point: " << projectedPoint << std::endl;
		cv::circle(this->MapImg, projectedPoint, projectedRadius, OBJECT_COLORS_BASED_ON_CATEG_ID[categId[i]], cv::FILLED);
		cv::circle(this->MapImg, projectedPoint, projectedRadius, cv::Scalar(0, 0, 0), 1);
	}
}

void MiniMap::projectRawImageOnMap(const cv::Mat src) {
	cv::warpPerspective(src, this->MapImg, this->H, this->MapImg.size());
}