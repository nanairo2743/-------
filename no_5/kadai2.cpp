#include <iostream>
#include <opencv2/opencv.hpp>

void myAnimeColor(const cv::Mat& src, cv::Mat& dst);
unsigned char lookupTable[3][256];

int main(int argc, char *argv[])
{
    // アニメ変換用ルックアップテーブルの設定
    for (int i = 0; i < 256; i++) {
        lookupTable[0][i] = i;
        lookupTable[1][i] = i;
        if (i < 64)
            lookupTable[2][i] = 0;
        else if (i < 128)
            lookupTable[2][i] = 85;
        else if (i < 196)
            lookupTable[2][i] = 170;
        else
            lookupTable[2][i] = 255;
    }

    cv::VideoCapture capture("scene.mov");
    if (!capture.isOpened()) {
        std::cerr << "Video not found" << std::endl;
        return -1;
    }

    cv::Mat frame, animeFrame, grayFrame, edgeFrame, finalFrame;
    int width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::VideoWriter writer("anime_scene.mov", cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 30, cv::Size(width, height));

    while (true) {
        capture >> frame;
        if (frame.empty()) break;

        myAnimeColor(frame, animeFrame);

        cv::cvtColor(animeFrame, grayFrame, cv::COLOR_BGR2GRAY);
        cv::medianBlur(grayFrame, grayFrame, 7);
        cv::Laplacian(grayFrame, edgeFrame, CV_8U, 5);
        cv::threshold(edgeFrame, edgeFrame, 80, 255, cv::THRESH_BINARY_INV);

        cv::cvtColor(edgeFrame, edgeFrame, cv::COLOR_GRAY2BGR);
        finalFrame = animeFrame & edgeFrame;

        writer.write(finalFrame);

        cv::imshow("Anime", finalFrame);
        if (cv::waitKey(10) == 'q') break;
    }

    capture.release();
    writer.release();
    cv::destroyAllWindows();
    return 0;
}

void myAnimeColor(const cv::Mat& src, cv::Mat& dst)
{
    cv::Mat hsvImage;
    cv::cvtColor(src, hsvImage, cv::COLOR_BGR2HSV);
    for (int j = 0; j < src.rows; j++) {
        for (int i = 0; i < src.cols; i++) {
            cv::Vec3b &pixel = hsvImage.at<cv::Vec3b>(j, i);
            pixel[0] = lookupTable[0][pixel[0]];
            pixel[1] = lookupTable[1][pixel[1]];
            pixel[2] = lookupTable[2][pixel[2]];
        }
    }
    cv::cvtColor(hsvImage, dst, cv::COLOR_HSV2BGR);
}
