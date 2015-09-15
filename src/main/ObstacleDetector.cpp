#include "ObstacleDetector.hpp"

std::vector<cv::Rect> BTB::DetectObstacles(const cv::Mat &frame)
{
  cv::vector<cv::Mat> colors(frame.channels());
  split(frame, colors);

  cv::Mat mean;
  cv::addWeighted(colors[0], 0.5, colors[1], 0.5, 0.0, mean);
  cv::addWeighted(mean, 0.67, colors[2], 0.33, 0.0, mean);

  cv::absdiff(colors[0], mean, colors[0]);
  cv::absdiff(colors[1], mean, colors[1]);
  cv::absdiff(colors[2], mean, colors[2]);

  cv::Mat result = colors[0] + colors[1] + colors[2];

  cv::threshold(result, result, 100.0, 255.0, cv::THRESH_BINARY);

  cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 15));
  cv::dilate(result, result, element);

  std::vector<cv::Rect> obstacles;
  std::vector< std::vector<cv::Point> > contours;
  cv::findContours(result, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
  for (std::vector< std::vector<cv::Point> >::iterator it = contours.begin(); it != contours.end(); ++it)
  {
    cv::Rect rect = cv::boundingRect(*it);
    if (rect.width > 20 && rect.height > 20)
    {
      double r = double(countNonZero(result(rect))) / (rect.width * rect.height);
      if (r > 0.6)
      {
        obstacles.push_back(rect);
      }
    }
  }

  return obstacles;
}
