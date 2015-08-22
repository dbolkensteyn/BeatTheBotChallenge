#ifndef __BTB_PHONE_BORDER_DETECTOR_H__
#define __BTB_PHONE_BORDER_DETECTOR_H__

#include "opencv2/opencv.hpp"

namespace BTB
{
  class PhoneBorderDetector
  {
  public:
    PhoneBorderDetector();

    // from must be a point within the phone screen (e.g. the moto)
    bool detectLeftBorder(const cv::Mat &image, cv::Point2i from, cv::Point2i &out);

    // from must be a point within the phone screen (e.g. the moto)
    bool detectRightBorder(const cv::Mat &image, cv::Point2i from, cv::Point2i &out);
  private:
    bool isBlack(const cv::Vec3b &v);
    bool isBlackVerticalLine(const cv::Mat &image, const cv::Point2i &p);
  };
}

#endif
