#include <iostream>
#include <opencv2/opencv.hpp>

int main (int argc, char * argv[])
{
    cv::VideoCapture capture("room.mov");  // Use the provided video file
    if (!capture.isOpened()) {
        std::cerr << "Error: Could not open video file." << std::endl;
        return -1;
    }
    
    cv::Size imageSize(720, 405);
    cv::Mat originalImage, frameImage(imageSize, CV_8UC3), optImage(imageSize, CV_8UC3), resultImage;
    cv::VideoWriter rec("rec4.mpg", cv::VideoWriter::fourcc('P','I','M','1'), 30, imageSize);
    
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("OpticalFlow");
    cv::moveWindow("OpticalFlow", 50, 0);
    
    cv::Mat priorImage(imageSize, CV_8UC1);
    cv::Mat presentImage(imageSize, CV_8UC1);
    std::vector<cv::Point2f> priorFeature, presentFeature;
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 20, 0.05);
    std::vector<unsigned char> status;
    std::vector<float> errors;
    
    while (true) {
        capture >> originalImage;
        if (originalImage.empty()) break;
        cv::resize(originalImage, frameImage, imageSize);
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);
        
        if (!priorImage.empty()) {
            cv::goodFeaturesToTrack(priorImage, priorFeature, 300, 0.01, 10);
            
            if (!priorFeature.empty()) {
                cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10, 10), 4, criteria);
                
                for (size_t i = 0; i < priorFeature.size(); i++) {
                    if (status[i]) {
                        cv::Point2f pt1 = priorFeature[i];
                        cv::Point2f pt2 = presentFeature[i];
                        double distance = cv::norm(pt2 - pt1);
                        
                        if (distance > 15 && distance < 50) {
                            cv::line(optImage, pt1, pt2, cv::Scalar(0, 255, 0), 2);
                            
                            // Draw arrow to indicate direction
                            double angle = atan2(pt1.y - pt2.y, pt1.x - pt2.x);
                            double hypotenuse = cv::norm(pt1 - pt2);
                            pt2.x = (int)(pt1.x - 3 * hypotenuse * cos(angle));
                            pt2.y = (int)(pt1.y - 3 * hypotenuse * sin(angle));
                            cv::arrowedLine(frameImage, pt1, pt2, cv::Scalar(0, 0, 255), 2);
                        }
                    }
                }
            }
        }
        
        presentImage.copyTo(priorImage);
        
        rec << frameImage;
        cv::imshow("Frame", originalImage);
        cv::imshow("OpticalFlow", priorImage);
        
        if (cv::waitKey(20) == 'q') {
            break;
        }
    }
    
    capture.release();
    std::cout << "Finished" << std::endl;
    return 0;
}
