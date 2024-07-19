#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp>
#include <opencv2/core/utils/filesystem.hpp>
#include <iostream>
#include <fstream>

#include "BallDetection.h"
#include "BBox.h"
#include "DisplayFunctions.h"
#include "MiniMap.h"
#include "SegmentationEvaluation.h"
#include "TableDetection.h"

using namespace std;
using namespace cv;

const string GROUND_TRUTH_EXTENSION_SEGMENTATION = "*.png";
const string GROUND_TRUTH_EXTENSION_CLASSIFICATION = "*.txt";

vector<Mat> multipleImRead(const string& path, const string& pattern, bool asGray);
vector<vector<BBox>> multipleBBoxRead(const string& path, const string& pattern);
Point computeCenterOfTableShape(vector<Point> vertices);
Point differenceVector(Point p1, Point p2);
vector<BBox> convertIntoBBoxes(vector<tuple<Point, int, int>> toBboxes);

int main(int argc, char** argv) {

	if (argc <= 2) {
		cerr << "Usage: " << argv[0] << " video_path output_type [ground_truth_path]\nutput_type can be [0: 'Ball Localization (circles)', 1: 'Ball Localization (bboxes)', 2: 'Segmentation', 3: 'Video with top-view Map']";
		return 0;
	}

	///////////////////////////////////////////
	//We should do a flag in argv[2] that decides which output the program shows
	///////////////////////////////////////////

	// Read video
	VideoCapture video(argv[1]);

	// Exit if video is not opened
	if (!video.isOpened())
	{
		cout << "Could not read video file" << endl;
		return 1;
	}

	// Read first frame
	Mat frame;
	bool ok = video.read(frame);
	if (!ok) {
		cout << "Cannot read video file" << endl;
		return 1;
	}

	/////////////////////////////////////
	//FIND BBOXES HERE
	/////////////////////////////////////

	// get vertices of the table
	Mat segmented, segmentedWithLines;
	vector<cv::Vec3f> lines;
	vector<cv::Point> vertices;
	segmentRegionGrowing(frame, segmented);
	detectLinesWithHoughTransform(segmented, segmentedWithLines, lines);
	getRectFromLines(lines, vertices, 0);

	// fill the mask for the table
	Mat mask = Mat::zeros(frame.rows, frame.cols, CV_8U);
	cv::drawContours(mask, std::vector<std::vector<cv::Point>>{vertices}, 0, 255, cv::FILLED);

	// ball localization and classification
	Mat roughSegm;
	bitwise_and(frame, frame, roughSegm, mask);
	std::vector<BBox> bboxes = convertIntoBBoxes(detectBalls(roughSegm, mask));

	//////////////////////////////////////
	//INITIALIZE ALL THE TRACKERS
	//////////////////////////////////////
	// std::vector<Ptr<cv::Tracker>> trackers(bboxes.size());
	// std::vector<BBox> newBBoxes = bboxes;
	// for (int i = 0; i < bboxes.size(); i++) {
	// 	trackers[i] = TrackerCSRT::create();
	// 	trackers[i]->init(frame, bboxes[i]);
	// }

	std::vector<BBox> newBBoxes = bboxes;

	legacy::MultiTracker trackers;
	vector<Rect2d> balls;
	vector<Ptr<legacy::Tracker> > algorithms;

	for (int i = 0; i < bboxes.size(); i++) {
		algorithms.push_back(legacy::TrackerCSRT::create());
		balls.push_back(bboxes[i].asRect());
	}

	trackers.add(algorithms,frame,balls);

	// init minimap
	MiniMap map;
	cv::Point center = computeCenterOfTableShape(vertices);
	map.initMiniMap(vertices, center, bboxes);

	// first printing
	Mat output;
	Mat segmMask;
	vector<float> meanIoUValues;
	vector<Mat> groundTruths;
	switch (stoi(argv[2]))
		{
		case 0:
			drawBallLocalization(frame, output, vertices, newBBoxes, false);
			break;
		case 1:
			drawBallLocalization(frame, output, vertices, newBBoxes, true);
			break;
		case 2:
			drawSegmentation(frame, output, vertices, newBBoxes);

			if (argc == 4) {
				groundTruths = multipleImRead(argv[3], GROUND_TRUTH_EXTENSION_SEGMENTATION, true);

				drawSegmentationMask(frame, segmMask, segmented, newBBoxes);
				
				if (groundTruths.empty()) {
					cerr << "No masks recognized for the segmentation metrics evaluation. The format required is .png" << endl;
				} 
				else {
					meanIoUValues.push_back(meanIoU(segmMask, groundTruths[0]));
				}
			}
			break;
		case 3:
			drawFrameWithMiniMap(frame, output, newBBoxes, map);
			break;
		default:
			std::cerr << "Output value not recognized, use [0: 'Ball Localization (circles)', 1: 'Ball Localization (bboxes)', 2: 'Segmentation', 3: 'Video with top-view Map']" << std::endl;
			break;
		}


	Mat prevFrame;
	while (video.read(frame))
	{
		bboxes = newBBoxes;
		//trackBalls(frame, newBBoxes, trackers);
		trackers.update(frame);
		
		for (int i = 0; i < trackers.getObjects().size(); ++i) {
			Rect box = trackers.getObjects()[i];
			newBBoxes[i] = BBox(box.x, box.y, box.width, box.height, bboxes[i].getCategID());
		}

		////////////////////////////////////////////
		//Draw tracking lines here
		///////////////////////////////////////////
		map.updateMiniMap(newBBoxes);


		switch (stoi(argv[2]))
		{
		case 0:
			drawBallLocalization(frame, output, vertices, newBBoxes, false);
			break;
		case 1:
			drawBallLocalization(frame, output, vertices, newBBoxes, true);
			break;
		case 2:
			drawSegmentation(frame, output, vertices, newBBoxes);
			break;
		case 3:
			drawFrameWithMiniMap(frame, output, newBBoxes, map);
			break;
		default:
			std::cerr << "Output value not recognized, use [0: 'Ball Localization (circles)', 1: 'Ball Localization (bboxes)', 2: 'Segmentation', 3: 'Video with top-view Map']" << std::endl;
			break;
		}

		imshow("Video", output);
		prevFrame = frame.clone();
		// Exit if ESC pressed.
		int k = waitKey(1);
		if (k == 27)
		{
			break;
		}
	}

	if (argc == 4) {
		if (stoi(argv[2]) == 2) {
			drawSegmentationMask(prevFrame, segmMask, segmented, newBBoxes);

			if (groundTruths.empty()) {
				cerr << "No masks recognized for the segmentation metrics evaluation. The format required is .png" << endl;
			} 
			else {
				meanIoUValues.push_back(meanIoU(segmMask, groundTruths[1]));
			}
		}

		cout << "First frame/Last frame eval: " << meanIoUValues[0] << "/" << meanIoUValues[1] << endl;
	}

	waitKey(0);

	return 0;
}

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

cv::Point differenceVector(cv::Point p1, cv::Point p2) {
	int diffX = p2.x - p1.x;
	int diffY = p2.y - p1.y;
	return cv::Point(diffX, diffY);
}

std::vector<BBox> convertIntoBBoxes(std::vector<std::tuple<cv::Point, int, int>> toBboxes) {
	std::vector<BBox> result;
	for (auto& toBbox : toBboxes) {
		result.push_back(BBox(
			std::get<0>(toBbox).x - std::get<1>(toBbox),
			std::get<0>(toBbox).y - std::get<1>(toBbox),
			std::get<1>(toBbox) * 2,
			std::get<1>(toBbox) * 2,
			std::get<2>(toBbox)
		));
	}
	return result;
}