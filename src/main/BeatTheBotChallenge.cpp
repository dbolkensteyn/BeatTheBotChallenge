#include "StaticDetector.hpp"
#include "PhoneBorderDetector.hpp"
#include "Serial.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>
#include "opencv2/opencv.hpp"

int main(int argc, char** argv)
{
  std::cout << "Opening serial port..." << std::endl;
  BTB::Serial serial("/dev/cu.usbmodemfd121");
  std::cout << "Serial port opened!" << std::endl;
  std::cout << "Banner: " << serial.readLine() << std::endl;

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
    //frame = cv::imread(argv[argc - 1]); // TODO: REMOVE ME
    if (!frame.data) throw std::invalid_argument("frame not valid - does the image exist?");

    cv::Point2f v;
    if (detector.detectIn(frame, v))
    {
      // Draw a rectangle around the detected object
      cv::Size trainImageSize = detector.getTrainImageSize();
      cv::rectangle(frame, cv::Point2f(0, 0) + v, cv::Point2f(trainImageSize.width, trainImageSize.height) + v, cv::Scalar(0, 255, 0), 4);

      // Detect & draw phone left border
      cv::Point2f leftBorder;
      bool hasLeftBorder = borderDetector.detectLeftBorder(frame, v, leftBorder);
      if (hasLeftBorder)
      {
        cv::line(frame, leftBorder, v, cv::Scalar(255, 0, 0), 4);
      }

      // Detect & draw phone right border
      cv::Point2f rightBorder;
      bool hasRightBorder = borderDetector.detectRightBorder(frame, v + cv::Point2f(trainImageSize.width, 0), rightBorder);
      if (hasRightBorder)
      {
        cv::line(frame, v + cv::Point2f(trainImageSize.width, 0), rightBorder, cv::Scalar(255, 0, 0), 4);
      }

      // Compute target line
      if (hasLeftBorder && hasRightBorder)
      {
        int screenWidth = rightBorder.x - leftBorder.x;
        double actual = (v.x + trainImageSize.width / 2 - leftBorder.x) / screenWidth;
        double target = 0.32;
        double targetX = target * screenWidth + leftBorder.x;

        // TODO Remove hardcoded y offsets
        cv::line(frame, cv::Point2f(targetX, leftBorder.y - 30), cv::Point2f(targetX, leftBorder.y + 120), cv::Scalar(0, 0, 255), 4);

        // Send angle correction order to the Arduino
        const int level = 113;
        const int angleMax = 35;

        double distance = actual - target;
        int angle = (int)(distance * angleMax) + 113;

        std::stringstream ss;
        ss << angle;
        std::string angleStr = ss.str();
        std::cout << "Angle: " << angleStr << std::endl;
        serial.writeLine(angleStr);
      }
      else
      {
        // Did not detect borders
        serial.writeLine("113");
      }
    }
    else
    {
      // Did not detect moto
      serial.writeLine("113");
    }

    cv::imshow("live", frame);

    if (cv::waitKey(100) >= 0)
    {
      break;
    }
  }

  return 0;
}
