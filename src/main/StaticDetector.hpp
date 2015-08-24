#ifndef __BTB_STATIC_DETECTOR_H__
#define __BTB_STATIC_DETECTOR_H__

#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/features2d.hpp"

namespace BTB
{
  class ProcessedImage
  {
  public:
    ProcessedImage(const cv::SURF &algo, const cv::Mat &image, const std::string &imageNumber);
    bool pruneKeypoints(const std::vector<cv::KeyPoint> &newKeypoints);
    cv::SURF algo;
    std::string imageNumber;
    cv::Mat greyImage;
    std::vector<cv::KeyPoint> keypoints;
    cv::Mat descriptors;
  };

  class StaticDetector
  {
  public:
    StaticDetector();
    static StaticDetector CreateFromTrainFolder(const std::string &path, const std::string &pruningPath = "");
    bool addTrainImage(const cv::Mat &image, const std::string &imageNumber = "");
    void pruneTrainDescriptors(const std::string &path);
    bool detectIn(const cv::Mat &image, cv::Point2i &out);
    cv::Size getTrainImageSize();

  private:
    static std::vector<cv::DMatch> computeMatches(cv::BFMatcher &matcher, const BTB::ProcessedImage &processedImage);
    cv::SURF algo;
    std::vector<ProcessedImage> processedTrainImages;
    cv::Size trainImageSize;
    cv::BFMatcher matcher;
  };
}

#endif
