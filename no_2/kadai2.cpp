// g++ dip02.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{

    //①画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color2.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0)
    { //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    //②画像格納用オブジェクト"resultImage"の生成
    cv::Mat resultImage = cv::Mat(sourceImage.size(), CV_8UC3);

    //③ウィンドウの生成と移動
    cv::namedWindow("Source");
    cv::moveWindow("Source", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 400, 0);

    //④画像の画素単位の読み込み・処理・書き込み
    /*
    cv::Vec3b s;
    int threshold = 90;
    for (int y = 0; y < sourceImage.rows; y++)
    {
        for (int x = 0; x < sourceImage.cols; x++)
        {
            s = sourceImage.at<cv::Vec3b>(y, x);
            cv::Vec3i s1;      // int型の3要素配列s1
            if(s[1] > s[0] && s[1] > s[2] && s[0] < threshold && s[2] < threshold){
                
            }else{
                s1[0] = 0;
                s1[1] = 0;
                s1[2] = 0;
            }
            s=s1; //int型をunsignedchar型に丸める 
            resultImage.at<cv::Vec3b>(y, x) = s;
        }
    }
    */
    cv::Mat hsvImage;
    cv::cvtColor(sourceImage, hsvImage, cv::COLOR_BGR2HSV);

    // グリーン・アーミーメンの抽出
    for (int y = 0; y < hsvImage.rows; y++) {
        for (int x = 0; x < hsvImage.cols; x++) {
            cv::Vec3b pixel = hsvImage.at<cv::Vec3b>(y, x);
            // 緑色の範囲を指定して抽出
            if (pixel[0] >= 60 && pixel[0] <= 90 && pixel[1] >= 40 && pixel[2] >= 40) {
                // 何もしない（そのまま残す）
            } else {
                // それ以外の部分を黒色にする
                hsvImage.at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
            }
        }
    }
    cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);
    //⑤ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);

    //⑥キー入力待ち
    cv::waitKey(0);

    //⑦画像の保存

    return 0;
}