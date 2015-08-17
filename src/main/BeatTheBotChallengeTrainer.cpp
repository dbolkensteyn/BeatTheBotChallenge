#include <iostream>
#include <sstream>
#include "opencv2/opencv.hpp"

#define WIDTH 640
#define HEIGHT 512 // 0.8 * WIDTH

int main()
{
  cv::VideoCapture cap(0);

  if (!cap.isOpened())
  {
    std::cout << "Unable to open VideoCapture" << std::endl;
    return 1;
  }

  cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

  std::cout << cv::getBuildInformation();
  std::cout << "Webcam width: " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << std::endl;
  std::cout << "Webcam height: " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl;
  std::cout << "Webcam FPS: " << cap.get(CV_CAP_PROP_FPS) << std::endl;
  std::cout << "Webcam FOURCC: " << cap.get(CV_CAP_PROP_FOURCC) << std::endl;
  std::cout << "Webcam Format: " << cap.get(CV_CAP_PROP_FORMAT) << std::endl;

  cv::namedWindow("live", cv::WINDOW_AUTOSIZE);

  auto start = std::chrono::system_clock::now();
  const int maxCounter = 30;
  int counter = 0;
  while (true)
  {
    cv::Mat frame;
	cap >> frame;
    cv::imshow("live", frame);
    counter++;

    if (counter == maxCounter)
    {
      auto end = std::chrono::system_clock::now();
      double elapsed = std::chrono::duration_cast<std::chrono::duration<double> >(end - start).count();
      std::cout << "FPS: " << (maxCounter / elapsed) << ", Elapsed time: " << elapsed << std::endl;
      start = std::chrono::system_clock::now();
      counter = 0;
    }
  }

  return 0;
}
