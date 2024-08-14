// g++ kadai2.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    // ①カメラの初期化
    cv::VideoCapture capture("dance.mov");      // カメラ0番をオープン
    cv::VideoCapture capture2("landscape.mov"); // カメラ0番をオープン
    // カメラがオープンできたかどうかをチェック
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }
    if (capture2.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    int width = 640, height = 360; // 処理画像サイズ
    cv::Mat captureImage;          // キャプチャ用
    cv::Mat captureImage2;
    cv::Mat frameImage = cv::Mat(cv::Size(width, height), CV_8UC3);  // 処理用
    cv::Mat frameImage2 = cv::Mat(cv::Size(width, height), CV_8UC3); // 処理用
    cv::Mat hsvImage;
    cv::Mat hsvImage2;
    cv::Mat recImage = cv::Mat(cv::Size(width, height), CV_8UC3); // ファイル出力用

    // ③ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 0, height);

    //(X)ビデオライタ生成
    cv::VideoWriter rec("dance_land.mpg", cv::VideoWriter::fourcc('P', 'I', 'M', '1'), 30, recImage.size());

    // 動画像処理無限ループ
    while (1)
    {
        // 4カメラから 1 フレーム読み込んで captureImage に格納
        capture >> captureImage;
        capture2 >> captureImage2;

        if (captureImage.data == 0)
            break; // フレーム読み込みに失敗した場合には無限ループから脱出
        // 5captureImage を frameImage に合わせてサイズ変換して格納
        cv::resize(captureImage, frameImage, frameImage.size());

        if (captureImage2.data == 0)
            break; // フレーム読み込みに失敗した場合には無限ループから脱出
        // 5captureImage を frameImage に合わせてサイズ変換して格納
        cv::resize(captureImage2, frameImage2, frameImage2.size());

        // // 6画像処理
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);
        cv::cvtColor(frameImage2, hsvImage2, cv::COLOR_BGR2HSV);

        for (int y = 0; y < hsvImage.rows; y++)
        {
            for (int x = 0; x < hsvImage.cols; x++)
            {
                cv::Vec3b pixel = hsvImage.at<cv::Vec3b>(y, x);
                cv::Vec3b pixel2 = hsvImage2.at<cv::Vec3b>(y, x);
                // 緑色の範囲を指定して抽出
                if (pixel[0] <= 90 && pixel[1] >= 160)
                {
                    hsvImage.at<cv::Vec3b>(y, x) = hsvImage2.at<cv::Vec3b>(y, x);
                }
                else
                {
                }
            }
        }
        // 7ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Frame2", frameImage2);

        //(Y)動画ファイル書き出し
        cv::cvtColor(hsvImage, recImage, cv::COLOR_HSV2BGR); // 動画用3チャンネル画像生成
        rec << recImage;                                     // ビデオライタに画像出力
        cv::imshow("Result", recImage);

        // 8'キー入力待ち
        char key = cv::waitKey(20);
        if (key == 'q')
            break;
    }
    return 0;
}
