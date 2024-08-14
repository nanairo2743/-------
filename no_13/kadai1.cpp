//g++ dip13.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main(int argc, char* argv[]) {
    //ビデオキャプチャの初期化
    cv::VideoCapture capture("pantora.mp4");  //ビデオファイルをオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        return -1;
    }
    
    //画像格納用インスタンス準備
    cv::Mat captureImage;  //キャプチャ用
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    cv::Size imageSize(width, height);  //ビデオ画像サイズ
    printf("imageSize = (%d, %d)\n", width, height);  //ビデオ画像サイズ表示
    cv::Mat frameImage = cv::Mat(cv::Size(width,height), CV_8UC3);  //処理用
    cv::Mat grayImage;  //グレースケール画像
    cv::Mat binImage;  //二値化画像
    cv::Mat hsvImage;
    cv::Mat resultImage(cv::Size(width,height), CV_8UC3); 

    //画像表示用ウィンドウの生成
    // cv::namedWindow("Frame");
    //CV_RGB配列を定義
    cv::Scalar colors[] = {
        // CV_RGB(255, 0, 0), //赤
        CV_RGB(0, 255, 0), //緑
        CV_RGB(0, 0, 255), //青
        CV_RGB(255, 255, 0), //黄
        CV_RGB(255, 0, 255), //マゼンタ
        CV_RGB(0, 255, 255), //シアン
        CV_RGB(255, 255, 255), //白
        CV_RGB(0, 0, 0) //黒
    };

    //ビデオライタの生成
    cv::VideoWriter rec(
        "rec.mpg",
        cv::VideoWriter::fourcc('P','I','M','1'),
        30,
        imageSize
    );

    //動画処理用無限ループ
    while (1) {
        //ビデオキャプチャから1フレーム"frameImage"に取り込み
        capture >> captureImage;
        if (captureImage.data == NULL) break;
        
        //(b)" captureImage"をリサイズして" frameImage"に格納
        cv::resize(captureImage, frameImage, frameImage.size());

        resultImage = frameImage.clone();

        //グレースケール変換
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        //二値化
        cv::threshold(grayImage, binImage, 189, 255, cv::THRESH_BINARY_INV);

        //frameImageをHSVに変換
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);

        //膨張縮小処理
        //収縮
        cv::erode(binImage, binImage, cv::Mat(), cv::Point(-1, -1), 11); 
        //膨張
        cv::dilate(binImage, binImage, cv::Mat(), cv::Point(-1, -1), 20);
        //収縮
        cv::erode(binImage, binImage, cv::Mat(), cv::Point(-1, -1), 9);
        
        //座標リスト宣言
        std::vector<std::vector<cv::Point>> contours;

        //輪郭抽出
        cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

        //輪郭走査
        int colorIndex = 0;
        for (int i = 0; i < contours.size(); i++) {
            double l = cv::arcLength(contours[i], true); //周囲長
            double s = cv::contourArea(contours[i]); //面積
            double r = 4.0 * M_PI * s / pow(l, 2); //円形度

            //色を抽出用の座標を取得
            cv::Point pixelColor(contours[i][0].x + 5, contours[i][0].y + 10);
            //一点を描画(デバッグ用)
            // cv::circle(resultImage,pixelColor,3, CV_RGB(255, 255, 255), -1);
            //画素を取得
            cv::Vec3b color = hsvImage.at<cv::Vec3b>(pixelColor.y, pixelColor.x);

            //座標が左上にある時
            if (pixelColor.x < 100 || pixelColor.y < 30) continue;
            if (l < 150) continue;

            // //肌色の判定
            // if ((color[0] > 160 || color[0] < 20) && color[1] > 50 && color[1] < 105 && color[2] < 155) {
            //     continue;
            // }


            //黄色の判定
            if (color[0] > 20 && color[0] < 40 && color[1] > 100 && color[2] > 100) {
                //レモンバウム(赤)
                cv::drawContours(resultImage, contours, i, CV_RGB(255, 0, 0), 3);
                continue;
            }

            //黒色の判定
            if (color[1] < 50 && color[2] < 90) {
                //厚切りチョコケーキ(緑)
                cv::drawContours(resultImage, contours, i, CV_RGB(0, 255, 0), 3);
                continue;
            }

            //四角っぽいやつ
            if (r < 0.67) { //0.65
                printf("%d\n", pixelColor.x);
                if (pixelColor.x > 1000) continue;
                //チョコチップ(シアン)
                cv::drawContours(resultImage, contours, i, CV_RGB(0, 255, 255), 3);
                //色をprint
                printf("Contour[%d]:l = %f, s = %f, r = %f\n", colorIndex, l, s, r);
                printf("Contour[%d]:l = %f, s = %f, r = %f\n", colorIndex, l, s, r);
                colorIndex++;
                continue;
            }
            if (r > 0.83) {
                //チーズブッセ(青)
                cv::drawContours(resultImage, contours, i, CV_RGB(0, 0, 255), 3);
                continue;
            }
            if (r > 0.75) {
                if (pixelColor.x > 1000) continue;
                //バナナブレッド(マゼンタ)
                cv::drawContours(resultImage, contours, i, CV_RGB(255, 0, 255), 3);
                continue;

            } //緑0.806182 青0.585608
        }

        //動画ファイルの書き出し
        rec << resultImage;  //ビデオライタに画像出力


        //画像表示
        cv::imshow("Bin", binImage);
        cv::imshow("Result", resultImage);

        //キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key=='q')
            break;
    }
    
    //終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
