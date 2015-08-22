#include "StaticDetector.hpp"
#include "Files.hpp"

#include <numeric>
#include <stdexcept>
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/features2d.hpp"

BTB::ProcessedImage::ProcessedImage(const cv::SURF &algo, const cv::Mat &image) :
  keypoints(),
  descriptors()
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

  algo.detect(greyImage, keypoints);
  algo.compute(greyImage, keypoints, descriptors);
}

BTB::StaticDetector::StaticDetector() :
  algo(2500),
  processedTrainImages(),
  trainImageSize(),
  matcher()
{
}

BTB::StaticDetector BTB::StaticDetector::CreateFromTrainFolder(const std::string &path)
{
  BTB::StaticDetector result;

  std::vector<std::string> filenames = BTB::GetFilesIn(path);
  for (std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
  {
    std::string filename = *it;
    std::string fullpath = path + "/" + filename;
    cv::Mat image = cv::imread(fullpath);
    if (!image.data)
    {
      throw std::logic_error("error loading: " + fullpath);
    }
    if (!result.addTrainImage(image))
    {
      throw std::logic_error("error loading: " + fullpath + ": no useful information - delete it");
    }
  }

  return result;
}

bool BTB::StaticDetector::addTrainImage(const cv::Mat &image)
{
  if (processedTrainImages.empty())
  {
    trainImageSize = image.size();
  }

  if (trainImageSize != image.size())
  {
    throw std::invalid_argument("image");
  }

  ProcessedImage processedTrainImage(algo, image);
  if (processedTrainImage.descriptors.empty())
  {
    return false;
  }

  processedTrainImages.push_back(processedTrainImage);

  std::vector<cv::Mat> newDescriptor;
  newDescriptor.push_back(processedTrainImage.descriptors);
  matcher.add(newDescriptor);

  return true;
}

cv::Size BTB::StaticDetector::getTrainImageSize()
{
  if (processedTrainImages.empty())
  {
    throw std::logic_error("no train image");
  }

  return trainImageSize;
}

// TODO Remove me
bool DMatchSorter(const cv::DMatch& left, const cv::DMatch& right)
{
  return left.distance < right.distance;
}

bool BTB::StaticDetector::detectIn(const cv::Mat &image, cv::Point2i &out)
{
  ProcessedImage processedSceneImage(algo, image);

  std::vector<cv::DMatch> matches;
  // TODO Use radiusMatch()? with radius < 0.3
  matcher.match(processedSceneImage.descriptors, matches);
  std::sort(matches.begin(), matches.end(), DMatchSorter);
  std::vector<cv::DMatch> goodMatches;
  for (int i = 0; i < matches.size(); i++)
  {
    if (matches[i].distance < 0.3)
    {
      goodMatches.push_back(matches[i]);
    }
  }

  std::vector<cv::Point2i> translationVectors;
  for (int i = 0; i < goodMatches.size(); i++)
  {
    for (int j = i + 1; j < goodMatches.size(); j++)
    {
      cv::Point2i scenePoint1 = processedSceneImage.keypoints[matches[i].queryIdx].pt;
      cv::Point2i scenePoint2 = processedSceneImage.keypoints[matches[j].queryIdx].pt;
      cv::Point2i trainPoint1 = processedTrainImages[matches[i].imgIdx].keypoints[matches[i].trainIdx].pt;
      cv::Point2i trainPoint2 = processedTrainImages[matches[j].imgIdx].keypoints[matches[j].trainIdx].pt;

      double distScene = cv::norm(scenePoint1 - scenePoint2);
      double distTrain = cv::norm(trainPoint1 - trainPoint2);

      double diff = std::abs(distScene - distTrain) / std::min(distScene, distTrain);

      if (diff < 1)
      {
        translationVectors.push_back(scenePoint1 - trainPoint1);
        translationVectors.push_back(scenePoint2 - trainPoint2);
      }
    }
  }

  if (translationVectors.empty())
  {
    return false;
  }

  cv::Point2i translationVectorsSum = std::accumulate(
                                          translationVectors.begin(), translationVectors.end(),
                                          cv::Point2i(0, 0));

  cv::Point2f outFloat = translationVectorsSum * (1.0f / translationVectors.size());
  out = cv::Point2i(int(outFloat.x + 0.5f), int(outFloat.y + 0.5f));

  return true;
}
