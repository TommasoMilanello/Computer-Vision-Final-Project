//AUTHOR: Tommaso Milanello

#include "MainUtilities.h"

std::vector<cv::Mat> multipleImRead(const std::string& path, const std::string& pattern, bool asGray)
{
	std::vector<cv::Mat> images;
	std::vector<cv::String> filenames;

	cv::utils::fs::glob(
		path, pattern, filenames
	);
	if (filenames.empty())
	{
		std::cout << "Image folder path not valid";
	}
	else
	{
		for (int i = 0; i < filenames.size(); i++)
		{
			if (asGray) {
				images.push_back(cv::imread(filenames[i], cv::IMREAD_GRAYSCALE));
			} else {
				images.push_back(cv::imread(filenames[i]));
			}			
		}
	}
	return images;
}

std::vector<std::vector<BBox>> multipleBBoxRead(const std::string& path, const std::string& pattern)
{
	std::vector<std::vector<BBox>> result;
	std::vector<cv::String> filenames;

	cv::utils::fs::glob(
		path, pattern, filenames
	);
	if (filenames.empty())
	{
		std::cout << "BBoxes folder path not valid";
	}
	else
	{
		result.resize(filenames.size());

		for (int i = 0; i < filenames.size(); i++)
		{
			std::ifstream file(filenames[i]);

			if (!file) {
				std::cerr << "Error opening file: " << filenames[i] << std::endl;
			}

			std::string line;
			while (std::getline(file, line)) {
				std::istringstream iss(line);
				int x, y, width, heigth, categID;

				if (iss >> x >> y >> width >> heigth >> categID) {
					result[i].emplace_back(x, y, width, heigth, categID);
				}
				else {
					std::cerr << "Error parsing line: " << line << std::endl;
				}
			}
		}
	}

	return result;
}

cv::Point computeCenterOfTableShape(std::vector<cv::Point> vertices) {
	float m1 = (float)(vertices[2].y - vertices[0].y) / (float)(vertices[2].x - vertices[0].x);
	float m2 = (float)(vertices[3].y - vertices[1].y) / (float)(vertices[3].x - vertices[1].x);
	float q1 = vertices[0].y - m1 * vertices[0].x;
	float q2 = vertices[1].y - m2 * vertices[1].x;
	return cv::Point(
		cvRound((q2 - q1) / (m1 - m2)),
		cvRound(((m1 * q2) - (m2 * q1)) / (m1 - m2))
	);
}

std::vector<BBox> convertIntoBBoxes(std::vector<std::vector<int>> toBboxes) {
	std::vector<BBox> result;
	int i = 0;
	for (auto& toBbox : toBboxes) {
		result.push_back(BBox(
			toBbox[0] - toBbox[2],
			toBbox[1] - toBbox[2],
			toBbox[2] * 2,
			toBbox[2] * 2,
			toBbox[3]
		));
	}
	return result;
}