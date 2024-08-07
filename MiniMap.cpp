//AUTHOR: Matteo Ruta

#include "MiniMap.h"

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

cv::Point projectPoint(cv::Point point, cv::Mat homographyMatrix) {
	cv::Mat pointAsMat, projectedPointAsMat;

	pointAsMat = (cv::Mat_<double>(3, 1) << point.x,
			point.y,
			1);
	projectedPointAsMat = homographyMatrix * pointAsMat;
	projectedPointAsMat /= projectedPointAsMat.at<double>(2);

	return cv::Point(
			cvRound(projectedPointAsMat.at<double>(0, 0)),
			cvRound(projectedPointAsMat.at<double>(1, 0))
		);
}

MiniMap::MiniMap() {
	this->Background = cv::imread(TABLE_SCHEME_PATH);
	this->TableMainPoints = MAIN_POINTS_WITH_IMAGE;
}

void MiniMap::drawMiniMapOnFrame(cv::Mat& frame) const {
	cv::Rect miniMapOnFrameShape;
	cv::Mat resizedMap;

	cv::resize(this->MapImg, resizedMap, cv::Size(), RESIZE_RATIO, RESIZE_RATIO);

	miniMapOnFrameShape = cv::Rect(
		cvRound(frame.cols * RESIZE_PADDING),
		cvRound((frame.rows * (1 - RESIZE_PADDING)) - resizedMap.rows),
		resizedMap.cols,
		resizedMap.rows
	);

	resizedMap.copyTo(frame(miniMapOnFrameShape));
}

void MiniMap::computeHomography(const std::vector<cv::Point> corners, const cv::Point center, int verbose) {
	std::vector<cv::Point> orderedCorners;

	//pick closest corner to the origin
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

	//shifting the corners s.t. the first on the vector is the closest to the origin (preserving the ordering)
	for (int i = 0; i < 4; ++i) {
		orderedCorners.push_back(corners[(closestToOriginCornerIndex + i) % 4]);
	}

	// check the alignment
	std::vector<float> distCornersFromCenter;
	int farthestCornerFromCenterIndex;
	int farthestDist = 0;
	for (int i = 0; i < 4; ++i) {
		distCornersFromCenter.push_back(euclideanDistance(orderedCorners[i], center));
		if (distCornersFromCenter[i] > farthestDist) {
			farthestDist = distCornersFromCenter[i];
			farthestCornerFromCenterIndex = i;
		}
	}

	//shifting the corners s.t. the first on the vector is the farthest from the center of the table (preserving the ordering)
	//see report for details
	shiftToBegin(distCornersFromCenter, farthestCornerFromCenterIndex);
	std::vector<int> longEdgeExtremeIndexes = (distCornersFromCenter[1] <= distCornersFromCenter[3]) ? std::vector<int>{0, 1} : std::vector<int>{ 3, 0 };
	bool isHorizontal = abs(
		orderedCorners[longEdgeExtremeIndexes[0]].x - orderedCorners[longEdgeExtremeIndexes[1]].x
	) > abs(
		orderedCorners[longEdgeExtremeIndexes[0]].y - orderedCorners[longEdgeExtremeIndexes[1]].y
	);

	//output debug
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

	if (verbose > 0) {
		std::cout << "Ordered corners:";
		for (auto point : orderedCorners) {
			std::cout << std::endl << point;
		}
		std::cout << " (center)" << std::endl;
	}

	//homography matrix computation
	this->H = cv::findHomography(orderedCorners, this->TableMainPoints);
}

void MiniMap::initMiniMap(const std::vector<cv::Point> corners, const cv::Point center, const std::vector<BBox> bboxes, bool approxRadius) {
	this->computeHomography(corners, center, 0);

	this->MapImg = this->Background.clone();
	this->radius = FIXED_RADIUS;
	float estimatedRadius;
	cv::Mat radiusAsMat, projectedRadiusAsMat;

	for (auto& bbox : bboxes) {
		this->ballCentersHistory.push_back(std::vector<cv::Point>{projectPoint(bbox.getCenter(), this->H)});
		this->ballCategID.push_back(bbox.getCategID());
	}

	// we approx the radius on the minimap picking the max avg raidus of the ball is approxRadius = true
	// then we apply it on an ipothetic ball in the center and then apply the homography to translate the dimension
	if (approxRadius) {
		estimatedRadius = avgMaxRadius(bboxes);
		radiusAsMat = (cv::Mat_<double>(3, 1) << estimatedRadius + center.x,
			center.y,
			1);
		projectedRadiusAsMat = this->H * radiusAsMat;
		projectedRadiusAsMat /= projectedRadiusAsMat.at<double>(2);
		this->radius = euclideanDistance(cv::Point(
			projectedRadiusAsMat.at<double>(0, 0),
			projectedRadiusAsMat.at<double>(1, 0)
		), this->TableMainPoints[4]);
	}

	this->projectLastFrameOnMap();
}

void MiniMap::updateMiniMap(const std::vector<BBox> newBboxes) {
	this->MapImg = this->Background.clone();

	//update balls history
	for (int i = 0; i < newBboxes.size(); ++i) {
		cv::Point projectedCenter = projectPoint(newBboxes[i].getCenter(), this->H);
		if (this->ballCentersHistory[i].back() != projectedCenter) {
			this->ballCentersHistory[i].push_back(projectedCenter);
		}
	}

	for (int i = 0; i < this->ballCentersHistory.size(); ++i) {
		std::vector<cv::Point> ballHistory = this->ballCentersHistory[i];
		if (ballHistory.size() > 1) {
			for (int j = 0; j < ballHistory.size() - 1; ++j) {
				cv::line(this->MapImg, ballHistory[j], ballHistory[j + 1], cv::Scalar(0, 0, 0));
			}
		}
	}

	this->projectLastFrameOnMap();
}

void MiniMap::projectLastFrameOnMap() {
	for (int i = 0; i < this->ballCentersHistory.size(); ++i) {
		cv::circle(this->MapImg, this->ballCentersHistory[i].back(), cvRound(this->radius), BBox::OBJECT_COLORS_BASED_ON_CATEG_ID[this->ballCategID[i]], cv::FILLED);
		cv::circle(this->MapImg, this->ballCentersHistory[i].back(), cvRound(this->radius), cv::Scalar(0, 0, 0), 1);
	}
}
