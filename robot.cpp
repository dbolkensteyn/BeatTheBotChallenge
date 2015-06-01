// g++ robot.cpp -Wc++11-extensions -o robot `pkg-config --cflags --libs opencv`

#include <stdio.h>
#include <iostream>
#include <limits>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;

class ImageObject
{
public:
  Mat originalImage;
  Mat greyImage;

  ImageObject(Mat image)
  {
    originalImage = image;
    if (image.channels() == 1)
    {
      greyImage = image;
    }
    else
    {
      cvtColor(image, greyImage, CV_BGR2GRAY);
    }
  }
};

bool isBlack(Vec3b v);
bool isBlackLine(Mat &im, Point2f p);

int main(int argc, char** argv)
{
  VideoCapture cap(0);
  if(!cap.isOpened())
  {
    std::cout<< "Error opening the default webcam" << std::endl;
    return -1;
  }

  std::vector<ImageObject> trainObjects;
  for (int i = 1; i < argc - 1; i++) // TODO: Remove -1 to use all images as training ones
  {
    char* path = argv[i];
    std::cout << "Loading training image: " << path << std::endl;
    Mat img = imread(path);
    if (!img.data)
    {
      std::cout<< "Error reading image " << path << std::endl;
      return -1;
    }
    ImageObject trainObject(img);
    trainObjects.push_back(trainObject);
  }

  namedWindow("live", WINDOW_NORMAL);
  for (;;)
  {
    Mat frame;
    cap >> frame;
    frame = imread(argv[argc - 1]); // TODO: REMOVE ME
    ImageObject sceneObject(frame);

    Mat result;
    matchTemplate(sceneObject.originalImage, trainObjects[0].originalImage, result, CV_TM_SQDIFF_NORMED);

    double minVal;
    Point minLoc;
    double maxVal;
    Point maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

    std::vector<Point2f> scene_corners_rect(4);
    scene_corners_rect[0] = cvPoint(minLoc.x, minLoc.y);
    scene_corners_rect[1] = cvPoint(minLoc.x + trainObjects[0].originalImage.cols, minLoc.y);
    scene_corners_rect[2] = cvPoint(minLoc.x + trainObjects[0].originalImage.cols, minLoc.y + trainObjects[0].originalImage.rows);
    scene_corners_rect[3] = cvPoint(minLoc.x, minLoc.y + trainObjects[0].originalImage.rows);

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
    line(frame, scene_corners_rect[0], scene_corners_rect[1], Scalar(0, 255, 0), 4);
    line(frame, scene_corners_rect[1], scene_corners_rect[2], Scalar( 0, 255, 0), 4);
    line(frame, scene_corners_rect[2], scene_corners_rect[3], Scalar( 0, 255, 0), 4);
    line(frame, scene_corners_rect[3], scene_corners_rect[0], Scalar( 0, 255, 0), 4);

    //-- Locate the left border of the phone
    Point2f left_corner = scene_corners_rect[3];
    Point2f left = left_corner;
    while (left.x >= 0)
    {
      if (isBlackLine(frame, left))
      {
        break;
      }
      left.x--;
    }
    line(frame, left_corner, left, Scalar(255, 0, 0), 12);

    //-- Locate the left border of the phone
    Point2f right_corner = scene_corners_rect[2];
    Point2f right = right_corner;
    while (right.x < sceneObject.greyImage.rows)
    {
      if (isBlackLine(frame, right))
      {
        break;
      }
      right.x++;
    }
    line(frame, right_corner, right, Scalar(0, 0, 255), 12);

    //-- Show the frame
    imshow("live", frame);

    if(waitKey(100) >= 0) break;
  }

  return 0;
}

bool isBlack(Vec3b v)
{
  int threshold = 50;
  return v.val[0] < threshold && v.val[1] < threshold && v.val[2] < threshold;
}

bool isBlackLine(Mat &im, Point2f p)
{
  for (int i = -50; i < 50; i++)
  {
    int y = p.y + i;
    if (y >= 0 && y < im.rows && !isBlack(im.at<Vec3b>(Point2f(p.x, y))))
    {
      return false;
    }
  }
  return true;
}
