#ifndef __BTB_DYNAMIC_DETECTOR_H__
#define __BTB_DYNAMIC_DETECTOR_H__

#include "opencv2/opencv.hpp"

namespace BTB
{
  class DynamicDetector
  {
  public:
    DynamicDetector(const cv::Mat &image, const cv::Rect &roi);
    void resync(const cv::Mat &image, const cv::Rect &roi);
    bool track(const cv::Mat &image, cv::Point2f &out);

  private:
    static cv::Mat toGreyscale(const cv::Mat &image);
    cv::Mat oldImage;
    cv::Rect roi;
  };
}

#endif
