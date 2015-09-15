#include "ObstacleDetector.hpp"

#include <gtest/gtest.h>

TEST(roadDetector, nonregression)
{
  cv::VideoCapture cap("../../../database/videos/webcam_2.avi");
  ASSERT_TRUE(cap.isOpened()) << "Unable to open video!";
  int frames = cap.get(CV_CAP_PROP_FRAME_COUNT);

  for (int i = 0; i < frames; i++)
  {
    cv::Mat frame;
    cap >> frame;
    ASSERT_TRUE(frame.data) << "Could not load frame";

    std::vector<cv::Rect> obstacles = BTB::DetectObstacles(frame);
    for (std::vector<cv::Rect>::iterator it = obstacles.begin(); it != obstacles.end(); ++it)
    {
      cv::rectangle(frame, *it, cv::Scalar(255, 0, 0), 3);
    }

    cv::imshow("live", frame);
    cv::waitKey(1);
  }
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
