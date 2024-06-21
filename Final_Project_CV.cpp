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

	std::string path = argv[1];
	cv::String pattern = "*.png";

	std::vector<cv::Mat> images = multipleImRead(path, pattern);

	for (int i = 0; i < images.size(); i++) {
		Mat segmented;
		segmented = segmentTable(images[i]);
		
		findHoughLines(segmented);
		imshow(to_string(i), segmented);

	}

	waitKey(0);

//	string video = argv[1];
//
//	VideoCapture capture(video);
//
//	// Check if camera opened successfully
//	if (!capture.isOpened()) {
//		cout << "Error opening video stream or file" << endl;
//		return -1;
//	}
//
//	bool first_frame = true;
//
//	while (1) {
//
//		Mat frame;
//		// Capture frame-by-frame
//		capture >> frame;
//
//		// If the frame is empty, break immediately
//		if (frame.empty())
//			break;
//
//		imshow("Original", frame);
//
//		Mat segmented;
//
//		if (first_frame)
//		{
//			Mat dilated;
//			//erode(frame, dilated, getStructuringElement(MORPH_RECT, Size(9,9)));
//			dilate(frame, dilated, getStructuringElement(MORPH_CROSS, Size(3,3)));
//
//			segmented = segmentTable(dilated);
//
//			findHoughLines(segmented);
//			imshow("Segmented", segmented);
//			waitKey(0);
//			first_frame = false;
//		}
//
//
//
//		// Display the resulting frame
//		imshow("Frame", frame);
//		imshow("Segmented", segmented);
//		//waitKey(0);
//		// Press  ESC on keyboard to exit
//		char c = (char)waitKey(25);
//		if (c == 27)
//			break;
//	}
//	
//
//	capture.release();
//
//	// Closes all the frames
//	destroyAllWindows();
//
//	
//	return 0;
}

//hours of work: 10
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

