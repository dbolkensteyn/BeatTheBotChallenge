#ifndef __BTB_DYNAMIC_DETECTOR_H__
#define __BTB_DYNAMIC_DETECTOR_H__

#include "StaticDetector.hpp"

#include "opencv2/opencv.hpp"

namespace BTB
{
  class DynamicDetector
  {
  public:
    DynamicDetector(BTB::StaticDetector &staticDetector);
    bool detectIn(const cv::Mat &frame, cv::Point2i &out);

  private:
    BTB::StaticDetector &staticDetector;
    bool hasLastLocation;
    cv::Point2i lastLocation;
  };
}

#endif
