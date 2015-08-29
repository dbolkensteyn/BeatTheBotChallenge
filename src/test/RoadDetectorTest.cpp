#include "opencv2/opencv.hpp"

#include <vector>
#include <gtest/gtest.h>

void detectRoad(cv::Mat &image, uchar &b, uchar &g, uchar &r)
{
  const int tolerance = 45;
  const uchar min = 70;
  const uchar max = 255;

  uchar pMin = std::min(std::min(b, g), r);
  uchar pMax = std::max(std::max(b, g), r);
  if (pMin >= min && pMax <= max && pMax - pMin <= tolerance)
  {
    b = 0;
    g = 255;
    r = 0;
  }
}

void detectRoad(cv::Mat &image)
{
  cv::Mat grey;
  cvtColor(image, grey, CV_BGR2GRAY);

  int morph_size = 2;
  cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size( 2*morph_size + 1, 2*morph_size+1 ));
  cv::morphologyEx(grey, grey, cv::MORPH_GRADIENT, element);

  cv::threshold(grey, grey, 150, 255.0, cv::THRESH_BINARY_INV);

  element = getStructuringElement(cv::MORPH_RECT, cv::Size(9, 9));
  cv::erode(grey, grey, element);

  cv::vector< cv::vector<cv::Point> > contours;
  cv::Mat grey2;
  grey.copyTo(grey2);
  cv::findContours(grey2, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
  // filter contours
  for (int i = 0; i < contours.size(); i++)
  {
      cv::Rect rect = cv::boundingRect(contours[i]);

      if (rect.width > 20 && rect.width < 120 && rect.height > 50 && rect.height < 110)
      {
        cv::drawContours(grey, contours, i, cv::Scalar(0), CV_FILLED);
        double area = rect.width * rect.height;
        double r = 1 - cv::countNonZero(grey(rect)) / area;
        if (r > 0.7)
        {
          cv::rectangle(grey, rect, cv::Scalar(0), 5);
        }
        else
        {
          cv::drawContours(grey, contours, i, cv::Scalar(255), CV_FILLED);
        }
      }
      else
      {
        //cv::drawContours(grey, contours, i, cv::Scalar(255), CV_FILLED);
      }
  }

  cv::imshow("live", grey);
  cv::waitKey(10);
}

TEST(roadDetector, nonregression)
{
  cv::VideoCapture cap("../../../database/videos/webcam_1.avi");

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
