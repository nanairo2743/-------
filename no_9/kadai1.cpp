/*
g++ dip09.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
g++ dip09.cpp `pkg-config --cflags --libs opencv`
 */

#include <iostream>           // 入出力関連ヘッダ
#include <opencv2/opencv.hpp> // OpenCV関連ヘッダ

int main(int argc, char *argv[])
{
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("balls.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0)
    {
        printf("Capture not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    int w = capture.get(cv::CAP_PROP_FRAME_WIDTH);  // captureから動画横サイズ取得
    int h = capture.get(cv::CAP_PROP_FRAME_HEIGHT); // captureから動画縦サイズ取得
    cv::Size imageSize(w, h);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3); // 3チャンネル
    cv::Mat grayImage(imageSize, CV_8UC1);  // 1チャンネル
    cv::Mat edgeImage(imageSize, CV_8UC1);  // 1チャンネル

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Edge");
    cv::moveWindow("Edge", 100, 100);

    // ④ハフ変換用変数
    std::vector<cv::Vec2f> lines;     // ρ, θの組で表現される直線群
    std::vector<cv::Point3f> circles; //(cx,cy)とrの組で表現される円群

    // ⑤動画処理用無限ループ
    while (1)
    {
        // (a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら巻き戻し
        if (originalImage.empty())
        {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            continue;
        }
        // "originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        // (b)"frameImage"からグレースケール画像"grayImage"を生成
        cv::cvtColor(frameImage, grayImage, cv::COLOR_BGR2GRAY);

        // (c)"grayImage"からエッジ画像"edgeImage"を生成
        //cv::Canny(grayImage, edgeImage, 120, 160, 3); // ケニーのエッジ検出アルゴリズム

        //(d)"edgeImage"に直線検出ハフ変換を施して，閾値(250)以上の投票数を得た直線群(ρ,θ)を"lines"に格納
        //cv::HoughLines(edgeImage, lines, 1, CV_PI / 180, 250);
        //(d')"grayImage"に円検出ハフ変換を施して，しきい値(90)以上の得票数を得た円群(x0,y0,r)を"circles"に格納
        cv::HoughCircles(grayImage, circles, cv::HOUGH_GRADIENT, 1, 20, 30, 15, 7, 15);

        // (e)ハフ変換結果表示
        // 検出された直線の数("lines.size()")と閾値(100)の小さい方の数だけ繰り返し
        // for (size_t i = 0; i < std::min(lines.size(), size_t(200)); i++)
        // {
        //     float rho = lines[i][0];          // ρ
        //     float theta = lines[i][1];        // θ
        //     double a = cos(theta);            // θからaを計算
        //     double b = sin(theta);            // θからbを計算
        //     double x0 = a * rho;              // 直線上の1点p0(x0, y0)のx0を計算
        //     double y0 = b * rho;              // 直線上の1点p0(x0, y0)のy0を計算
        //     cv::Point p1, p2;                 // 直線描画用の端点p1, p2
        //     p1.x = cvRound(x0 + 1000 * (-b)); // p1のx座標の計算
        //     p1.y = cvRound(y0 + 1000 * (a));  // p1のy座標の計算
        //     p2.x = cvRound(x0 - 1000 * (-b)); // p2のx座標の計算
        //     p2.y = cvRound(y0 - 1000 * (a));  // p2のy座標の計算
        //     // p1とp2を結ぶ線分を描画
        //     cv::line(frameImage, p1, p2, cv::Scalar(0, 0, 255), 2, 8, 0);
        // }
        //(e')検出された円の数("circles.size()")としきい値(200)の小さい方の数だけ繰り返し
        int sum = 0;
        for (int i = 0; i < MIN(circles.size(),200); i++)
        {
            cv::Point3f circle = circles[i];                                               //"circles"から円(x0,y0,r)を1組取り出し
            float x0 = circle.x;                                                           // 円の中心座標(x0,y0)のx座標"x0"
            float y0 = circle.y;                                                           // 円の中心座標(x0,y0)のy座標"y0"
            float r = circle.z;                                                            // 円の半径"r"
            //cv::circle(frameImage, cv::Point(x0, y0), 3, cv::Scalar(0, 255, 0), -1, 8, 0); // 中心点の描画
            cv::circle(frameImage, cv::Point(x0, y0), r, cv::Scalar(0, 0, 255), 2, 8, 0);  // 円の描画

            sum += 1;
        }
        printf("%d\n",sum);

        // (f)"frameImage"，"edgeImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Edge", edgeImage);

        // (g)キー入力待ち
        int key = cv::waitKey(10);
        // [Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
    }

    // ⑥終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
