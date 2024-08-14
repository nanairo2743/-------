#include <iostream>
#include <opencv2/opencv.hpp>

int main(int argc, char *argv[])
{
    // ①ビデオキャプチャの初期化
    cv::VideoCapture capture("walking.mov"); // ビデオファイルをオープン
    if (capture.isOpened() == 0)
    {
        printf("Camera not found\n");
        return -1;
    }

    // ②画像格納用インスタンス準備
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat backImage(imageSize, CV_8UC3);
    cv::Mat subImage(imageSize, CV_8UC3);
    cv::Mat subBinImage(imageSize, CV_8UC1);
    cv::Mat resultImage(imageSize, CV_8UC3);

    // ③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Back");
    cv::moveWindow("Back", 50, 50);
    cv::namedWindow("Subtraction");
    cv::moveWindow("Subtraction", 100, 100);
    cv::namedWindow("Result");
    cv::moveWindow("Result", 150, 150);

    //ビデオライタ生成(ファイル名，コーデック，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec.mpg", cv::VideoWriter::fourcc('P','I','M','1'), 30, cv::Size(720, 405));

    // ④動画処理用無限ループ
    while (1)
    {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        // ビデオが終了したら無限ループから脱出
        if (originalImage.data == NULL)
            break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);

        //(b)"frameImage"と"backImage"との差分画像"subImage"の生成
        cv::absdiff(frameImage, backImage, subImage);
        //(b')"subImage"をグレースケール変換→しきい値処理した画像"subBinImage"を生成 
        cv::cvtColor(subImage, subBinImage, cv::COLOR_BGR2GRAY); 
        cv::threshold(subBinImage, subBinImage, 30, 255, cv::THRESH_BINARY);
        int count = 0;

        //(b")"frameImage"を"subBinImage"マスク付きで"resultImage"にコピー
        resultImage = cv::Scalar(0);
        frameImage.copyTo(resultImage, subBinImage);

        // 動く物体のピクセル数をカウント
        for (int y = 0; y < subBinImage.rows; y++)
        {
            for (int x = 0; x < subBinImage.cols; x++)
            {
                uchar s = subBinImage.at<uchar>(y, x);
                if (s != 0)
                {
                    count++;
                }
            }
        }

        // カウントに基づいて赤い円を描画
        if(count > 30) {
            int radius = count / 100; // ピクセル数に基づいて半径を決定
            cv::circle(resultImage, cv::Point(imageSize.width / 2, imageSize.height / 2), radius, cv::Scalar(0, 0, 255), 2);
        }

        //(c)"frameImage"，"backImage"，"subImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Back", backImage);
        cv::imshow("Subtraction", subBinImage);
        cv::imshow("Result", resultImage);


        //(d)"frameImage"で"backImage"を更新
        // frameImage.copyTo(backImage);

        //(動画ファイル書き出し
        rec << resultImage;  //ビデオライタに画像出力

        //(e)キー入力待ち
        int key = cv::waitKey(20);
        //[Q]が押されたら無限ループ脱出
        if (key == 'q')
            break;
        if (key == 'c')
            frameImage.copyTo(backImage);
    }

    // ⑤終了処理
    // カメラ終了
    capture.release();
    // メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
