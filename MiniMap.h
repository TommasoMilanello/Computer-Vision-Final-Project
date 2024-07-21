//AUTHOR: Matteo Ruta

#ifndef MINIMAP_H
#define MINIMAP_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <iostream>

#include "BBox.h"

const std::string TABLE_SCHEME_PATH = "./map_background.png";
const float RESIZE_RATIO = 0.55;
const float RESIZE_PADDING = 0.01;
const int FIXED_RADIUS = 7;
const std::vector<cv::Point2f> MAIN_POINTS_WITH_IMAGE = {
	cv::Point(56, 48),
	cv::Point(634, 48),
	cv::Point(634, 351),
	cv::Point(56, 351),
	cv::Point(345, 200)
};

class MiniMap {
public:

	/**
	 * Default constructor for the MiniMap class.
	 * Initializes the background and table main points.
	 */
	MiniMap();

	/**
	 * Draws the mini-map on a given frame.
	 *
	 * @param frame The frame on which to draw the mini-map (cv::Mat).
	 */
	void drawMiniMapOnFrame(cv::Mat& frame) const;

	/**
	 * Computes the homography matrix for the mini-map using table corners and center.
	 *
	 * @param corners The corners of the table (vector<cv::Point>).
	 * @param center The center of the table (cv::Point).
	 * @param verbose The verbosity level for debug output (0, 1, 2).
	 */
	void computeHomography(const std::vector<cv::Point> corners, const cv::Point center, int verbose = 0);

	/**
	 * Initializes the mini-map with table corners, center, and bounding boxes.
	 *
	 * @param corners The corners of the table (vector<cv::Point>).
	 * @param center The center of the table (cv::Point).
	 * @param bboxes A vector of bounding boxes (BBox).
	 * @param approxRadius A flag to approximate the radius.
	 */
	void initMiniMap(const std::vector<cv::Point> corners, const cv::Point center, const std::vector<BBox> bboxes, bool approxRadius = false);

	/**
	 * Updates the mini-map with new bounding boxes.
	 *
	 * @param newBboxes A vector of new bounding boxes (BBox).
	 */
	void updateMiniMap(const std::vector<BBox> newBboxes);

	/**
	 * Projects the last frame's bounding box centers onto the mini-map image.
	 * Draws circles representing the objects' locations on the mini-map, using their category-based colors.
	 */
	void projectLastFrameOnMap();
	
private:
	cv::Mat Background;
	cv::Mat MapImg;
	std::vector<cv::Point2f> TableMainPoints;
	cv::Mat H;
	int radius;
	std::vector<std::vector<cv::Point>> ballCentersHistory;
	std::vector<int> ballCategID;
};

#endif // MINIMAP_H