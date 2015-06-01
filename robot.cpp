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
  std::vector<KeyPoint> keypoints;
  Mat descriptors;

  // TODO: Passing FeatureExtractor here results in a "pure virtual function called" runtime error
  ImageObject(ORB detector, ORB extractor, Mat image)
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

    detector.detect(greyImage, keypoints);
    extractor.compute(greyImage, keypoints, descriptors);

    // For FLANN, see http://stackoverflow.com/questions/11565255/opencv-flann-with-orb-descriptors
    //if(descriptors.type() != CV_32F) {
    //  descriptors.convertTo(descriptors, CV_32F);
    //}
  }
};

bool isBlack(Vec3b v);
bool isBlackLine(Mat &im, Point2f p);

bool DMatchSorter(const DMatch& left, const DMatch& right)
{
  return left.distance < right.distance;
}

int main(int argc, char** argv)
{
  VideoCapture cap(0);
  if(!cap.isOpened())
  {
    std::cout<< "Error opening the default webcam" << std::endl;
    return -1;
  }

  // See http://stackoverflow.com/questions/16688909/orb-compute-bug-it-removes-all-keypoints-with-a-small-image
  ORB orb(500, 1.2f, 8, 14, 0, 2, 0, 14);
  //SIFT sift;

  std::vector<ImageObject> trainObjects;
  for (int i = 1; i < 42; i++) // TODO: Remove -1 to use all images as training ones
  {
    if (i == 23)
    {
      continue;
    }
    std::string path = std::string("training/") + std::to_string(i) + std::string(".png");
    //char* path = argv[i];
    std::cout << "Loading training image: " << path << std::endl;
    Mat img = imread(path);
    if (!img.data)
    {
      std::cout<< "Error reading image " << path << std::endl;
      return -1;
    }
    ImageObject trainObject(orb, orb, img);
    trainObjects.push_back(trainObject);
  }

  std::vector<Mat> trainDescriptors;
  for (auto &trainObject: trainObjects)
  {
    trainDescriptors.push_back(trainObject.descriptors);
  }
  BFMatcher matcher; // TODO: BFMatcher vs FlannBasedMatcher
  matcher.add(trainDescriptors);
  matcher.train();

  namedWindow("live", WINDOW_NORMAL);
  for (;;)
  {
    Mat frame;
    cap >> frame;
    frame = imread(argv[argc - 1]); // TODO: REMOVE ME
    ImageObject sceneObject(orb, orb, frame);

    // Check for sufficient points to compute homography
    if (sceneObject.keypoints.size() >= 4)
    {
      std::vector<DMatch> matches;
      std::cout << "scene: " << sceneObject.descriptors.size() << std::endl;
      matcher.match(sceneObject.descriptors, matches);

      std::sort(matches.begin(), matches.end(), DMatchSorter);

      std::vector<Point2f> obj;
      std::vector<Point2f> scene;
      for(int i = 0; i < matches.size(); i++)
      {
        if (matches[i].distance < 50 || i < 4)
        {
          std::cout << "adding point " << (i + 1) << " @ " << matches[i].distance << " from img: " << matches[i].imgIdx << std::endl;
          scene.push_back(sceneObject.keypoints[matches[i].queryIdx].pt);
          obj.push_back(trainObjects[matches[i].imgIdx].keypoints[matches[i].trainIdx].pt);
        }
      }

      std::cout << "points: " << obj.size() << std::endl;

      Mat H = findHomography(obj, scene, CV_LMEDS);

      //-- Get the corners from the image_1 ( the object to be "detected" )
      std::vector<Point2f> obj_corners(4);
      obj_corners[0] = cvPoint(0, 0);
      obj_corners[1] = cvPoint(trainObjects[0].greyImage.cols, 0);
      obj_corners[2] = cvPoint(trainObjects[0].greyImage.cols, trainObjects[0].greyImage.rows);
      obj_corners[3] = cvPoint(0, trainObjects[0].greyImage.rows);

      //-- Find the object corners in the scene
      std::vector<Point2f> scene_corners(4);
      perspectiveTransform(obj_corners, scene_corners, H);

      //-- Forces the object corners to look like a rectangle
      int minX = std::min(scene_corners[0].x, std::min(scene_corners[1].x, std::min(scene_corners[2].x, scene_corners[3].x)));
      int maxX = std::max(scene_corners[0].x, std::max(scene_corners[1].x, std::max(scene_corners[2].x, scene_corners[3].x)));
      int minY = std::min(scene_corners[0].y, std::min(scene_corners[1].y, std::min(scene_corners[2].y, scene_corners[3].y)));
      int maxY = std::max(scene_corners[0].y, std::max(scene_corners[1].y, std::max(scene_corners[2].y, scene_corners[3].y)));

      if (minX > 0 && maxX < frame.cols && minY > 0 && maxY < frame.rows)
      {
        // The detected object fits entirely in the frame

        std::vector<Point2f> scene_corners_rect(4);
        scene_corners_rect[0] = cvPoint(minX,minY);
        scene_corners_rect[1] = cvPoint(maxX, minY);
        scene_corners_rect[2] = cvPoint(maxX, maxY);
        scene_corners_rect[3] = cvPoint(minX, maxY);

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
      }
    }

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
