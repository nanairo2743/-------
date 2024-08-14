//g++ dip06-1.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <stdio.h>
#include<math.h>
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, const char* argv[])
{
    //①ルートディレクトリの画像ファイル"col.jpg"を読み込んで"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("kadai.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data==0) {  //画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    
    //②グレースケール(2値)画像"grayImage"，処理用2値画像"binImage"，輪郭表示画像"contourImage"の領域確保
    cv::Mat grayImage(sourceImage.size(), CV_8UC1);  //1チャンネル
    cv::Mat binImage(sourceImage.size(), CV_8UC1);  //1チャンネル
    cv::Mat contourImage(sourceImage.size(), CV_8UC3);  //3チャンネル
    
    //③原画像を変換して"grayImage"，"binImage"，"contourImage"を生成
    //"sourceImage"をグレースケール画像に変換して"grayImage"に出力
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);
    //"grayImage"を2値化して"binImage"に出力
    cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY);
    //"sourceImage"のコピーを"contourImage"に出力
    sourceImage.copyTo(contourImage);

    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3,3));
    cv::dilate(binImage, binImage, element, cv::Point(-1,-1), 2);//膨らます
    cv::erode(binImage, binImage, element, cv::Point(-1,-1), 12);//削る
    cv::dilate(binImage, binImage, element, cv::Point(-1,-1), 9);//膨らます



    
    //④"binImage"から領域輪郭検出・ラベリング
    std::vector<std::vector<cv::Point>> contours; //領域輪郭群の格納用
    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    int num = 1;
    
    
    //⑤輪郭を順次抽出・表示
    for(int i=0; i<contours.size(); i++){
        double length = cv::arcLength(contours[i], true);
        double area = cv::contourArea(contours[i]);
        double enkeido = 4*M_PI*area/(length*length)*100;
        
        
        if(enkeido>85){
            cv::drawContours(contourImage, contours, i, cv::Scalar(255,255,255), 2, 8);
            printf("%d length:%f Area:%f enn:%f\n", num, length, area, enkeido);
            num = num + 1;
        }
    }
    //i番目の領域のj番目の頂点：contours[i][j]
    
    //⑥ウィンドウを生成して各画像を表示
    //原画像
    cv::namedWindow("Source");  //ウィンドウの生成
    cv::imshow("Source", sourceImage);  //ウィンドウに画像を表示
    //グレースケール(2値化)
    cv::namedWindow("Bin");  //ウィンドウの生成
    cv::imshow("Bin", binImage);  //ウィンドウに画像を表示
    //輪郭画像(原画像に輪郭を追加)
    cv::namedWindow("Contour");  //ウィンドウの生成
    cv::imshow("Contour", contourImage);  //ウィンドウに画像を表示
    
    //⑦キー入力待ち
    cv::waitKey(0);
    
    printf("Finished\n");
    
    cv::imwrite("result.jpg",contourImage);
    cv::imwrite("2ti.jpg",binImage);
    return 0;
}
