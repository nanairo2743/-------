#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ
int main(int argc, const char *argv[])
{
    // 1画像ファイルの読み込み
    cv::Mat sourceImage = cv::imread("photo.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0)
    { //画像ファイルが読み込めなかった場合 printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows); //横幅と高さ
    // 2画像格納用インスタンスの生成
    cv::Mat grayImage,binImage;                 // cv::Matクラス

    // 4画像処理
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);
    cv::threshold(grayImage,binImage,128,255,cv::THRESH_BINARY);

    // 5ウィンドウへの画像の表示 
    cv::imshow("Source", sourceImage); 
    cv::imshow("Gray", grayImage); 
    cv::imshow("bin", binImage); 
    // 6キー入力待ち 
    cv::waitKey(0);
    // 7画像の保存 
    cv::imwrite("gray.jpg", grayImage);
    return 0;
}