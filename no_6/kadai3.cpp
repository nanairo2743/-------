#include <iostream>           // 入出力関連ヘッダ
#include <opencv2/opencv.hpp> // OpenCV関連ヘッダ

// レインボーカラーの軌跡を描画するための色を取得する関数
cv::Scalar getRainbowColor(int index)
{
    double hue = index % 180;                              // HSV色空間での色相を計算
    cv::Mat hsv(1, 1, CV_8UC3, cv::Scalar(hue, 255, 255)); // HSV色空間の行列を作成
    cv::Mat bgr;
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR); // HSVからBGR色空間に変換
    return cv::Scalar(bgr.at<cv::Vec3b>(0, 0)[0], bgr.at<cv::Vec3b>(0, 0)[1], bgr.at<cv::Vec3b>(0, 0)[2]);
}

int main(int argc, const char *argv[])
{
    // ビデオファイル"movie.mov"を取り込み
    cv::VideoCapture capture("movie.mov"); // 指定したビデオファイルをオープン
    // ビデオファイルがオープンできたかどうかをチェック
    if (!capture.isOpened())
    {
        printf("Specified video not found\n");
        return -1;
    }

    // フレームの大きさを取得
    int width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("FRAME SIZE = (%d %d)\n", width, height);

    // 画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat hsvImage;
    cv::Mat resultImage;
    cv::Mat grayImage;
    cv::Mat binImage;
    cv::Mat contourImage;

    // 動画ファイルの書き出し準備
    // ビデオライタの生成
    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P', 'I', 'M', '1'), 30, cv::Size(width, height));

    // ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", width, 0);

    // 動画像処理無限ループ：「ビデオキャプチャから1フレーム取り込み」→「画像処理」→「表示」の繰り返し
    while (1)
    {
        // カメラから1フレーム読み込み（ストリーム入力）
        capture >> frameImage;
        if (frameImage.empty())
            break;

        // 色空間変換(BGR -> HSV)
        cv::cvtColor(frameImage, hsvImage, cv::COLOR_BGR2HSV);

        // 色の抽出
        for (int y = 0; y < frameImage.rows; y++)
        {
            for (int x = 0; x < frameImage.cols; x++)
            {
                cv::Vec3b s = hsvImage.at<cv::Vec3b>(y, x);
                // 色相(H)と彩度(S)の値を用いてボール抽出
                if (s[0] > 60 && s[0] < 80 && s[1] > 100)
                {
                }
                else
                {
                    s[0] = 0;
                    s[1] = 0;
                    s[2] = 0;
                }
                hsvImage.at<cv::Vec3b>(y, x) = s;
            }
        }

        // 色空間変換(HSV -> BGR)
        cv::cvtColor(hsvImage, resultImage, cv::COLOR_HSV2BGR);

        // 追加部分：輪郭抽出
        // 画像をグレースケールに変換
        cv::cvtColor(resultImage, grayImage, cv::COLOR_BGR2GRAY);
        // 画像を2値化
        cv::threshold(grayImage, binImage, 50, 255, cv::THRESH_BINARY);
        // 収縮処理
        cv::Mat element = cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(3, 3));
        cv::erode(binImage, binImage, element, cv::Point(-1, -1), 3);
        // 輪郭抽出
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(binImage, contours, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
        // 輪郭描画
        contourImage = resultImage.clone();
        cv::Point center(-1, -1); // 初期値は無効なポイント
        if (!contours.empty())
        {
            std::vector<cv::Point> largestContour = contours[0];
            for (const auto &contour : contours)
            {
                if (cv::contourArea(contour) > cv::contourArea(largestContour))
                {
                    largestContour = contour;
                }
            }

            cv::Moments m = cv::moments(largestContour, false);
            center = cv::Point(m.m10 / m.m00, m.m01 / m.m00);

            // 中心に白い円を描画
            cv::circle(frameImage, center, 10, cv::Scalar(255, 255, 255), -1);
        }


        // ウィンドウへの画像の表示
        cv::imshow("Frame", frameImage);

        // 動画ファイルの書き出し
        rec << frameImage; // ビデオライタに画像出力

        // キー入力待ち
        char key = cv::waitKey(30); // 30ミリ秒待機
        if (key == 'q')
            break;
    }

    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
