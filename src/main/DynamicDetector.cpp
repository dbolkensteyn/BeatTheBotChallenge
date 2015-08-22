#include "DynamicDetector.hpp"

#include <algorithm>
#include <numeric>

BTB::DynamicDetector::DynamicDetector(BTB::StaticDetector &staticDetector) :
  staticDetector(staticDetector),
  hasLastLocation(false),
  lastLocation()
{
}

bool BTB::DynamicDetector::detectIn(const cv::Mat &frame, cv::Point2i &out)
{
  if (hasLastLocation)
  {
    cv::Point2i tl(std::max(0, lastLocation.x - 30), std::max(0, lastLocation.y - 30));
    cv::Rect roi(tl.x, tl.y, std::min(frame.rows, tl.x + 70 + 2*30) - tl.x, std::min(frame.rows, tl.y + 90 + 2*30) - tl.y);
    cv::Mat image = frame(roi);

    cv::Point2i v;
    if (staticDetector.detectIn(image, v))
    {
      out = v + cv::Point2i(roi.tl().x, roi.tl().y);
      lastLocation = out;
      return true;
    }
  }

  if (staticDetector.detectIn(frame, out))
  {
    hasLastLocation = true;
    lastLocation = out;
  }
  else
  {
    hasLastLocation = false;
  }

  return hasLastLocation;
}
