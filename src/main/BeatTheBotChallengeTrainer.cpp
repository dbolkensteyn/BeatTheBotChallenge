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

  cv::namedWindow("webcam", cv::WINDOW_AUTOSIZE);

  int counter = 0;
  while (true)
  {
    cv::Mat frame;
	cap >> frame;

    if (cv::waitKey(30) >= 0)
    {
    	counter++;

        std::stringstream ss;
        ss << "webcam_";
        ss << counter;
        ss << ".png";
        std::string path = ss.str();

    	cv::imwrite(path, frame);
    	std::cout << "Frame written to: " << path << std::endl;
    }

    cv::imshow("live", frame);
  }

  return 0;
}
