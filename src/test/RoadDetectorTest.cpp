#include "opencv2/opencv.hpp"

#include <vector>
#include <gtest/gtest.h>

void detectRoad(cv::Mat &image)
{
  cv::vector<cv::Mat> colors(image.channels());
  split(image, colors);

  cv::Mat mean;
  cv::addWeighted(colors[0], 0.5, colors[1], 0.5, 0.0, mean);
  cv::addWeighted(mean, 0.67, colors[2], 0.33, 0.0, mean);

  cv::absdiff(colors[0], mean, colors[0]);
  cv::absdiff(colors[1], mean, colors[1]);
  cv::absdiff(colors[2], mean, colors[2]);

  cv::Mat result = colors[0] + colors[1] + colors[2];
}

TEST(roadDetector, nonregression)
{
  cv::VideoCapture cap("../../../database/videos/webcam_2.avi");

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
