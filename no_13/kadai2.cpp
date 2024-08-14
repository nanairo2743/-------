#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
#include <fstream>

int main(int argc, char *argv[])
{
    // ビデオキャプチャの初期化
    cv::VideoCapture capture("senro.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0)
    { // オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }

    // 画像格納用インスタンス準備
    cv::Mat frameImage; // ビデオキャプチャ用
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size imageSize(width, height);               // ビデオ画像サイズ
    printf("imageSize = (%d, %d)\n", width, height); // ビデオ画像サイズ表示

    // 画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::namedWindow("Frame2");

    // frameを変数に格納
    cv::Mat frame;

    // 動画処理用無限ループ
    while (1)
    {
        // ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> frameImage;
        // ビデオが終了したら無限ループから脱出
        if (frameImage.data == NULL)
        {
            break;
        }

        // 画像を前の値との平均の値を格納
        if (frame.empty())
        {
            frame = frameImage.clone();
        }
        else
        {
            for (int y = 0; y < frame.rows; y++)
            {
                for (int x = 0; x < frame.cols; x++)
                {
                    cv::Vec3b pixel = frame.at<cv::Vec3b>(y, x);
                    cv::Vec3b pixel2 = frameImage.at<cv::Vec3b>(y, x);
                    for (int i = 0; i < 3; i++)
                    {
                        pixel[i] = (pixel[i] * 0.98 + pixel2[i] * 0.02);
                    }
                    frame.at<cv::Vec3b>(y, x) = pixel;
                }
            }
        }

        // 画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Frame2", frame);

        // キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
    }

    // 最後の画像を保存
    cv::imwrite("output.jpg", frame);

    // 終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}