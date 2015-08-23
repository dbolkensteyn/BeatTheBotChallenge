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

    cv::Point2i expected = findExactMatch(testImage, templateImage);

    cv::Point2i actual;
    bool matched = detector.detectIn(testImage, actual);
    double distance = cv::norm(expected - actual);

    if (matched && distance <= MAX_MATCH_THRESHOLD)
    {
      matches++;
      std::cout << "MATCH, distance = " << distance << std::endl;
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
    }
  }

  std::cout << "Matches: " << matches << ", Mismatches: " << mismatches << ", Total: " << testImageFilenames.size() << ", Matching rate: " << (((double)matches / testImageFilenames.size()) * 100) << "%" << ", Mismatching rate: " << (((double)mismatches / testImageFilenames.size()) * 100) << "%" << std::endl;
}

TEST(staticDetector, performance)
{
  cv::VideoCapture cap("../../../database/videos/webcam_1.avi");
  ASSERT_TRUE(cap.isOpened()) << "Unable to open video!";

  BTB::StaticDetector detector = BTB::StaticDetector::CreateFromTrainFolder("../../../database/training/");

  time_t start;
  time(&start);

  int frames = 0;
  int tracked = 0;
  for (; frames < 200; frames++)
  {
    cv::Mat frame;
    cap >> frame;
    if (!frame.data)
    {
      break;
    }

    cv::Point2i v;
    if (detector.detectIn(frame, v))
    {
      tracked++;
    }
  }

  time_t end;
  time(&end);
  double duration = end - start;
  double fps = frames / duration;

  std::cout << "Performances: Processed " << frames << " frames in " << duration << " seconds, " << fps << " FPS, successfully tracked: " << tracked << " frames or " << ((double(tracked) / frames) * 100) << "%" << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
