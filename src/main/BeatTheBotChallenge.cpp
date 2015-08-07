#include <iostream>

#include "opencv2/opencv.hpp"
#include "StaticDetector.hpp"

int main(int argc, char** argv)
{
  cv::VideoCapture cap(0);
  if (!cap.isOpened())
  {
    std::cout << "Error opening the default webcam" << std::endl;
    return -1;
  }

  BTB::StaticDetector detector;
  for (int i = 1; i < 10; i++)
  {
    std::stringstream path;
    path << "database/training/";
    path << i;
    path << ".png";
    std::cout << "Loading training image: " << path.str() << std::endl;

    cv::Mat image = cv::imread(path.str());
    if (!image.data)
    {
      std::cout<< "Error reading image " << path.str() << std::endl;
      return -1;
    }
    detector.addTrainImage(image);
  }

  cv::namedWindow("live", cv::WINDOW_AUTOSIZE);
  for (;;)
  {
    cv::Mat frame;
    cap >> frame;
    frame = cv::imread(argv[argc - 1]); // TODO: REMOVE ME
    cv::Point2f v = detector.detectIn(frame);

    //-- Draw a rectangle around the detected object
    cv::Size trainImageSize = detector.getTrainImageSize();
    cv::rectangle(frame, cv::Point2f(0, 0) + v, cv::Point2f(trainImageSize.width, trainImageSize.height) + v, cv::Scalar(0, 255, 0), 4);

    cv::imshow("live", frame);

    if (cv::waitKey(100) >= 0)
    {
      break;
    }
  }

  return 0;
}
