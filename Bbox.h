//AUTHOR: Matteo Ruta

#ifndef BBOX_H
#define BBOX_H

#include <opencv2/highgui.hpp>

class BBox {
public:
	/**
	 * Predefined colors for bounding box categories.
	 */
	static const std::vector<cv::Scalar> OBJECT_COLORS_BASED_ON_CATEG_ID;

	/**
	 * Constructor for the BBox class.
	 *
	 * @param x The x-coordinate of the bounding box.
	 * @param y The y-coordinate of the bounding box.
	 * @param width The width of the bounding box.
	 * @param height The height of the bounding box.
	 * @param categID The category ID of the object within the bounding box.
	 */
	BBox(int x, int y, int width, int height, int categID);

	/**
	 * Gets the center point of the bounding box.
	 *
	 * @return The center point of the bounding box.
	 */
	cv::Point getCenter() const;

	/**
	 * Gets the maximum radius of the bounding box.
	 *
	 * @return The maximum radius of the bounding box.
	 */
	int getMaxRadius() const;

	/**
	 * Gets the category ID of the bounding box.
	 *
	 * @return The category ID of the bounding box.
	 */
	int getCategID() const;

	/**
	 * Converts the bounding box to a cv::Rect object.
	 *
	 * @return The bounding box as a cv::Rect object.
	 */
	cv::Rect asRect() const;

	/**
	 * Converts the bounding box to a vector of points representing its corners.
	 *
	 * @return A vector of points representing the corners of the bounding box.
	 */
	std::vector<cv::Point> asPoints() const;
	
private:
	int X;
	int Y;
	int Width;
	int Height;
	int CategID;
};

#endif // BBox_H