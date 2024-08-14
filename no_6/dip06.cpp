#include <iostream>           //入出力関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

int main(int argc, const char *argv[])
{
    // ①ルートディレクトリの画像ファイル"col.jpg"を読み込んで"sourceImage"に格納
    cv::Mat sourceImage = cv::imread("col.jpg", cv::IMREAD_COLOR);
    if (sourceImage.data == 0)
    { // 画像ファイルが読み込めなかった場合
        printf("File not found\n");
        exit(0);
    }
    printf("Width=%d, Height=%d\n", sourceImage.cols, sourceImage.rows);

    // ②画像格納用インスタンスの生成
    cv::Mat grayImage(sourceImage.size(), CV_8UC1);    // グレースケール画像用（1チャンネル）
    cv::Mat binImage(sourceImage.size(), CV_8UC1);     // ２値画像用（1チャンネル）
    cv::Mat contourImage(sourceImage.size(), CV_8UC3); // 輪郭表示画像用（3チャンネル）

    // 3原画像をグレースケール画像に、グレースケール画像を2値画像に変換
    //"sourceImage"をグレースケール画像に変換して"grayImage"に出力
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);
    //"grayImage"を 2 値化して"grayImage"に出力
    cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY);
    //"sourceImage"のコピーを"contourImage"に出力
    contourImage = sourceImage.clone();
    // 構造要素"element"の作成(3×3，十字型)
    cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3)); // 収縮処理(構造要素"element"を3回適用)
    cv::erode(binImage, binImage, element, cv::Point(-1, -1), 3);
    // ④輪郭点格納用配列、輪郭の階層用配列の確保
    // 輪郭点格納用配列
    std::vector<std::vector<cv::Point>> contours;

    // ⑤"binImage"からの輪郭抽出処理
    cv::Mat tmpImage = binImage.clone();
    // 輪郭抽出処理.輪郭ごとに輪郭画素位置を"counter"に格納
    cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
    // 輪郭データ"contour"を順次描画
    for (int i = 0; i < contours.size(); i++)
    {
        // 輪郭線の長さを計算
        double length = cv::arcLength(contours[i], true);
        // 輪郭内部の面積を計算
        double area = cv::contourArea(contours[i]);
        cv::Point p = contours[i][0];
        // コンソールに表示
        printf("Length=%f, Area=%f, Start=(%d, %d)\n", length, area, p.x, p.y);
        cv::drawContours(contourImage, contours, i, cv::Scalar(255, 255, 255), 2, 8);
        // 輪郭開始点に円を描画(描画先画像，中心座標，半径，色，線の太さ)
        cv::circle(contourImage, p, 5, CV_RGB(255, 0, 0), 1);
        cv::drawContours(contourImage, contours, i, cv::Scalar(255, 255, 255), 2, 8);
    }

    // ⑥ウィンドウを生成して各画像を表示
    // 原画像
    cv::namedWindow("Source");         // ウィンドウの生成
    cv::moveWindow("Source", 0, 50);   // ウィンドウの表示位置の指定
    cv::imshow("Source", sourceImage); // ウィンドウに画像を表示
    // グレースケール(2値化)
    cv::namedWindow("Gray");         // ウィンドウの生成
    cv::moveWindow("Gray", 150, 50); // ウィンドウの表示位置の指定
    cv::imshow("Gray", grayImage);   // ウィンドウに画像を表示
    // 輪郭画像(原画像に輪郭を追加)
    cv::namedWindow("Contour");          // ウィンドウの生成
    cv::moveWindow("Contour", 300, 50);  // ウィンドウの表示位置の指定
    cv::imshow("Contour", contourImage); // ウィンドウに画像を表示

    // ⑦キー入力があるまでここでストップ
    cv::waitKey(0);

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
