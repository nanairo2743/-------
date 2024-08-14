//g++ kadai2.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  // 入出力関連ヘッダ
#include <opencv2/opencv.hpp>  // OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{
    int width = 640, height = 480;
    
    // カメラの初期化
    cv::VideoCapture capture(0);  // カメラ0番をオープン
    if (capture.isOpened() == 0) {
        printf("Camera not found\n");
        return -1;
    }
    
    // 画像格納用インスタンス準備
    cv::Mat captureImage;
    cv::Mat frameImage(cv::Size(width, height), CV_8UC3);  
    cv::Mat grayImage(cv::Size(width, height), CV_8UC1);
    cv::Mat resizedImage(cv::Size(width / 4, height / 4), CV_8UC1); 
    cv::Mat resultImage(cv::Size(width, height), CV_8UC1);  
    cv::Mat recImage(cv::Size(width, height), CV_8UC3);  
    
    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", width, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, height);
    
    // ハーフトーニング用の濃度パターンを定義
    const int halftonePatterns[17][16] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 0%
        {255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 6.25%
        {255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, // 12.5%
        {255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0}, // 18.75%
        {255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0}, // 25%
        {255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 0, 0, 255, 0, 0, 0}, // 31.25%
        {255, 0, 255, 0, 255, 0, 0, 0, 255, 0, 255, 0, 255, 0, 0, 0}, // 37.5%
        {255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 0, 0}, // 43.75%
        {255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0, 255, 0}, // 50%
        {255, 0, 255, 0, 255, 0, 255, 255, 255, 0, 255, 0, 255, 0, 255, 0}, // 56.25%
        {255, 0, 255, 255, 255, 0, 255, 255, 255, 0, 255, 0, 255, 0, 255, 0}, // 62.5%
        {255, 0, 255, 255, 255, 0, 255, 255, 255, 255, 255, 0, 255, 0, 255, 0}, // 68.75%
        {255, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 0, 255, 0}, // 75%
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 0, 255, 0}, // 81.25%
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 0}, // 87.5%
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0}, // 93.75%
        {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255}  // 100%
    };

    // ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M','P','4','V'), 30, recImage.size());
    
    // 動画像処理無限ループ
    while (1) {
        // カメラから1フレームを"captureImage"に読み込み
        capture >> captureImage;
        if (captureImage.data == NULL) break;
        
        // "captureImage"をリサイズして"frameImage"に格納
        cv::resize(captureImage, frameImage, frameImage.size());
        
        // "frameImage"をグレースケールに変換して"grayImage"に格納
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        // グレースケール画像を縮小
        cv::resize(grayImage, resizedImage, resizedImage.size());

        // 縮小画像の各画素を17階調に変換
        for (int j = 0; j < resizedImage.rows; ++j) {
            for (int i = 0; i < resizedImage.cols; ++i) {
                // 17段階の階調に変換
                int level = resizedImage.at<unsigned char>(j, i) * 16 / 255;
                const int* pattern = halftonePatterns[level];

                // 各画素を4x4ブロックに展開
                for (int y = 0; y < 4; ++y) {
                    for (int x = 0; x < 4; ++x) {
                        resultImage.at<unsigned char>(j * 4 + y, i * 4 + x) = pattern[y * 4 + x];
                    }
                }
            }
        }

        // ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Result", resultImage);
        
        // 動画ファイル書き出し
        cv::cvtColor(resultImage, recImage, cv::COLOR_GRAY2BGR);  // 動画用3チャンネル画像生成
        rec << recImage;  // ビデオライタに画像出力
        
        // キー入力待ち
        char key = cv::waitKey(20);  // 20ミリ秒待機
        if (key == 'q') break;
    }
    
    return 0;
}
