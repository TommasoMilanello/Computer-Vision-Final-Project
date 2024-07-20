//AUTHOR: Tommaso Milanello

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracking_legacy.hpp>
#include <iostream>


#include "BallDetection.h"
#include "BBox.h"
#include "DisplayFunctions.h"
#include "MiniMap.h"
#include "SegmentationEvaluation.h"
#include "TableDetection.h"
#include "MainUtilities.h"

using namespace std;
using namespace cv;

const string GROUND_TRUTH_EXTENSION_SEGMENTATION = "*.png";	
const string GROUND_TRUTH_EXTENSION_CLASSIFICATION = "*.txt";


int main(int argc, char** argv) {

	if (argc <= 2) {
		cerr << "Usage: " << argv[0] << " video_path output_type [ground_truth_path]\nOutput_type can be: [0: 'Ball Localization (circles)',\n 1: 'Ball Localization (bboxes)',\n 2: 'Segmentation',\n 3: 'Video with top-view Map',\n 4: 'mIoU',\n 5:'mAP']";
		return 0;
	}

	///////////////////////////////////////////
	//START THE VIDEO READING
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
	//SEGMENTATION + LOCALIZATION + CLASSIFICATION
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
	//INITIALIZIONS
	//////////////////////////////////////

	// initialize multitracker
	legacy::MultiTracker trackers;
	vector<Rect2d> balls;
	vector<Ptr<legacy::Tracker> > algorithms;

	for (int i = 0; i < bboxes.size(); i++) {
		algorithms.push_back(legacy::TrackerCSRT::create());
		balls.push_back(bboxes[i].asRect());
	}

	trackers.add(algorithms,frame,balls);

	// initialize minimap
	MiniMap map;
	cv::Point center = computeCenterOfTableShape(vertices);
	map.initMiniMap(vertices, center, bboxes);

	// first printing
	Mat output;
	Mat segmMask;
	vector<float> meanIoUValues;
	vector<float> mAPValues;
	vector<Mat> groundTruthMasks;
	vector<vector<BBox>> groundTruthBboxes;

	switch (stoi(argv[2]))
		{
		case 0:
			drawBallLocalization(frame, output, vertices, bboxes, false);
			break;
		case 1:
			drawBallLocalization(frame, output, vertices, bboxes, true);
			break;
		case 2:
			drawSegmentation(frame, output, vertices, bboxes);
			break;
		case 3:
			drawFrameWithMiniMap(frame, output, bboxes, map);
			break;
		case 4:
			drawSegmentation(frame, output, vertices, bboxes);
			drawSegmentationMask(frame, segmMask, segmented, bboxes);

			if (argc == 4) {
				groundTruthMasks = multipleImRead(argv[3], GROUND_TRUTH_EXTENSION_SEGMENTATION, true);
				
				if (groundTruthMasks.empty()) {
					cerr << "No masks recognized for the segmentation metrics evaluation. The format required is .png" << endl;
				} 
				else {
					meanIoUValues.push_back(meanIoU(segmMask, groundTruthMasks[0]));
				}
			}
			else {
				cerr << "No path provided for the ground truth" << endl;
			}
			break;
		case 5:
			drawBallLocalization(frame, output, vertices, bboxes, true);
			if (argc == 4) {
				groundTruthBboxes = multipleBBoxRead(argv[3], GROUND_TRUTH_EXTENSION_CLASSIFICATION);
				if (groundTruthBboxes.empty()) {
					cerr << "No bounding boxes recognized for the segmentation metrics evaluation. The format required is .txt" << endl;
				} 
				else {
					mAPValues.push_back(mAP(bboxes, groundTruthBboxes[0]));
				}
			}
			else {
				cerr << "No path provided for the ground truth" << endl;
			}
		default:
			std::cerr << "Output value not recognized, use [0: 'Ball Localization (circles)', 1: 'Ball Localization (bboxes)', 2: 'Segmentation', 3: 'Video with top-view Map', 4: 'Metrics']" << std::endl;
			break;
		}
	imshow("Video", output);

	Mat prevFrame;
	while (video.read(frame))
	{
		trackers.update(frame);
		
		for (int i = 0; i < trackers.getObjects().size(); ++i) {
			Rect box = trackers.getObjects()[i];
			bboxes[i] = BBox(box.x, box.y, box.width, box.height, bboxes[i].getCategID());
		}

		////////////////////////////////////////////
		//Draw tracking lines here
		///////////////////////////////////////////
		map.updateMiniMap(bboxes);


		switch (stoi(argv[2]))
		{
		case 0:
			drawBallLocalization(frame, output, vertices, bboxes, false);
			break;
		case 1:
			drawBallLocalization(frame, output, vertices, bboxes, true);
			break;
		case 2:
			drawSegmentation(frame, output, vertices, bboxes);
			break;
		case 3:
			drawFrameWithMiniMap(frame, output, bboxes, map);
			break;
		case 4:
			drawSegmentation(frame, output, vertices, bboxes);
			break;
		case 5:
			drawBallLocalization(frame, output, vertices, bboxes, true);
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

	if (stoi(argv[2]) == 4) {
		drawSegmentationMask(prevFrame, segmMask, segmented, bboxes);

		if (groundTruthMasks.empty()) {
			cerr << "No masks recognized for the segmentation metrics evaluation. The format required is .png" << endl;
		} 
		else {
			meanIoUValues.push_back(meanIoU(segmMask, groundTruthMasks[1]));
			cout << "First frame/Last frame eval: " << meanIoUValues[0] << "/" << meanIoUValues[1] << endl;
		}
	}

	if (stoi(argv[2]) == 5) {
		if (groundTruthBboxes.empty()) {
				cerr << "No bounding boxes recognized for the segmentation metrics evaluation. The format required is .txt" << endl;
			} 
			else {
				mAPValues.push_back(mAP(bboxes, groundTruthBboxes[1]));
				cout << "First frame/Last frame eval: " << mAPValues[0] << "/" << mAPValues[1] << endl;
			}
	}


	return 0;
}