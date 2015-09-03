#include "opencv2/opencv.hpp"

#include <vector>
#include <gtest/gtest.h>

#define MIN_OBSTACLE_WIDTH 20
#define MIN_OBSTACLE_HEIGHT 20
#define MIN_OBSTACLE_DENSITY 0.60

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

  cv::threshold(result, result, 100.0, 255.0, cv::THRESH_BINARY);

  cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 15));
  cv::dilate(result, result, element);

  std::vector< std::vector<cv::Point> > contours;
  cv::findContours(result, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
  for (std::vector< std::vector<cv::Point> >::iterator it = contours.begin(); it != contours.end(); ++it)
  {
    cv::Rect rect = cv::boundingRect(*it);
    if (rect.width > MIN_OBSTACLE_WIDTH && rect.height > MIN_OBSTACLE_HEIGHT)
    {
      double r = double(countNonZero(result(rect))) / (rect.width * rect.height);
      if (r > MIN_OBSTACLE_DENSITY)
      {
        cv::rectangle(image, rect, cv::Scalar(0, 255, 0), 4);
      }
    }
  }

  cv::imshow("live", image);
  cv::waitKey(20);
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
