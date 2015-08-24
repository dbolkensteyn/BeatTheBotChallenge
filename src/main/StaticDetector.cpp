#include "StaticDetector.hpp"
#include "Files.hpp"
#include "ExactDetector.hpp"

#include <numeric>
#include <map>
#include <stdexcept>
#include "opencv2/opencv.hpp"
#include "opencv2/nonfree/features2d.hpp"

BTB::ProcessedImage::ProcessedImage(const cv::SURF &algo, const cv::Mat &image, const std::string &imageNumber = "") :
  algo(algo),
  imageNumber(imageNumber),
  greyImage(),
  keypoints(),
  descriptors()
{
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

bool BTB::ProcessedImage::pruneKeypoints(const std::vector<cv::KeyPoint> &newKeypoints)
{
  keypoints = newKeypoints;
  algo.compute(greyImage, keypoints, descriptors);
  return !keypoints.empty();
}

BTB::StaticDetector::StaticDetector() :
  algo(2500),
  processedTrainImages(),
  trainImageSize(),
  matcher()
{
}

BTB::StaticDetector BTB::StaticDetector::CreateFromTrainFolder(const std::string &path, const std::string &pruningPath)
{
  BTB::StaticDetector result;

  std::vector<std::string> filenames = BTB::GetFilesIn(path);
  for (std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
  {
    std::string filename = *it;
    std::string fullpath = path + "/" + filename;

    int p = filename.find_last_of('.');
    if (p == std::string::npos)
    {
      throw std::logic_error("error extracting image number from: " + filename);
    }
    std::string imageNumber = filename.substr(0, p);

    cv::Mat image = cv::imread(fullpath);
    if (!image.data)
    {
      throw std::logic_error("error loading: " + fullpath);
    }
    if (!result.addTrainImage(image, imageNumber))
    {
      throw std::logic_error("error loading: " + fullpath + ": no useful information - delete it");
    }
  }

  if (!pruningPath.empty())
  {
    result.pruneTrainDescriptors(pruningPath);
  }

  return result;
}

bool BTB::StaticDetector::addTrainImage(const cv::Mat &image, const std::string &imageNumber)
{
  if (processedTrainImages.empty())
  {
    trainImageSize = image.size();
  }

  if (trainImageSize != image.size())
  {
    throw std::invalid_argument("image");
  }

  ProcessedImage processedTrainImage(algo, image, imageNumber);
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

void BTB::StaticDetector::pruneTrainDescriptors(const std::string &path)
{
  matcher.clear();

  std::vector<ProcessedImage> pruningImages;
  std::map<std::string, cv::Rect> pruningImagesExpected;
  for (std::vector<ProcessedImage>::iterator it = processedTrainImages.begin(); it != processedTrainImages.end(); ++it)
  {
      std::string fullImagePath = path + "webcam_" + it->imageNumber + ".png";
      cv::Mat fullImage = cv::imread(fullImagePath);
      if (!fullImage.data)
      {
        throw std::logic_error("error loading image: " + fullImagePath);
      }
      ProcessedImage processedImage(algo, fullImage, it->imageNumber);
      pruningImages.push_back(processedImage);

      std::string motoImagePath = path + it->imageNumber + ".png";
      cv::Mat motoImage = cv::imread(motoImagePath);
      if (!motoImage.data)
      {
        throw std::logic_error("error loading image: " + motoImagePath);
      }
      cv::Point2i p = BTB::findExactMatch(fullImage, motoImage);

      const double factor = 0.3;
      cv::Point2i tl(std::max(p.x - int(motoImage.cols * factor), 0), std::max(p.y - int(motoImage.rows * factor), 0));
      cv::Point2i br(std::min(p.x + int(motoImage.cols * (1 + factor)), fullImage.cols), std::min(p.y + int(motoImage.rows * (1 + factor)), fullImage.rows));
      cv::Rect expected(tl, br);
      pruningImagesExpected.insert(std::pair<std::string, cv::Rect>(it->imageNumber, expected));
  }

  for (std::vector<ProcessedImage>::iterator itTraining = processedTrainImages.begin(); itTraining != processedTrainImages.end(); ++itTraining)
  {
    std::vector<int> prunedKeypoints;

    cv::BFMatcher matcher;
    std::vector<cv::Mat> descriptors;
    descriptors.push_back(itTraining->descriptors);
    matcher.add(descriptors);

    for (std::vector<ProcessedImage>::iterator itPruning = pruningImages.begin(); itPruning != pruningImages.end(); ++itPruning)
    {
      std::vector<cv::DMatch> matches = computeMatches(matcher, *itPruning);
      for (std::vector<cv::DMatch>::iterator itMatches = matches.begin(); itMatches != matches.end(); ++itMatches)
      {
        cv::KeyPoint keypoint = itPruning->keypoints[itMatches->queryIdx];
        cv::Point2i point = keypoint.pt;
        if (!pruningImagesExpected[itPruning->imageNumber].contains(point))
        {
          prunedKeypoints.push_back(itMatches->trainIdx);
        }
      }
    }

    std::vector<cv::KeyPoint> goodKeypoints;
    for (int i = 0; i < itTraining->keypoints.size(); i++)
    {
      if (std::find(prunedKeypoints.begin(), prunedKeypoints.end(), i) == prunedKeypoints.end())
      {
        goodKeypoints.push_back(itTraining->keypoints[i]);
      }
    }

    if (itTraining->pruneKeypoints(goodKeypoints))
    {
      std::vector<cv::Mat> newDescriptors;
      newDescriptors.push_back(itTraining->descriptors);
      this->matcher.add(newDescriptors);
    }
  }
}

cv::Size BTB::StaticDetector::getTrainImageSize()
{
  if (processedTrainImages.empty())
  {
    throw std::logic_error("no train image");
  }

  return trainImageSize;
}

std::vector<cv::DMatch> BTB::StaticDetector::computeMatches(cv::BFMatcher &matcher, const BTB::ProcessedImage &processedImage)
{
  // TODO Use radiusMatch()? with radius < 0.3
  std::vector<cv::DMatch> matches;
  matcher.match(processedImage.descriptors, matches);

  std::vector<cv::DMatch> goodMatches;
  for (int i = 0; i < matches.size(); i++)
  {
    if (matches[i].distance < 0.3)
    {
      goodMatches.push_back(matches[i]);
    }
  }
  return goodMatches;
}

bool BTB::StaticDetector::detectIn(const cv::Mat &image, cv::Point2i &out)
{
  ProcessedImage processedSceneImage(algo, image);

  std::vector<cv::DMatch> matches = computeMatches(matcher, processedSceneImage);

  std::vector<cv::Point2i> translationVectors;
  for (int i = 0; i < matches.size(); i++)
  {
    for (int j = i + 1; j < matches.size(); j++)
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
