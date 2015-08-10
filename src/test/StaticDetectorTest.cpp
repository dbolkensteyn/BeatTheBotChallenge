#include "StaticDetector.hpp"
#include "Files.hpp"

#include <stdexcept>
#include <gtest/gtest.h>

#define MAX_MATCH_THRESHOLD 8

cv::Mat loadImage(std::string path)
{
  cv::Mat image = cv::imread(path);
  if (!image.data)
  {
    throw std::invalid_argument("could not load image: " + path);
  }
  return image;
}

bool imageEquals(cv::Mat m1, cv::Mat m2)
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

cv::Point2i findExactMatch(cv::Mat image, cv::Mat templateImage)
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

std::string extractImageNumber(std::string filename)
{
  int p1 = filename.find_last_of('_');
  int p2 = filename.find_last_of('.');
  if (p1 == std::string::npos || p2 == std::string::npos || p1 >= p2)
  {
    throw std::invalid_argument("no image number");
  }

  return filename.substr(p1 + 1, p2 - p1 - 1);
}

TEST(staticDetector, nonregression)
{
  std::string expectedMatchesArray[] = {
    "11", "12", "13", "14", "15", "16", "17", "18", "19",
    "20", "21", "22", "28",
    "33",
    "40"
  };
  std::set<std::string> expectedMatches(expectedMatchesArray, expectedMatchesArray + sizeof(expectedMatchesArray) / sizeof(expectedMatchesArray[0]));

  BTB::StaticDetector detector = BTB::StaticDetector::CreateFromTrainFolder("../../../database/training/");

  int matches = 0;
  int mismatches = 0;
  std::vector<std::string> testImageFilenames = BTB::GetFilesIn("../../../database/testing/");
  for (std::vector<std::string>::iterator it = testImageFilenames.begin(); it != testImageFilenames.end(); ++it)
  {
    std::string testImageFilename = *it;
    std::cout << "Testing image " << testImageFilename << "... ";

    std::string imageNumber = extractImageNumber(testImageFilename);
    cv::Mat testImage = loadImage("../../../database/testing/" + testImageFilename);
    cv::Mat templateImage = loadImage("../../../database/full/" + imageNumber + ".png");

    cv::Point2f expected = findExactMatch(testImage, templateImage);

    cv::Point2f actual;
    bool matched = detector.detectIn(testImage, actual);
    double distance = cv::norm(expected - actual);

    if (matched && distance <= MAX_MATCH_THRESHOLD)
    {
      matches++;
      std::cout << "MATCH, distance = " << distance << std::endl;
      EXPECT_TRUE(expectedMatches.find(imageNumber) != expectedMatches.end()) << "Image " << imageNumber << " is an unexpected match!";
    }
    else
    {
      if (matched)
      {
        mismatches++;
        std::cout << "MISMATCH, distance = " << distance << std::endl;
      }
      else
      {
        std::cout << "NO MATCH" << std::endl;
      }
      EXPECT_TRUE(expectedMatches.find(imageNumber) == expectedMatches.end()) << "Image " << imageNumber << " was expected to match but did not!";
    }
  }

  std::cout << "Matches: " << matches << ", Mismatches: " << mismatches << ", Total: " << testImageFilenames.size() << ", Matching rate: " << (((double)matches / testImageFilenames.size()) * 100) << "%" << ", Mismatching rate: " << (((double)mismatches / testImageFilenames.size()) * 100) << "%" << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
