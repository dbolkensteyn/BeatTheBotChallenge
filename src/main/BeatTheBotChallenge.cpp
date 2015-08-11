#include "StaticDetector.hpp"
#include "PhoneBorderDetector.hpp"

#include <iostream>
#include <stdexcept>
#include "opencv2/opencv.hpp"

int main(int argc, char** argv)
{
  cv::VideoCapture cap(0);
  if (!cap.isOpened())
  {
    std::cout << "Error opening the default webcam" << std::endl;
    return -1;
  }

  BTB::StaticDetector detector = BTB::StaticDetector::CreateFromTrainFolder("database/training/");
  BTB::PhoneBorderDetector borderDetector;

  cv::namedWindow("live", cv::WINDOW_AUTOSIZE);
  for (;;)
  {
    cv::Mat frame;
    cap >> frame;
    frame = cv::imread(argv[argc - 1]); // TODO: REMOVE ME
    if (!frame.data) throw std::invalid_argument("frame not valid - does the image exist?");

    cv::Point2f v;
    if (detector.detectIn(frame, v))
    {
      // Draw a rectangle around the detected object
      cv::Size trainImageSize = detector.getTrainImageSize();
      cv::rectangle(frame, cv::Point2f(0, 0) + v, cv::Point2f(trainImageSize.width, trainImageSize.height) + v, cv::Scalar(0, 255, 0), 4);

      // Detect & draw phone left border
      cv::Point2f leftBorder;
      if (borderDetector.detectLeftBorder(frame, v, leftBorder))
      {
        cv::line(frame, leftBorder, v, cv::Scalar(255, 0, 0), 4);
      }

      // Detect & draw phone right border
      cv::Point2f rightBorder;
      if (borderDetector.detectRightBorder(frame, v + cv::Point2f(trainImageSize.width, 0), rightBorder))
      {
        cv::line(frame, v + cv::Point2f(trainImageSize.width, 0), rightBorder, cv::Scalar(255, 0, 0), 4);
      }
    }

    cv::imshow("live", frame);

    if (cv::waitKey(100) >= 0)
    {
      break;
    }
  }

  return 0;
}
