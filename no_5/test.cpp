//(OpenCV4) g++ -std=c++11 dip05.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip05.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ
#include <random>
#define FILTER_SIZE 5
#define TH 60
#define MAX_VAL 255

//アニメ調色変換関数の宣言
void myAnimeColor(const cv::Mat& src, cv::Mat& dst);
unsigned char lookupTable[3][256];
//線の本数テーブル
unsigned char lineNumTable[256];

int main (int argc, char *argv[]) {
    //アニメ変換用ルックアップテーブル
    for (int i = 0; i < 256; i++) {
        //H
        lookupTable[0][i] = i;
        //S
        lookupTable[1][i] = i;
        //V
         if (i<64)
            lookupTable[2][i] = 64;
        else if (i<128)
            lookupTable[2][i] = 128;
//        else if (i<192)
//            lookupTable[2][i] = 192;
        else
            lookupTable[2][i] = 255;
        //線の本数
        if (i < 15)
            lineNumTable[i] = 3;
        else if (i < 30)
            lineNumTable[i] = 2;
        else if (i < 40)
            lineNumTable[i] = 1;
        else
            lineNumTable[i] = 0;
        
    }

    
    //①ビデオキャプチャを初期化して，映像を取り込む
    //cv::VideoCapture capture(0);  //内臓カメラをオープン
    cv::VideoCapture capture("scene.mov");  //指定したビデオファイルをオープン
    //ビデオファイルがオープンできたかどうかをチェック
    if (capture.isOpened() == 0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    int width = 720;
    int height = 405;
    cv::Mat frameImage(cv::Size(width,height), CV_8UC3);
    cv::Mat originalImage(cv::Size(width,height), CV_8UC3);
    cv::Mat originalSmallImage(cv::Size(width/5,height/5), CV_8UC3);
    cv::Mat graySmallImage(cv::Size(width/5,height/5), CV_8UC1);
    cv::Mat grayImage(cv::Size(width,height), CV_8UC1);
    cv::Mat medianImage(cv::Size(width,height), CV_8UC1);
    cv::Mat filterImage(cv::Size(width,height), CV_8UC1);
    cv::Mat lineDrawingImage(cv::Size(width,height), CV_8UC1);
    cv::Mat animeImage(cv::Size(width,height), CV_8UC3);
    cv::Mat resultImage(cv::Size(width,height), CV_8UC3);
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Original");
    cv::moveWindow("Original", 0, 50);
    cv::namedWindow("Gray");
    cv::moveWindow("Gray", 0, 200);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 200, 200);
    
    //④線形空間フィルタの生成
    //線形空間フィルタの係数を要素とする1次元配列の作成(5x5の平均値フィルタ用)
     float fdata[] = {
        1, 1, 1,
        1, -8, 1,
        1, 1, 1 
    };

    //一次元配列の要素に基づき，線形空間フィルタをCvMat型の5x5行列"kernel"として生成
    cv::Mat kernel(cv::Size(3,3), CV_32F, fdata);

    //ビデオライタ生成
    cv::VideoWriter rec("rec.mp4", cv::VideoWriter::fourcc('M','P','4','V'), 30, cv::Size(width,height));
    
    //⑤動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し
    while (1) {
        //(a)ビデオキャプチャ"capture"から1フレームを取り込んで，"frameImage"に格納
        capture >> frameImage;
        //ビデオが終了したら無限ループから脱出
        if (frameImage.data == NULL) break;
        
        //(b)"frameImage"をリサイズして"originalImage"に格納
        cv::resize(frameImage, originalSmallImage, originalSmallImage.size());
        cv::resize(frameImage, originalImage, originalImage.size());
        
        //(c)"frameImage"をグレースケールに変換して"graySmallImage"に格納
        cv::cvtColor(originalSmallImage, graySmallImage, cv::COLOR_BGR2GRAY);
        cv::cvtColor(originalImage, grayImage, cv::COLOR_BGR2GRAY);

        //アニメ調生成
        myAnimeColor(originalImage, animeImage);
        resultImage = animeImage.clone();

        // 平滑化
        cv::medianBlur(grayImage, medianImage, FILTER_SIZE);

        //(d)"graySmallImage"に線形空間フィルタ"kernel"を適用して"resultImage"を出力
        cv::filter2D(medianImage, filterImage, graySmallImage.depth(), kernel);

        //二値化
        cv::threshold(filterImage, lineDrawingImage, TH, MAX_VAL, cv::THRESH_BINARY);


        // 非決定的な乱数生成器
        std::random_device rnd;
        std::mt19937 mt(rnd());
        // [0, 15] 範囲の一様乱数
        std::uniform_int_distribution<> rand3(0, 2);
        //走査(暗さを判別)
        for (int y = 0; y < graySmallImage.rows; y++) { //縦
            for (int x = 0; x < graySmallImage.cols; x++) { //横
                //画素値の取得
                uchar s = graySmallImage.at<uchar>(y, x);
                //線の本数に変換
                int lineNum = lineNumTable[s];
                
                if (lineNum > 0) {
                    cv::line(
                        resultImage,
                        cv::Point(x*5+rand3(mt), y*5+rand3(mt)),
                        cv::Point(x*5+4+rand3(mt), y*5+4+rand3(mt)),
                        cv::Scalar(0,0,0),
                        1,
                        8
                    );
                }
                if (lineNum > 1) {
                    cv::line(
                        resultImage,
                        cv::Point(x*5+3+rand3(mt), y*5+rand3(mt)),
                        cv::Point(x*5+4+rand3(mt), y*5+1+rand3(mt)),
                        cv::Scalar(0,0,0),
                        1,
                        8
                    );
                }
                if (lineNum > 2) {
                    cv::line(
                        resultImage,
                        cv::Point(x*5+rand3(mt), y*5+3+rand3(mt)),
                        cv::Point(x*5+1+rand3(mt), y*5+4+rand3(mt)),
                        cv::Scalar(0,0,0),
                        1,
                        8
                    );
                }
            }
        }

        //走査(輪郭)
        for (int y = 0; y < animeImage.rows; y++) { //縦
            for (int x = 0; x < animeImage.cols; x++) { //横
                //画素値の取得
                uchar s = lineDrawingImage.at<uchar>(y, x);
                if (s == 255) { //白
                    resultImage.at<cv::Vec3b>(y, x) = {0, 0, 0}; //黒
                }
            }
        }
        
        //(e)ウィンドウに画像表示
        cv::imshow("Original", originalImage);
        cv::imshow("Anime", animeImage);
        cv::imshow("Result", resultImage);

        rec << resultImage;  //ビデオライタに画像出力
        
        //(f)[q]キーが押されたら無限ループから脱出
        int key = cv::waitKey(10);
        if (key == 'q') break;
    }
    
    //⑥メッセージを出力して終了
    printf("Finished\n");
    return 0;
}



//アニメ調色変換関数（src：入力画像，dst：出力画像）
void myAnimeColor(const cv::Mat& src, cv::Mat& dst) {
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
