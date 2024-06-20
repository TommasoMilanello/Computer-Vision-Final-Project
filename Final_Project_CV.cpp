#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


#include <iostream>
#include <map>

#include "Segmentation_Functions.h"





using namespace cv;

using namespace std;


int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Image folder path not specified";
		return 0;
	}


	string video = argv[1];

	VideoCapture capture(video);

	// Check if camera opened successfully
	if (!capture.isOpened()) {
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	while (1) {

		Mat frame;
		// Capture frame-by-frame
		capture >> frame;

		// If the frame is empty, break immediately
		if (frame.empty())
			break;

		imshow("Original", frame);
		Mat test = frame.clone();


		// find average RGB values in the center since the table is always centered
		std::vector<cv::Vec3b> pixels;
		for (int i = frame.rows / 2 - 20; i < frame.rows / 2 + 20; i++) {
			for (int j = frame.cols / 2 - 20; j < frame.cols / 2 + 20; j++) {
				pixels.push_back(frame.at<cv::Vec3b>(i, j));
				//frame.at<cv::Vec3b>(i, j) = (0, 255, 0);
			}
		}
		int blue = 0;
		int green = 0;
		int red = 0;
		for (auto pixel : pixels) {
			blue += static_cast<int>(pixel[0]);
			green += static_cast<int>(pixel[1]);
			red += static_cast<int>(pixel[2]);
		}
		blue /= pixels.size();
		green /= pixels.size();
		red /= pixels.size();

		int t = 75;

		for (int i = 0; i < frame.rows; i++) {
			for (int j = 0; j < frame.cols; j++) {
				if ((std::abs(frame.at<cv::Vec3b>(i, j)[0] - blue) <= t && std::abs(frame.at<cv::Vec3b>(i, j)[1] - green) <= t && std::abs(frame.at<cv::Vec3b>(i, j)[2] - red) <= t))
					frame.at<Vec3b>(i, j) = Vec3b(255,255,255);
				else
					frame.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			}
		}
		
		Mat segmented = segmentTable(test);

		//findHoughLines(&frame);

		// Display the resulting frame
		imshow("Frame", frame);
		imshow("Segmented", segmented);
		//waitKey(0);
		// Press  ESC on keyboard to exit
		char c = (char)waitKey(25);
		if (c == 27)
			break;
	}
	
	// GAME 2 CLIP 1 SUCKS RIGHT NOW!!!!!!!!!!!!!

	capture.release();

	// Closes all the frames
	destroyAllWindows();

	
	return 0;
}

//hours of work: 6
//Today started at 15:10


/*cout << "\nSEGMENT NOW!!!!\n";

		cv::Mat segmentedFrameHSV;
		cv::cvtColor(frame, segmentedFrameHSV, cv::COLOR_BGR2HSV);

		//find the most frequent hue value in a 20x20 kernel in the middle of the image
		std::map<uchar, int> pixels;
		for (int i = segmentedFrameHSV.cols / 2 - 10; i < segmentedFrameHSV.cols/2 + 10; i++) {
			for (int j = segmentedFrameHSV.rows / 2 - 10; j < segmentedFrameHSV.rows / 2 + 10; j++) {
				pixels[segmentedFrameHSV.at<cv::Vec3b>(i, j)[0]]++;
			}
		}
		// now find the most frequent hue inside the map and segment the frame with only the table value value

		uchar mostFrequentHue = 0;
		int maxCount = 0;

		for (const auto& pair : pixels) {
			if (pair.second > maxCount) {
				maxCount = pair.second;
				mostFrequentHue = pair.first;
			}
		}

		for (int i = 0; i < segmentedFrameHSV.rows; i++)	{
			for (int j = 0; j < segmentedFrameHSV.cols; j++) {
				if (abs(segmentedFrameHSV.at<cv::Vec3b>(i, j)[0] - mostFrequentHue) < 10)	{
					frame.at<cv::Vec3b>(i, j)[0] = 0;
					frame.at<cv::Vec3b>(i, j)[1] = 0;
					frame.at<cv::Vec3b>(i, j)[2] = 0;
				}
			}
		}

		*/


/*	
* 
* 
* 
*/