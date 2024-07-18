#include "TrackerFunctions.h"

BBox trackBall(cv::Mat& frame, BBox bbox, Ptr<cv::Tracker> tracker) {
    BBox old = bbox;
    // Update tracker
    bool ok = tracker->update(frame, bbox);

    if (ok) {
        // Tracking success
        //cv::rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
        //std::cout << bbox <<"\n";
        return bbox;
    }
    else {
        // Tracking failure
        // Return the previous bbox in hope that the ball is found in the next frame   
        return old;
    }
}