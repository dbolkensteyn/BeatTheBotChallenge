#include "StaticDetector.hpp"
#include "DynamicDetector.hpp"
#include "PhoneBorderDetector.hpp"
#include "ObstacleDetector.hpp"
#include "Serial.hpp"

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <time.h>
#include "opencv2/opencv.hpp"

#define WIDTH 640
#define HEIGHT 512 // 0.8 * WIDTH

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
  cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

  BTB::StaticDetector staticDetector = BTB::StaticDetector::CreateFromTrainFolder("database/training/", "database/full/");
  BTB::DynamicDetector detector(staticDetector);
  BTB::PhoneBorderDetector borderDetector;

  time_t lastTargetDecision;
  time(&lastTargetDecision);
  const int laneWidthPx = 78;
  const int middleTarget = laneWidthPx * 1.5;
  bool hasObstacleOnMiddleLane = false;
  int target = middleTarget;

  cv::namedWindow("live", cv::WINDOW_AUTOSIZE);
  for (;;)
  {
    cv::Mat frame;
    cap >> frame;
    if (!frame.data)
    {
      throw std::invalid_argument("captured frame is invalid");
    }

    cv::Point2i v;
    if (detector.detectIn(frame, v))
    {
      // Draw a rectangle around the detected object
      cv::Size trainImageSize = staticDetector.getTrainImageSize();
      cv::rectangle(frame, cv::Point2i(0, 0) + v, cv::Point2i(trainImageSize.width, trainImageSize.height) + v, cv::Scalar(0, 255, 0), 4);

      // Detect & draw phone right border
      cv::Point2i rightBorder;
      bool hasRightBorder = borderDetector.detectRightBorder(frame, v + cv::Point2i(trainImageSize.width, 0), rightBorder);
      if (hasRightBorder)
      {
        cv::line(frame, v + cv::Point2i(trainImageSize.width, 0), rightBorder, cv::Scalar(255, 0, 0), 4);
      }

      // Compute target line
      if (hasRightBorder)
      {
        time_t now;
        time(&now);

        if (now - lastTargetDecision >= 3)
        {
          // Compute new target
          lastTargetDecision = now;

          if (hasObstacleOnMiddleLane)
          {
            target = laneWidthPx;
          }
          else
          {
            target = middleTarget;
          }

          hasObstacleOnMiddleLane = false;
        }

        int actual = v.x + trainImageSize.width / 2;
        int expected = rightBorder.x - target;

        // Detect obstacles
        std::vector<cv::Rect> obstacles = BTB::DetectObstacles(frame);
        for (std::vector<cv::Rect>::iterator it = obstacles.begin(); it != obstacles.end(); ++it)
        {
          cv::rectangle(frame, *it, cv::Scalar(0, 0, 255), 3);
          cv::Point2i tl = it->tl();
          cv::Point2i br = it->br();
          if (tl.x >= rightBorder.x - 2*laneWidthPx && br.x <= rightBorder.x - laneWidthPx)
          {
            hasObstacleOnMiddleLane = true;
            target = laneWidthPx;
          }
        }

        // TODO Remove hardcoded y offsets
        cv::line(frame, cv::Point2i(expected, rightBorder.y - 30), cv::Point2i(expected, rightBorder.y + 120), cv::Scalar(255, 0, 0), 3);

        // Send angle correction order to the Arduino
        const int level = 113;

        int angle;
        int absDiff = std::abs(actual - expected);
        if (absDiff <= 2)
        {
          // close enough
          angle = level;
        }
        else
        {
          int sharp;
          if (absDiff >= 10)
          {
            sharp = 3;
          }
          else
          {
            sharp = 2;
          }

          if (actual < expected)
          {
            // tilt to the left
            angle = level - sharp;
          }
          else
          {
            // tilt to the right
            angle = level + sharp;
          }
        }

        std::stringstream ss;
        ss << angle;
        std::string angleStr = ss.str();
        std::cout << "Angle: " << angleStr << std::endl;
        serial.writeLine(angleStr);
      }
      else
      {
        // Did not detect borders
      }
    }
    else
    {
      // Did not detect moto
    }

    cv::imshow("live", frame);

    if (cv::waitKey(1) >= 0)
    {
      break;
    }
  }

  return 0;
}
