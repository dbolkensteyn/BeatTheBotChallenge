#include <iostream>
#include "opencv2/opencv.hpp"
 
using namespace cv;

//#define WIDTH 1280
//#define HEIGHT 1024 // 0.8 * WIDTH

#define WIDTH 640
#define HEIGHT 512 // 0.8 * WIDTH
#define IMAGES 1

std::string to_string(int i);

int main()
{
    VideoCapture cap(0); // open the default camera

    cap.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);

    std::cout << "OpenCV: " << cv::getBuildInformation() << std::endl;
    std::cout << "Webcam width: " << cap.get(CV_CAP_PROP_FRAME_WIDTH) << std::endl;
    std::cout << "Webcam height: " << cap.get(CV_CAP_PROP_FRAME_HEIGHT) << std::endl;
    std::cout << "Webcam FPS: " << cap.get(CV_CAP_PROP_FPS) << std::endl;
    std::cout << "Webcam FOURCC: " << cap.get(CV_CAP_PROP_FOURCC) << std::endl;
    std::cout << "Webcam Format: " << cap.get(CV_CAP_PROP_FORMAT) << std::endl;
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    namedWindow("webcam", WINDOW_AUTOSIZE);

    Mat frames[IMAGES];
    for(int i = 0; i < IMAGES; i++)
    {
    	cap >> frames[i];
    }

    int counter = 0;
    for(int i = 0; ; i = (i + 1) % IMAGES)
    {
		cap >> frames[i];


        Mat calculatedFrame(HEIGHT, WIDTH, CV_32FC3);
        calculatedFrame.setTo(Scalar(0, 0, 0));
        for (int j = i + 1; j < IMAGES; j++)
        {
          accumulate(frames[j], calculatedFrame);
        }
        for (int j = 0; j <= i; j++)
        {
          accumulate(frames[j], calculatedFrame);
        }

        calculatedFrame /= IMAGES;
        calculatedFrame.convertTo(calculatedFrame,CV_8U);
        imshow("webcam", calculatedFrame);

        if (waitKey(30) >= 0)
        {
        	counter++;
        	imwrite("webcam_" + to_string(counter) + ".png", calculatedFrame);
        	std::cout << "image written!" << std::endl;
        }
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}

std::string to_string(int i)
{
  std::stringstream ss;
  ss << i;
  return ss.str();
}
