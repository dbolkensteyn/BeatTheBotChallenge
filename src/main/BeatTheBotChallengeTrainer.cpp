#include <iostream>
#include <time.h>
#include <sstream>
#include "opencv2/opencv.hpp"

#define WIDTH 640
#define HEIGHT 512 // 0.8 * WIDTH

static void printHelp()
{
  std::cout << "HELP: Set focus on the live video and press 'i' to capture an image, 'v' to start/end a video capture or 'q' to quit." << std::endl;
}

int main()
{
  cv::VideoCapture cap(0);
  if (!cap.isOpened())
  {
    std::cout << "ERROR: Unable to open VideoCapture" << std::endl;
    return 1;
  }

  cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

  std::cout << cv::getBuildInformation();

  bool videoCapturing = false;
  time_t videoStart;
  std::vector<cv::Mat> videoFrames;
  int counter = 0;

  printHelp();
  cv::namedWindow("live", cv::WINDOW_AUTOSIZE);

  while (true)
  {
    cv::Mat frame;
    cap >> frame;

    if (videoCapturing)
    {
      videoFrames.push_back(frame);
    }

    int key = cv::waitKey(10);
    if (key == 'q')
    {
      if (videoCapturing)
      {
        std::cout << "ERROR: Video capture is still on!" << std::endl;
        printHelp();
      }
      else
      {
        break;
      }
    }
    else if (key == 'i')
    {
      if (videoCapturing)
      {
        std::cout << "ERROR: Video capture is still on!" << std::endl;
        printHelp();
      }
      else
      {
        counter++;

        std::stringstream ss;
        ss << "webcam_";
        ss << counter;
        ss << ".png";
        std::string path = ss.str();

        cv::imwrite(path, frame);
        std::cout << "Image written to: " << path << std::endl;
      }
    }
    else if (key == 'v' && !videoCapturing)
    {
      videoCapturing = true;
      time(&videoStart);
      std::cout << "Video capture started!" << std::endl;
    }
    else if (key == 'v' && videoCapturing)
    {
      time_t videoEnd;
      time(&videoEnd);
      double fps = double(videoFrames.size()) / (videoEnd - videoStart);

      counter++;

      std::stringstream ss;
      ss << "webcam_";
      ss << counter;
      ss << ".avi";
      std::string path = ss.str();

      cv::VideoWriter video(path, CV_FOURCC('X','V','I','D'), fps, cv::Size(WIDTH, HEIGHT));
      if (!video.isOpened())
      {
        std::cout << "ERROR: Unable to open output video file: " << path << std::endl;
        return 1;
      }

      for (std::vector<cv::Mat>::iterator it = videoFrames.begin(); it != videoFrames.end(); ++it)
      {
        video.write(*it);
      }

      std::cout << "Video at " << fps << " FPS written to: " << path << std::endl;

      videoCapturing = false;
      videoFrames.clear();
    }
    else if (key != -1)
    {
      std::cout << "Unknown command '" << (char)key << "'" << std::endl;
      printHelp(); 
    }

    cv::imshow("live", frame);
  }

  return 0;
}
