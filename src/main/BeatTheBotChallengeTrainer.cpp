#include <iostream>
#include <time.h>
#include <sstream>
#include "opencv2/opencv.hpp"

#define WIDTH 640
#define HEIGHT 512 // 0.8 * WIDTH

static void printAcquireHelp()
{
  std::cout << "HELP: Set focus on the live video and press 'i' to capture an image, 'v' to start/end a video capture or 'q' to quit." << std::endl;
}

static int acquire()
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

  printAcquireHelp();
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
        printAcquireHelp();
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
        printAcquireHelp();
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
      printAcquireHelp(); 
    }

    cv::imshow("live", frame);
  }

  return 0;
}

static int split(std::string filename, int interval, int offset)
{
  cv::VideoCapture cap(filename);
  if (!cap.isOpened())
  {
    std::cout << "Unable to open video file: " << filename << std::endl;
    return 1;
  }

  int counter = 0;
  cv::Mat frame;
  for (cap >> frame; frame.data; cap >> frame, counter++)
  {
    if (counter % interval == 0)
    {
      std::stringstream ss;
      ss << "webcam_";
      ss << offset;
      ss << ".png";
      std::string path = ss.str();

      cv::imwrite(path, frame);
      std::cout << "Image written to: " << path << std::endl;

      offset++;
    }
  }

  return 0;
}

int main(int argc, char* argv[])
{
  std::string executable = std::string(argv[0]);
  if (argc < 2)
  {
    std::cout << "Usage: " << executable << " [acquire|split]" << std::endl;
    return 1;
  }

  std::string command = std::string(argv[1]);
  if (command == "acquire")
  {
    if (argc != 2)
    {
      std::cout << "Usage: " << executable << " acquire" << std::endl;
      return 1;
    }

    return acquire();
  }
  else if (command == "split")
  {
    if (argc != 5)
    {
      std::cout << "Usage: " << executable << " split [filename] [frame interval] [start offset]" << std::endl;
      return 1;
    }

    std::string filename = std::string(argv[2]);
    int interval = std::stoi(std::string(argv[3]));
    int offset = std::stoi(std::string(argv[4]));

    return split(filename, interval, offset);
  }

  std::cout << "Unknown command: " << command << std::endl;
  return 1;
}
