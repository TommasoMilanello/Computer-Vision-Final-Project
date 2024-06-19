#include "Segmentation_Functions.h"
#include <iostream>
#include <map>


//idea: a pool table has a uniform color which is very similarly illuminated, the HSV space let's us focus only on the hue value and by taking a decent size kernel
//in the middle of the frame we sould manage to segment the table
cv::Mat segmentTable(const cv::Mat& frame)
{
    cv::Mat segmentedFrameHSV;
    cv::cvtColor(frame, segmentedFrameHSV, cv::COLOR_BGR2HSV);
    //find the most frequent hue value in a 100x100 kernel in the middle of the image
    std::map<uchar, int> pixels;
    for (int i = segmentedFrameHSV.rows/2 - 50; i < segmentedFrameHSV.rows/2 + 50; i++)
    {
        for (int j = segmentedFrameHSV.cols/2 - 50; j < segmentedFrameHSV.cols/2 + 50; j++) {
            pixels[segmentedFrameHSV.at<cv::Vec3b>(i,j)[2]]++;
        }
    }
    // now find the most frequent inside the map and segment the frame with only the table hue value
    std::map<uchar, int>::iterator it = pixels.begin();

    uchar mostRepeated = it->first;
    int freq = it->second;
    while (it != pixels.end()) {
        if (it->second > freq) {
            mostRepeated = it->first;
            freq = it->second;
        }
    }

    for (int i = 0; i < segmentedFrameHSV.rows; i++)
    {
        for (int j = 0; i < segmentedFrameHSV.cols; j++) {
            if (segmentedFrameHSV.at<cv::Vec3b>(i, j)[2] != mostRepeated)
                segmentedFrameHSV.at<cv::Vec3b>(i, j)[2] = mostRepeated;
        }
    }

    cv::Mat segmentedFrame;
    cv::cvtColor(segmentedFrameHSV, segmentedFrame, cv::COLOR_HSV2BGR);

    return segmentedFrameHSV;
}

void findHoughLines(cv::Mat* src)
{
    cv::Mat gray;
    cv::cvtColor(*src, gray, cv::COLOR_BGR2GRAY);
    // Store the edges 
    cv::Mat edges;
    // Find the edges in the image using canny detector
    Canny(gray, edges, 60, 240);
    imshow("Edges", edges);
    // Create a vector to store lines of the image
    std::vector<cv::Vec4i> lines;
    // Apply Hough Transform
    int thresh = 210;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, thresh, 10, 250);
    // Draw lines on the image
    for (size_t i = 0; i < lines.size(); i++) {
        cv::Vec4i l = lines[i];
        cv::line(*src, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
    }
}
