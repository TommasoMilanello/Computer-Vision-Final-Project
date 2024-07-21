//AUTHOR: Ilyas Issa

#ifndef DISPLAYFUNTIONS_H
#define DISPLAYFUNCTIONS_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "BBox.h"
#include "MiniMap.h"

/**
 * The category identifier for the playing field.
 */
const int PLAYING_FIELD_CATEG = 5;

/**
 * Draws the segmentation on the destination image.
 *
 * @param src The source image.
 * @param dst The destination image where the segmentation will be drawn.
 * @param vertices The vertices of the playing field.
 * @param bboxes The bounding boxes of objects to be drawn.
 */
void drawSegmentation(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes);

/**
 * Draws the segmentation mask on the destination image.
 *
 * @param src The source image.
 * @param dst The destination image where the segmentation mask will be drawn.
 * @param segmented The segmented image.
 * @param bboxes The bounding boxes of objects to be drawn.
 */
void drawSegmentationMask(const cv::Mat& src, cv::Mat& dst, const cv::Mat& segmeted, const std::vector<BBox> bboxes);

/**
 * Draws ball localization on the destination image.
 *
 * @param src The source image.
 * @param dst The destination image where the ball localization will be drawn.
 * @param vertices The vertices of the playing field.
 * @param bboxes The bounding boxes of the balls to be drawn.
 * @param asBboxes Boolean flag indicating whether to draw balls as bounding boxes or circles.
 */
void drawBallLocalization(const cv::Mat& src, cv::Mat& dst, const std::vector<cv::Point> vertices, const std::vector<BBox> bboxes, bool asBboxes = true);

/**
 * Draws the frame with the minimap.
 *
 * @param src The source image.
 * @param dst The destination image where the frame with the minimap will be drawn.
 * @param bboxes The bounding boxes of objects to be drawn on the minimap.
 * @param map The MiniMap object containing the minimap information.
 */
void drawFrameWithMiniMap(const cv::Mat& src, cv::Mat& dst, const std::vector<BBox> bboxes, const MiniMap& map);

#endif // DISPLAYFUNCTIONS_H