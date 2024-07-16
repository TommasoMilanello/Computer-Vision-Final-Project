#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/filesystem.hpp>
#include <iostream>
#include <fstream>

#include "LinesDetector.h"
#include "Segmentator.h"
#include "MiniMap.h"
#include "BBox.h"
#include "BallDetector.h"
#include "Seed.h"

std::vector<cv::Mat> multipleImRead(const std::string& path, const std::string& pattern);
std::vector<std::vector<BBox>> multipleBBoxRead(const std::string& path, const std::string& pattern);
cv::Point differenceVector(cv::Point p1, cv::Point p2);

int main(int argc, char** argv) {

	if (argc <= 2) {
		std::cerr << "Usage: " << argv[0] << " video_path";
		return 0;
	}

	std::string videoPath = argv[1];
	cv::Mat src;
	cv::Mat segmented;
	cv::Mat dst;
	cv::Mat mask;
	cv::Mat ballSegm;
	std::vector<cv::Vec3f> lines;
	std::vector<Seed> ballSeeds;
	std::vector<cv::Point> vertices;

	LinesDetector detect;
	Segmentator segm;
	BallDetector ballDetect;

	cv::VideoCapture cap = cv::VideoCapture(videoPath);
	cv::Mat frame;
	mask = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);
	bool firstFrame = true;
	cv::namedWindow("frame");

	while (cap.isOpened()) {
		bool hasRead = cap.read(frame);

		if (!hasRead) {
			std::cout << "Unable to read the frame, closing..." << std::endl;
			break;
		}

		segm.segmentRegionGrowing(frame, segmented);

		if (firstFrame) {
			detect.detectLinesWithHoughTransform(segmented, dst, lines);
			detect.getRectFromLines(lines, vertices, 0);

			// fill the mask for the table
			cv::drawContours(mask, std::vector<std::vector<cv::Point>>{vertices}, -1, 255, cv::FILLED);

			firstFrame = false;
		}

		// TODO change with Ilyas Segmentation
		ballDetect.findBallSeeds(segmented, dst, ballSeeds, ballSegm, vertices);
		ballDetect.localizeBallsFromSeeds(frame, dst, ballSeeds, std::vector<BBox>());
			
		vertices.push_back(vertices[0]);
		std::vector<std::vector<cv::Point>> verticesArray = { vertices };
		cv::polylines(dst, verticesArray, true, cv::Scalar(0, 255, 0));

		// TODO show minimap + tracking

		cv::imshow("frame", frame);

		if (cv::waitKey(1) == 'q') {
			break;
		}
	}

	cap.release();
	cv::destroyAllWindows();
}

std::vector<cv::Mat> multipleImRead(const std::string& path, const std::string& pattern)
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
			images.push_back(cv::imread(filenames[i]));
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

cv::Point differenceVector(cv::Point p1, cv::Point p2) {
	int diffX = p2.x - p1.x;
	int diffY = p2.y - p1.y;
	return cv::Point(diffX, diffY);
}