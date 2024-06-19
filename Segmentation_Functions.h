#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utils/filesystem.hpp>


cv::Mat segmentTable(const cv::Mat& frame);
void findHoughLines(cv::Mat* src);
