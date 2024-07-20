//AUTHOR: Matteo Ruta

#include "BBox.h"

const std::vector<cv::Scalar> BBox::OBJECT_COLORS_BASED_ON_CATEG_ID = {
	cv::Scalar(128, 128, 128),
	cv::Scalar(255, 255, 255),
	cv::Scalar(0, 0, 0),
	cv::Scalar(0, 0, 255),
	cv::Scalar(255, 0, 0),
	cv::Scalar(0, 255, 0)
};

BBox::BBox(int x, int y, int width, int height, int categID) {
	this->X = x;
	this->Y = y;
	this->Width = width;
	this->Height = height;
	this->CategID = categID;
}

cv::Point BBox::getCenter() const {
	return cv::Point(this->X + cvRound((float)this->Width / 2), this->Y + cvRound((float)this->Height / 2));
}

int BBox::getMaxRadius() const {
	bool horizontal = this->X > this->Y;
	if (horizontal) {
		return this->getCenter().x - this->X;
	}
	else {
		return this->getCenter().y - this->Y;
	}
}

int BBox::getCategID() const {
	return this->CategID;
}

cv::Rect BBox::asRect() const {
	return cv::Rect(this->X, this->Y, this->Width, this->Height);
}

std::vector<cv::Point> BBox::asPoints() const {
	return std::vector<cv::Point>{
		cv::Point(this->X, this->Y),
		cv::Point(this->X + this->Width, this->Y),
		cv::Point(this->X + this->Width, this->Y + this->Height),
		cv::Point(this->X, this->Y + this->Height),
	};
}