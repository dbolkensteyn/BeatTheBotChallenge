// g++ robot.cpp -Wc++11-extensions -o robot `pkg-config --cflags --libs opencv`

#include <stdio.h>
#include <iostream>
#include <limits>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/opencv.hpp"

using namespace cv;

#define IMG_MATCHES

class ImageObject
{
public:
  Mat originalImage;
  Mat greyImage;
  std::vector<KeyPoint> keypoints;
  Mat descriptors;

  // TODO: Passing FeatureExtractor here results in a "pure virtual function called" runtime error
  ImageObject(SIFT detector, SIFT extractor, Mat image)
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
  if (!cap.isOpened())
  {
    std::cout << "Error opening the default webcam" << std::endl;
    return -1;
  }

  // See http://stackoverflow.com/questions/16688909/orb-compute-bug-it-removes-all-keypoints-with-a-small-image
  //ORB orb(5000, 1.2f, 8, 14, 0, 2, 0, 14);
  SIFT sift;

  std::vector<ImageObject> trainObjects;
  for (int i = 1; i < 10; i++) // TODO: Remove -1 to use all images as training ones
  {
    if (i == 23)
    {
      continue;
    }
    std::string path = std::string("c170_day/") + std::to_string(i) + std::string(".png");
    //char* path = argv[i];
    std::cout << "Loading training image: " << path << std::endl;
    Mat img = imread(path);
    if (!img.data)
    {
      std::cout<< "Error reading image " << path << std::endl;
      return -1;
    }
    ImageObject trainObject(sift, sift, img);
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

  namedWindow("live", WINDOW_AUTOSIZE);
  for (;;)
  {
    auto startFrame = std::chrono::system_clock::now();

    Mat frame;
    cap >> frame;
    frame = imread(argv[argc - 1]); // TODO: REMOVE ME

    auto startFeatures = std::chrono::system_clock::now();
    ImageObject sceneObject(sift, sift, frame);
    auto endFeatures = std::chrono::system_clock::now();
    double elapsedFeatures = std::chrono::duration_cast<std::chrono::duration<double> >(endFeatures - startFeatures).count();

    // Check for sufficient points to compute homography
    if (sceneObject.keypoints.size() >= 4)
    {
      std::vector<DMatch> matches;
      std::cout << "scene: " << sceneObject.descriptors.size() << std::endl;
      matcher.match(sceneObject.descriptors, matches);

      std::sort(matches.begin(), matches.end(), DMatchSorter);

#ifdef IMG_MATCHES
      Mat trainImg = trainObjects[0].originalImage;
      Rect roi(Point(0, 0), Size(trainImg.cols, trainImg.rows));
      Mat destinationRoi = frame(roi);
      trainImg.copyTo(destinationRoi);
#endif

      std::vector<DMatch> goodMatches;
      for (int i = 0; i < matches.size(); i++)
      {
        if (matches[i].distance < 180)
        {
          std::cout << "adding point " << (i + 1) << " @ " << matches[i].distance << " from img: " << matches[i].imgIdx << std::endl;
          goodMatches.push_back(matches[i]);
        }
      }

      std::vector<Point2f> translationVectors;
      for (int i = 0; i < goodMatches.size(); i++)
      {
        for (int j = i + 1; j < goodMatches.size(); j++)
        {
          Point2f scenePoint1 = sceneObject.keypoints[matches[i].queryIdx].pt;
          Point2f scenePoint2 = sceneObject.keypoints[matches[j].queryIdx].pt;
          Point2f trainPoint1 = trainObjects[matches[i].imgIdx].keypoints[matches[i].trainIdx].pt;
          Point2f trainPoint2 = trainObjects[matches[j].imgIdx].keypoints[matches[j].trainIdx].pt;

          double distScene = norm(scenePoint1 - scenePoint2);
          double distTrain = norm(trainPoint1 - trainPoint2);

          double diff = std::abs(distScene - distTrain) / std::min(distScene, distTrain);

          if (diff < 1)
          {
            std::cout << "good pair @ " << diff << std::endl;

            translationVectors.push_back(scenePoint1 - trainPoint1);
            translationVectors.push_back(scenePoint2 - trainPoint2);

#ifdef IMG_MATCHES
            line(frame, trainPoint1, scenePoint1, Scalar(0, 0, 255), 2);
            line(frame, trainPoint2, scenePoint2, Scalar(0, 0, 255), 2);
#endif
          }
          else
          {
            std::cout << "bad pair @ " << diff << std::endl;
          }
        }
      }

      if (translationVectors.size() > 0)
      {
        Point2f v = translationVectors[0];

        std::vector<Point2f> obj_corners(4);
        obj_corners[0] = cvPoint(0, 0);
        obj_corners[1] = cvPoint(trainObjects[0].greyImage.cols, 0);
        obj_corners[2] = cvPoint(trainObjects[0].greyImage.cols, trainObjects[0].greyImage.rows);
        obj_corners[3] = cvPoint(0, trainObjects[0].greyImage.rows);

        std::vector<Point2f> scene_corners_rect(4);
        scene_corners_rect[0] = obj_corners[0] + v;
        scene_corners_rect[1] = obj_corners[1] + v;
        scene_corners_rect[2] = obj_corners[2] + v;
        scene_corners_rect[3] = obj_corners[3] + v;

        //-- Draw lines between the corners (the mapped object in the scene - image_2 )
        line(frame, scene_corners_rect[0], scene_corners_rect[1], Scalar(0, 255, 0), 4);
        line(frame, scene_corners_rect[1], scene_corners_rect[2], Scalar(0, 255, 0), 4);
        line(frame, scene_corners_rect[2], scene_corners_rect[3], Scalar(0, 255, 0), 4);
        line(frame, scene_corners_rect[3], scene_corners_rect[0], Scalar(0, 255, 0), 4);

#ifndef IMG_MATCHES
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
#endif
      }
    }

    auto endFrame = std::chrono::system_clock::now();

    double elapsedFrame = std::chrono::duration_cast<std::chrono::duration<double> >(endFrame - startFrame).count();

    std::cout << "FPS: " << (1 / elapsedFrame) << " vs " << (1 / elapsedFeatures) << std::endl;

    //-- Show the frame
    //resize(frame, frame, Size(0, 0), 0.25, 0.25);
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
