#include "opencv2/opencv.hpp"

#include <vector>
#include <gtest/gtest.h>

void detectRoad(cv::Mat &image, uchar &b, uchar &g, uchar &r)
{
  const int tolerance = 45;
  const uchar min = 70;
  const uchar max = 255;

  uchar pMin = std::min(std::min(b, g), r);
  uchar pMax = std::max(std::max(b, g), r);
  if (pMin >= min && pMax <= max && pMax - pMin <= tolerance)
  {
    b = 0;
    g = 255;
    r = 0;
  }
}

void detectRoad(cv::Mat &image)
{
  for (int x = 0; x < image.rows; x++)
  {
    uchar* p = image.ptr<uchar>(x);
    for (int y = 0; y < image.cols; y++)
    {
      uchar b = *p++;
      uchar g = *p++;
      uchar r = *p++;
      detectRoad(image, b, g, r);
    }
  }
}

TEST(roadDetector, nonregression)
{
  cv::VideoCapture cap("../../../database/videos/webcam_1.avi");
  ASSERT_TRUE(cap.isOpened()) << "Unable to open video!";

  time_t start;
  time(&start);

  int frames = 0;
  for (;;)
  {
    cv::Mat frame;
    cap >> frame;
    if (!frame.data) break;

    detectRoad(frame);
    frames++;
  }

  time_t end;
  time(&end);
  double duration = end - start;
  double fps = frames / duration;

  std::cout << "Performances: Processed " << frames << " frames in " << duration << " seconds, " << fps << " FPS" << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
