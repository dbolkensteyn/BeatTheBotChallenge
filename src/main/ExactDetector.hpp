#ifndef __BTB_EXACT_DETECTOR_H__
#define __BTB_EXACT_DETECTOR_H__

#include "opencv2/opencv.hpp"

namespace BTB {
  cv::Point2i findExactMatch(const cv::Mat &image, const cv::Mat &templateImage);
}

#endif
