#include "DynamicDetector.hpp"

#include <numeric>

BTB::DynamicDetector::DynamicDetector(const cv::Mat &image, const cv::Rect &roi) :
  oldImage(toGreyscale(image)),
  roi(roi)
{
}

void BTB::DynamicDetector::resync(const cv::Mat &image, const cv::Rect &roi)
{
  oldImage = toGreyscale(image);
  this->roi = roi;
}

bool BTB::DynamicDetector::track(const cv::Mat &image, cv::Point2f &out)
{
  cv::Mat greyImage = toGreyscale(image);

  // extract good features to track from ROI
  cv::Mat mask = cv::Mat::zeros(oldImage.size(), CV_8U);
  mask(roi) = 1;
  std::vector<cv::Point2f> corners;
  cv::goodFeaturesToTrack(oldImage, corners, 100, 0.4, 3, mask);
  if (corners.empty())
  {
    return false;
  }

  // compute optical flow
  std::vector<cv::Point2f> newCorners;
  std::vector<unsigned char> newCornersFound;
  std::vector<float> errors;
  cv::calcOpticalFlowPyrLK(oldImage, greyImage, corners, newCorners, newCornersFound, errors);

  // compute average vector
  std::vector<cv::Point2f> translationVectors;
  for (int i = 0; i < corners.size(); i++)
  {
    if (newCornersFound[i])
    {
      translationVectors.push_back(newCorners[i] - corners[i]);
    }
  }

  if (translationVectors.empty())
  {
    return false;
  }

  cv::Point2f translationVectorsSum = std::accumulate(
                                          translationVectors.begin(), translationVectors.end(),
                                          cv::Point2f(0.0f, 0.0f));

  out = translationVectorsSum * (1.0f / translationVectors.size());

  roi = roi + cv::Point2i(int(out.x), int(out.y));
  oldImage = greyImage;

  return true;
}

cv::Mat BTB::DynamicDetector::toGreyscale(const cv::Mat &image)
{
  cv::Mat greyImage;
  if (image.channels() == 1)
  {
    greyImage = image;
  }
  else
  {
    cv::cvtColor(image, greyImage, CV_BGR2GRAY);
  }
  return greyImage;
}
