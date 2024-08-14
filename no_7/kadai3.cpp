//(OpenCV4) g++ -std=c++11 kadai3.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07a.cpp `pkg-config --cflags --libs opencv`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, char *argv[])
{
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("obj.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat backImage(imageSize, CV_8UC3);
    cv::Mat subImage(imageSize, CV_8UC3);
    cv::Mat subBinImage(imageSize, CV_8UC1);
    cv::Mat resultImage(imageSize, CV_8UC3);

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 50, 50);
    cv::namedWindow("Subtraction");
    cv::moveWindow("Subtraction", 100, 100);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 150, 150);

    // ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P', 'I', 'M', '1'), 30, cv::Size(720, 405));

    int flag = 0;

    // ④動画処理用無限ループ
    while (1)
    {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら無限ループから脱出
        if (originalImage.data == NULL)
            break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //(b)"frameImage"と"backImage"との差分画像"subImage"の生成
        cv::absdiff(frameImage, backImage, subImage);
        //(b')"subImage"をグレースケール変換→しきい値処理した画像"subBinImage"を生成
        cv::cvtColor(subImage, subBinImage, cv::COLOR_BGR2GRAY);
        cv::threshold(subBinImage, subBinImage, 30, 255, cv::THRESH_BINARY);

        // ④"binImage"から領域輪郭検出・ラベリング
        std::vector<std::vector<cv::Point>> contours; // 領域輪郭群の格納用
        cv::findContours(subBinImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
        int num = 1;

        // ⑤輪郭を順次抽出・表示
        for (int i = 0; i < contours.size(); i++)
        {
            double length = cv::arcLength(contours[i], true);
            double area = cv::contourArea(contours[i]);
            double enkeido = 4 * M_PI * area / (length * length) * 100;

            if (enkeido < 30 && enkeido > 20)
            {
                cv::drawContours(resultImage, contours, i, cv::Scalar(0, 255, 255), 2, 8);
            }
            if (enkeido < 80 && enkeido > 65)
            {
                cv::drawContours(resultImage, contours, i, cv::Scalar(255, 0, 0), 2, 8);
            }
        }

        //(c)"frameImage"，"backImage"，"subImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Back", backImage);
        cv::imshow("Subtraction", subBinImage);
        cv::imshow("Result", resultImage);

        //(動画ファイル書き出し
        rec << resultImage;  //ビデオライタに画像出力

        resultImage = cv::Scalar(0,0,0);

        //(d)"frameImage"で"backImage"を更新
        if (flag == 0)
        {
            frameImage.copyTo(backImage);
            flag = 1;
        }

        //(e)キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
    }

    // ⑤終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
