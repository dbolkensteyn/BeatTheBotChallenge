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
  cv::Mat grey;
  cvtColor(image, grey, CV_BGR2GRAY);

  int morph_size = 50;

  cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size( 2*morph_size + 1, 2*morph_size+1 ));
  cv::Mat op1;
  cv::dilate(grey, op1, element);

  morph_size = 2;
  element = getStructuringElement(cv::MORPH_RECT, cv::Size( 2*morph_size + 1, 2*morph_size+1 ));
  cv::Mat op2;
  cv::erode(grey, op2, element);
  cv::Mat result = op1 - op2;

  // phone borders here

  cv::threshold(result, result, 170, 255.0, cv::THRESH_BINARY);

  cv::imshow("live", result);
  cv::waitKey(10);
}

TEST(roadDetector, nonregression)
{
  cv::VideoCapture cap("../../../database/videos/webcam_1.avi");

  //cv::VideoCapture cap(0);
  //cap.set(CV_CAP_PROP_FRAME_WIDTH, 640);
  //cap.set(CV_CAP_PROP_FRAME_HEIGHT, 512);

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
