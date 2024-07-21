//AUTHOR: Tommaso Milanello

#ifndef MAINUTILITIES_H
#define MAINUTILITIES_H

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/filesystem.hpp>
#include <iostream>
#include <fstream>

#include "BBox.h"

/**
 * Reads multiple images from the specified directory and pattern.
 *
 * @param path The directory path where the images are located.
 * @param pattern The pattern to match the filenames of the images.
 * @param asGray Boolean flag indicating whether to read images as grayscale.
 * @return A vector of cv::Mat objects containing the read images.
 */
std::vector<cv::Mat> multipleImRead(const std::string& path, const std::string& pattern, bool asGray);


/**
 * Reads multiple bounding box (BBox) files from the specified directory and pattern.
 *
 * @param path The directory path where the BBox files are located.
 * @param pattern The pattern to match the filenames of the BBox files.
 * @return A vector of vectors of BBox objects containing the read bounding boxes.
 */
std::vector<std::vector<BBox>> multipleBBoxRead(const std::string& path, const std::string& pattern);

/**
 * Computes the center of a table shape given its vertices.
 *
 * @param vertices A vector of cv::Point representing the vertices of the table shape.
 * @return The center point of the table shape.
 */
cv::Point computeCenterOfTableShape(std::vector<cv::Point> vertices);

/**
 * Converts a vector of vectors of integers into a vector of BBox objects.
 *
 * @param toBboxes A vector of vectors of integers where each inner vector represents [x, y, size, categoryID].
 * @return A vector of BBox objects created from the input vectors.
 */
std::vector<BBox> convertIntoBBoxes(std::vector<std::vector<int>> toBboxes);

#endif // MAINUTILITIES_H