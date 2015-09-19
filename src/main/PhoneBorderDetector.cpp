#include "PhoneBorderDetector.hpp"

#define BLACK_THRESHOLD 70
#define MIN_VERTICAL_LINE_SIZE 50

BTB::PhoneBorderDetector::PhoneBorderDetector()
{
}

bool BTB::PhoneBorderDetector::detectLeftBorder(const cv::Mat &image, cv::Point2i from, cv::Point2i &out)
{
  while (from.x >= 0)
  {
    if (isBlackVerticalLine(image, from))
    {
      out = from;
      return true;
    }
    from.x--;
  }
  return false;
}

bool BTB::PhoneBorderDetector::detectRightBorder(const cv::Mat &image, cv::Point2i from, cv::Point2i &out)
{
  while (from.x < image.cols)
  {
    if (isBlackVerticalLine(image, from))
    {
      out = from;
      return true;
    }
    from.x++;
  }
  return false;
}

bool BTB::PhoneBorderDetector::isBlack(const cv::Vec3b &v)
{
  const int threshold = BLACK_THRESHOLD;
  return v.val[0] < threshold && v.val[1] < threshold && v.val[2] < threshold;
}

bool BTB::PhoneBorderDetector::isBlackVerticalLine(const cv::Mat &image, const cv::Point2i &p)
{
  for (int i = -MIN_VERTICAL_LINE_SIZE; i < MIN_VERTICAL_LINE_SIZE; i++)
  {
    int y = p.y + i;
    if (y >= 0 && y < image.rows && !isBlack(image.at<cv::Vec3b>(cv::Point2i(p.x, y))))
    {
      return false;
    }
  }
  return true;
}
