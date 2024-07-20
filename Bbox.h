//AUTHOR: Matteo Ruta

#ifndef BBOX_H
#define BBOX_H

#include <opencv2/highgui.hpp>

class BBox {
public:
	static const std::vector<cv::Scalar> OBJECT_COLORS_BASED_ON_CATEG_ID;

	BBox(int x, int y, int width, int height, int categID);
	cv::Point getCenter() const;
	int getMaxRadius() const;
	int getCategID() const;
	cv::Rect asRect() const;
	std::vector<cv::Point> asPoints() const;
private:
	int X;
	int Y;
	int Width;
	int Height;
	int CategID;
};

#endif // BBox_H