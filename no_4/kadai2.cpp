#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, const char* argv[]) {
    int width = 640, height = 480;
    
    // カメラの初期化
    cv::VideoCapture capture(0);  //カメラ0番をオープン
    if (capture.isOpened() == 0) {
        printf("Camera not found\n");
        return -1;
    }
    
    // 画像格納用インスタンス準備
    cv::Mat captureImage;  //キャプチャ用
    cv::Mat frameImage(cv::Size(width, height), CV_8UC3);  //処理用
    cv::Mat hsvImage(cv::Size(width, height), CV_8UC3);  // HSV用
    cv::Mat resultImage(cv::Size(width, height), CV_8UC3);  // 結果用

    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, 0);
    
    // ルックアップテーブルの作成
    unsigned char lookupTable[256];
    int levels = 8;
    for (int i = 0; i < 256; i++) {
        lookupTable[i] = ((i * levels) / 256) * 255 / (levels - 1);
    }
    

    // ビデオライタ生成
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M','P','4','V'), 30, frameImage.size());
    
    // 動画像処理無限ループ
    while (1) {
        // カメラから1フレームをcaptureImageに読み込み
        capture >> captureImage;
        if (captureImage.data == NULL) break;

        // frameImageにリサイズ
        cv::resize(captureImage, frameImage, frameImage.size());

        // BGRからHSVに変換
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);

        // HSVの各チャネルに対してルックアップテーブルを適用
        for (int j = 0; j < height; j++) {
            for (int i = 0; i < width; i++) {
                cv::Vec3b pixel = hsvImage.at<cv::Vec3b>(j, i);
                pixel[0] = lookupTable[pixel[0]];  // H
                pixel[1] = lookupTable[pixel[1]];  // S
                pixel[2] = lookupTable[pixel[2]];  // V
                hsvImage.at<cv::Vec3b>(j, i) = pixel;
            }
        }

        // HSVからBGRに再変換
        cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);

        // ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Result", resultImage);

        // 動画ファイル書き出し
        rec << resultImage;

        // キー入力待ち
        char key = cv::waitKey(20);  //20ミリ秒待機
        if (key == 'q') break;
    }
    
    return 0;
}
