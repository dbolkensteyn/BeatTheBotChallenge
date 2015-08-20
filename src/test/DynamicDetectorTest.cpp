#include "DynamicDetector.hpp"

#include <gtest/gtest.h>
#include <time.h>

const int initialX = 302;
const int initialY = 265;
const int motoWidth = 70;
const int motoHeight = 90;
const int expectedMinTrackLossFrame = 150;

TEST(dynamicDetector, nonregression)
{
  const int expectedFrameCount = 825;
  const int expectedWidth = 640;
  const int expectedHeight = 512;

  const double tolerance = 20;
  const cv::Point2i expected50(285, 260);
  const cv::Point2i expected100(336, 250); // poor
  const cv::Point2i expected150(307, 266); // poor

  cv::VideoCapture cap("../../../database/videos/webcam_1.avi");
  ASSERT_TRUE(cap.isOpened()) << "Unable to open video!";

  int frames = cap.get(CV_CAP_PROP_FRAME_COUNT);
  ASSERT_EQ(expectedFrameCount, frames) << "Unexpected number of frames";
  ASSERT_EQ(expectedWidth, cap.get(CV_CAP_PROP_FRAME_WIDTH)) << "Unexpected width";
  ASSERT_EQ(expectedHeight, cap.get(CV_CAP_PROP_FRAME_HEIGHT)) << "Unexpected height";

  cv::Mat frame;
  cap >> frame;
  ASSERT_TRUE(frame.data) << "Could not load frame";
  cv::Rect roi(initialX, initialY, motoWidth, motoHeight);
  cv::rectangle(frame, roi, cv::Scalar(0, 255, 0), 4);
  cv::imshow("live", frame);
  cv::waitKey(1000);

  BTB::DynamicDetector detector(frame, roi);

  int i = 1;
  while (i < frames)
  {
    cap >> frame;
    ASSERT_TRUE(frame.data) << "Could not load frame";

    cv::Point2f v;
    if (!detector.track(frame, v))
    {
      break;
    }

    roi = roi + cv::Point2i(int(v.x), int(v.y));
    cv::rectangle(frame, roi, cv::Scalar(0, 0, 255), 4);
    cv::imshow("live", frame);
    cv::waitKey(1);

    if (i == 50 || i == 100 || i == 150)
    {
      const cv::Point2i &expected = (i == 50 ? expected50 : (i == 100 ? expected100 : expected150));
      double dist = cv::norm(roi.tl() - expected);
      std::cout << "At frame " << i << " distance is " << dist << " (expected: " << expected << " vs actual: " << roi.tl() << "), higher than tolerated " << tolerance << std::endl;
      EXPECT_TRUE(dist < tolerance);
      cv::rectangle(frame, cv::Rect(expected, cv::Size(motoWidth, motoHeight)), cv::Scalar(0, 255, 0), 4);
      cv::imshow("live", frame);
      cv::waitKey(2000);
    }

    i++;
  }
  EXPECT_TRUE(i > expectedMinTrackLossFrame) << "Tracking algorithm did not reach at expected frame! " << expectedMinTrackLossFrame;
}

TEST(dynamicDetector, performance)
{
  cv::VideoCapture cap("../../../database/videos/webcam_1.avi");
  ASSERT_TRUE(cap.isOpened()) << "Unable to open video!";

  cv::Mat frame;
  cap >> frame;
  ASSERT_TRUE(frame.data) << "Could not load frame";
  cv::Rect roi(initialX, initialY, motoWidth, motoHeight);

  BTB::DynamicDetector detector(frame, roi);

  time_t start;
  time(&start);
  int i = 0;
  for (cap >> frame; frame.data; cap >> frame, i++)
  {
    cv::Point2f v;
    if (!detector.track(frame, v))
    {
      break;
    }
  }
  ASSERT_TRUE(i > expectedMinTrackLossFrame) << "Tracking algorithm did not reach at expected frame! " << expectedMinTrackLossFrame;

  time_t end;
  time(&end);
  double duration = end - start;
  double fps = i / duration;

  std::cout << "Performances: Tracked " << i << " frames in " << duration << " seconds, " << fps << " FPS" << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
