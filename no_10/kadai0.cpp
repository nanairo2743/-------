
#include <iostream>  // 入出力関連ヘッダ
#include <opencv2/opencv.hpp>  // OpenCV関連ヘッダ

// 配列の象限入れ替え用関数の宣言
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr);

int main(int argc, const char* argv[]) {
    // ① 原画像のグレースケール画像を"sourceImg"に格納
    cv::Mat sourceImg = cv::imread("nymegami.jpg", cv::IMREAD_GRAYSCALE);
    if (sourceImg.empty()) {  // ファイルが見つからないときはメッセージを表示して終了
        std::cout << "Source not found\n";
        return -1;
    }

    // 作業用配列領域、描画用画像領域の宣言
    cv::Mat cxMatrix(sourceImg.size(), CV_64FC2);  // 複素数用(実数 2 チャンネル)
    cv::Mat ftMatrix(sourceImg.size(), CV_64FC2);  // 複素数用(実数 2 チャンネル)
    cv::Mat spcMatrix(sourceImg.size(), CV_64FC1);  // スペクトルデータ(実数)
    cv::Mat spcImg(sourceImg.size(), CV_8UC1);  // スペクトル画像(自然数)
    cv::Mat resultImg(sourceImg.size(), CV_8UC1);  // 逆変換画像(自然数)

    // ウィンドウ生成
    cv::namedWindow("sourceImg");
    cv::moveWindow("sourceImg", 0, 0);
    cv::namedWindow("spcImg");
    cv::moveWindow("spcImg", sourceImg.cols, 0);
    cv::namedWindow("Result");
    cv::moveWindow("Result", sourceImg.cols * 2, 0);

    // ② 原画像を複素数(実数部と虚数部)の 2 チャンネル配列(画像)として表現. 虚数部はゼロ
    cv::Mat imgMatrix[] = {cv::Mat_<double>(sourceImg), cv::Mat::zeros(sourceImg.size(), CV_64FC1)};
    cv::merge(imgMatrix, 2, cxMatrix);

    // ③ フーリエ変換
    cv::dft(cxMatrix, ftMatrix);
    ShiftDFT(ftMatrix, ftMatrix);

    cv::Vec2d s(0.0, 0.0); 
    int mx = ftMatrix.cols/2.0;
    int my = ftMatrix.rows/2.0;
    for (int y = 0; y < ftMatrix.rows; y++)
    {
        for (int x = 0; x < ftMatrix.cols; x++)
        {
            int r = sqrt(pow(abs(x - mx),2) + pow(abs(y - my),2));

            if(r < 30){
                ftMatrix.at<cv::Vec2d>(y, x) = s;
            }
        }
    }


    // ④ フーリエスペクトル"spcMatrix"の計算
    cv::split(ftMatrix, imgMatrix);
    cv::magnitude(imgMatrix[0], imgMatrix[1], spcMatrix);

    // ⑤ フーリエスペクトルからフーリエスペクトル画像を生成
    spcMatrix += cv::Scalar::all(1);
    cv::log(spcMatrix, spcMatrix);
    cv::normalize(spcMatrix, spcImg, 0, 255, cv::NORM_MINMAX, CV_8U);

    // ⑥ フーリエ逆変換
    ShiftDFT(ftMatrix, ftMatrix);
    cv::idft(ftMatrix, cxMatrix);
    cv::split(cxMatrix, imgMatrix);
    cv::normalize(imgMatrix[0], resultImg, 0, 255, cv::NORM_MINMAX, CV_8U);

    // ⑦ 各画像を表示
    cv::imshow("sourceImg", sourceImg);
    cv::imshow("spcImg", spcImg);
    cv::imshow("Result", resultImg);

    // ⑧ キー入力されたら，ウィンドウと画像格納用領域を破棄して終了
    cv::waitKey(0);

    return 0;
}

// 画像の象限入れ替え用関数
void ShiftDFT(const cv::Mat& src_arr, cv::Mat& dst_arr) {
    int cx = src_arr.cols / 2;
    int cy = src_arr.rows / 2;

    cv::Mat q1(src_arr, cv::Rect(cx, 0, cx, cy));
    cv::Mat q2(src_arr, cv::Rect(0, 0, cx, cy));
    cv::Mat q3(src_arr, cv::Rect(0, cy, cx, cy));
    cv::Mat q4(src_arr, cv::Rect(cx, cy, cx, cy));

    cv::Mat tmp;
    q1.copyTo(tmp);
    q3.copyTo(q1);
    tmp.copyTo(q3);

    q2.copyTo(tmp);
    q4.copyTo(q2);
    tmp.copyTo(q4);
}
