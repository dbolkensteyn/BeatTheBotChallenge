#include "ExactDetector.hpp"

#include <stdexcept>

static bool imageEquals(const cv::Mat &m1, const cv::Mat &m2)
{
  if (m1.rows != m2.rows || m1.cols != m2.cols)
  {
    throw std::invalid_argument("different image sizes!");
  }

  if (m1.rows == 0 || m1.cols == 0)
  {
    throw std::invalid_argument("empty images");
  }

  for (int x = 0; x < m1.cols; x++)
  {
    for (int y = 0; y < m1.rows; y++)
    {
      // TODO: Why doesn't Vec3b work here?
      if (m1.at<uchar>(x, y) != m2.at<uchar>(x, y))
      {
        return false;
      }
    }
  }

  return true;
}

cv::Point2i BTB::findExactMatch(const cv::Mat &image, const cv::Mat &templateImage)
{
  if (image.rows < templateImage.rows || image.cols < templateImage.cols)
  {
    throw std::invalid_argument("templateImage must be smaller or equal to image");
  }

  cv::Rect dimension(0, 0, templateImage.cols, templateImage.rows);
  for (int x = 0; x < image.cols - templateImage.cols + 1; x++)
  {
    for (int y = 0; y < image.rows - templateImage.rows + 1; y++)
    {
      cv::Point2i p(x, y);
      cv::Mat candidate = image(dimension + p);
      if (imageEquals(candidate, templateImage))
      {
        return p;
      }
    }
  }

  throw std::invalid_argument("no exact match");
}
