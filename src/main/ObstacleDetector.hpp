#ifndef __BTB_OBSTACLE_DETECTOR_H__
#define __BTB_OBSTACLE_DETECTOR_H__

#include <vector>

#include "opencv2/opencv.hpp"

namespace BTB {
  std::vector<cv::Rect> DetectObstacles(const cv::Mat &frame);
}

#endif
