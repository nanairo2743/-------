//g++ dip13.cpp -std=c++11 pkg-config --cflags --libs opencv4
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ
#include <fstream>

int main(int argc, char* argv[])
{
    //ビデオキャプチャの初期化
    cv::VideoCapture videoCapture("colorful.mp4");  //ビデオファイルをオープン
    if (videoCapture.isOpened()==0) {  //オープンに失敗した場合
        printf("Video file not found\n");
        return -1;
    }
    
    //画像格納用インスタンス準備
    cv::Mat currentFrame;  //ビデオキャプチャ用
    int frameWidth = videoCapture.get(cv::CAP_PROP_FRAME_WIDTH);
    int frameHeight = videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size frameSize(frameWidth, frameHeight);  //ビデオ画像サイズ
    printf("Frame size = (%d, %d)\n", frameWidth, frameHeight);  //ビデオ画像サイズ表示

    //画像表示用ウィンドウの生成
    cv::namedWindow("Current Frame");
    cv::namedWindow("Binary Image");
    cv::namedWindow("Previous Frame");
    cv::namedWindow("Background");

    // 前のフレームを格納する変数
    cv::Mat previousFrame;
    cv::Mat binaryImage = cv::Mat(frameHeight, frameWidth, CV_8UC3);
    
    // binaryImageを0で初期化
    for(int y = 0; y < currentFrame.rows; y++) {
        for(int x = 0; x < currentFrame.cols; x++) {
            binaryImage.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
        }
    }

    //動画処理用無限ループ
    while (1) {
        //ビデオキャプチャから1フレームcurrentFrameに取り込み
        videoCapture >> currentFrame;
        //ビデオが終了したら無限ループから脱出
        if (currentFrame.data==NULL) {
            break;
        }

        // 前のフレームとの画像の差分を取る
        if ( !previousFrame.empty()) {
            for(int y = 0; y < previousFrame.rows; y++) {
                for(int x = 0; x < previousFrame.cols; x++) {
                    cv::Vec3b currentPixel = previousFrame.at<cv::Vec3b>(y, x);
                    cv::Vec3b nextPixel = currentFrame.at<cv::Vec3b>(y, x);

                    int difference = sqrt(pow(currentPixel[0] - nextPixel[0], 2) + pow(currentPixel[1] - nextPixel[1], 2) + pow(currentPixel[2] - nextPixel[2], 2));
                    if (difference > 20) {
                        binaryImage.at<cv::Vec3b>(y, x) = cv::Vec3b(192, 192, 192);
                    }
                }
            }
        }

        previousFrame = currentFrame.clone();

        //画像表示
        cv::imshow("Current Frame", currentFrame);
        cv::imshow("Previous Frame", previousFrame);
        // cv::imshow("Binary Image", binaryImage);

        //キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }

    cv::Mat backgroundFrame = cv::Mat(frameHeight, frameWidth, CV_8UC3);
    for(int y = 0; y < previousFrame.rows; y++) {
        for(int x = 0; x < previousFrame.cols; x++) {
            cv::Vec3b prevPixel = previousFrame.at<cv::Vec3b>(y, x);
            cv::Vec3b binaryPixel = binaryImage.at<cv::Vec3b>(y, x);

            if (binaryPixel[0] <= 30 && binaryPixel[1] <= 30 && binaryPixel[2] <= 30) {
                backgroundFrame.at<cv::Vec3b>(y, x) = prevPixel;
            } else {
                backgroundFrame.at<cv::Vec3b>(y, x) = binaryPixel;
            }
        }
    }

    //画像表示
    cv::imshow("Binary Image", binaryImage);
    cv::imshow("Background", backgroundFrame);

    // 最後の画像を保存
    cv::imwrite("background.png", backgroundFrame);

    // 一時停止
    cv::waitKey(0);

    //終了処理
    //カメラ終了
    videoCapture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
