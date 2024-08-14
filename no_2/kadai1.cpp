// g++ dip02.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{

    //①画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("color1.jpg", cv::IMREAD_COLOR);
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
    int mx = sourceImage.cols/2.0;
    int my = sourceImage.rows/2.0;
    int r = 0;

    cv::Vec3b s;
    for (int y = 0; y < sourceImage.rows; y++)
    {
        for (int x = 0; x < sourceImage.cols; x++)
        {
            s = sourceImage.at<cv::Vec3b>(y, x);
            r = sqrt(pow(abs(x - mx),2) + pow(abs(y - my),2));
            cv::Vec3i s1;      // int型の3要素配列s1
            s1[0] = s[0] - r/2; //rの値が大きい程，s1[0]の値も小さくなる 
            s1[1] = s[1] - r/2; //rの値が大きい程，s1[1]の値も小さくなる 
            s1[2] = s[2] - r/2; //rの値が大きい程，s1[2]の値も小さくなる 
            s=s1; //int型をunsignedchar型に丸める 
            resultImage.at<cv::Vec3b>(y, x) = s;
        }
    }
    //⑤ウィンドウへの画像の表示
    cv::imshow("Source", sourceImage);
    cv::imshow("Result", resultImage);

    //⑥キー入力待ち
    cv::waitKey(0);

    //⑦画像の保存

    return 0;
}