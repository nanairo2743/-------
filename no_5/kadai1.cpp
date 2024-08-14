//(OpenCV4) g++ -std=c++11 dip05.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip05.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

//アニメ調色変換関数の宣言
void myAnimeColor(const cv::Mat& src, cv::Mat& dst);
unsigned char lookupTable[3][256];

int main (int argc, char *argv[])
{
    //アニメ変換用ルックアップテーブル
    for (int i=0; i<256; i++) {
        //H
        lookupTable[0][i] = i;
        //S
        lookupTable[1][i] = i;
        //V
        if (i<64)
            lookupTable[2][i] = 0;
        else if (i<128)
            lookupTable[2][i] = 85;
        else if (i<196)
            lookupTable[2][i] = 170;
        else
            lookupTable[2][i] = 255;
    }
    
    //①ビデオキャプチャを初期化して，映像を取り込む
    //cv::VideoCapture capture(0);  //内臓カメラをオープン
    cv::VideoCapture capture("scene.mov");  //指定したビデオファイルをオープン
    //ビデオファイルがオープンできたかどうかをチェック
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    int width = 720, height = 405;
    cv::Mat frameImage,medianImage;
    cv::Mat originalImage(cv::Size(width,height), CV_8UC3);
    cv::Mat grayImage(cv::Size(width,height), CV_8UC1);
    cv::Mat resultImage(cv::Size(width,height), CV_8UC1);
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Original");
    cv::moveWindow("Original", 0, 50);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", 0, 200);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 200, 200);
    
    //④線形空間フィルタの生成
    //線形空間フィルタの係数を要素とする1次元配列の作成(5x5の平均値フィルタ用)
    //線形空間フィルタの係数を要素とする 1 次元配列の作成(5x5 の平均値フィルタ用) 
    /*float fdata[] = {
        1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 
        1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 
        1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 
        1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 
        1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0, 1.0/25.0 };*/

     float fdata[] = {
        1, 1, 1,
        1, -8, 1,
        1, 1, 1 
        };
    //一次元配列の要素に基づき，線形空間フィルタを CvMat 型の 5x5 行列"kernel"として生成 
    cv::Mat kernel(cv::Size(3,3), CV_32F, fdata);
    //⑤動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し

    cv::VideoWriter rec("rec.mov", cv::VideoWriter::fourcc('M','P','4','V'), 30, resultImage.size(),false);
    

    while (1) {
        //(a)ビデオキャプチャ"capture"から1フレームを取り込んで，"frameImage"に格納
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data==NULL) break;
        
        //(b)"frameImage"をリサイズして"originalImage"に格納
        cv::resize(frameImage, originalImage, originalImage.size());
        cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);

        cv::medianBlur(grayImage, medianImage, 3);
        
        //(d)"grayImage"に線形空間フィルタ"kernel"を適用して"resultImage"を出力
        cv::filter2D(medianImage, resultImage, grayImage.depth(), kernel);
        
        cv::threshold(resultImage, resultImage, 80, 255, cv::THRESH_BINARY);



        //(e)ウィンドウに画像表示
        cv::imshow("Original", originalImage);
        cv::imshow("Gray", grayImage);
        cv::imshow("Result", resultImage);
        rec << resultImage;  //ビデオライタに画像出力
        
        //(f)[q]キーが押されたら無限ループから脱出
        int key = cv::waitKey(10);
        if (key=='q')
            break;
    }
    
    //⑥メッセージを出力して終了
    printf("Finished\n");
    return 0;
}

//アニメ調色変換関数（src：入力画像，dst：出力画像）
void myAnimeColor(const cv::Mat& src, cv::Mat& dst)
{
    //作業用画像生成
    cv::Mat cImage(src.size(), CV_8UC3);  //3チャンネル
    
    //色変換
    cv::cvtColor(src, cImage, cv::COLOR_BGR2HSV);  //RGB→HSV
    cv::Vec3b s;
    for (int j=0; j<src.rows; j++) {
        for (int i=0; i<src.cols; i++) {
            //ルックアップテーブルで各画素値変換
            s = cImage.at<cv::Vec3b>(j,i);
            s[0] = lookupTable[0][s[0]];
            s[1] = lookupTable[1][s[1]];
            s[2] = lookupTable[2][s[2]];
            cImage.at<cv::Vec3b>(j,i) = s;
        }
    }
    
    cv::cvtColor(cImage, dst, cv::COLOR_HSV2BGR);  //HSV→RGB
}
