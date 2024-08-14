//(OpenCV4) g++ dip08_021.cpp -std=c++11 `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07-2.cpp `pkg-config --cflags --libs opencv`

#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char *argv[])
{
    //ビデオキャプチャを初期化して，映像を取り込む
    cv::VideoCapture capture("water1.mov");  //指定したビデオファイルをオープン
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    //フレームサイズ取得
    int width = capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = capture.get(cv::CAP_PROP_FRAME_HEIGHT);
    printf("Frame Size = (%d %d)\n", width, height);
    cv::Size imageSize(width, height);  //フレームと同じ画像サイズ定義
    
    //船画像"face.jpg"の読み込み
    cv::Mat shipImage = cv::imread("ship.jpg", cv::IMREAD_COLOR);
    cv::Mat greenImage(cv::Size(width,height),CV_8UC3);
    cv::Point2f original[4], translate[4];
    original[0] = cv::Point2f(0,0);
    original[1] = cv::Point2f(shipImage.cols,0);
    original[2] = cv::Point2f(shipImage.cols,shipImage.rows);
    original[3] = cv::Point2f(0,shipImage.rows);
    
    //画像格納用インスタンス準備
    cv::Mat frameImage;
    cv::Mat recImage = cv::Mat(cv::Size(width/2, height/2), CV_8UC3);
    
    //オプティカルフロー準備
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 30, 0.01);  //終了条件
    cv::Mat presentImage(imageSize, CV_8UC1), priorImage(imageSize, CV_8UC1);  //現フレーム濃淡画像，前フレーム濃淡画像
    std::vector<cv::Point2f> presentFeature, priorFeature;  //現フレーム対応点，前フレーム追跡点
    std::vector<unsigned char> status;  //処理用
    std::vector<float> errors;  //処理用
    
    //船の初期位置
    cv::Point2f shipPoint(130, 190);
    
    //ウィンドウの生成と移動
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("Ship");
    cv::moveWindow("Ship", width, 0);
    
    //船画像の表示
    cv::imshow("Ship", shipImage);
    
    //追跡点の設定（適当に決めた5つの点）
//    priorFeature.push_back(cv::Point2f(width/2.0-20, height/2.0));
//    priorFeature.push_back(cv::Point2f(width/2.0-10, height/2.0));
//    priorFeature.push_back(cv::Point2f(width/2.0, height/2.0));
//    priorFeature.push_back(cv::Point2f(width/2.0+10, height/2.0));
//    priorFeature.push_back(cv::Point2f(width/2.0+20, height/2.0));
    
    
    for(int j=0; j<height; j=j+4){
        for(int i=0;i<width; i=i+4){
            priorFeature.push_back(cv::Point2f(i, j));
        }
    }
    //ビデオライタ生成(ファイル名，コーデック(mp4v/mov)，フレームレート，フレームサイズ)
    cv::VideoWriter rec("rec3.mov", cv::VideoWriter::fourcc('m','p','4','v'), 20, cv::Size(width, height));
    
    //動画像処理無限ループ
    int fid = 0;
    while (1) {
        //===== カメラから1フレーム読み込み =====
        capture >> frameImage;
        if(frameImage.data == NULL) break;
        
        //===== オプティカルフロー =====
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);  //現フレーム濃淡画像"presentImage"を生成
        //cv::goodFeaturesToTrack(priorImage, priorFeature, 1000, 0.01, 1);  //前フレーム追跡点"priorFeature"生成
        int opCnt = priorFeature.size();  //追跡点の個数
        //オプティカルフローの計算と描画
        cv::Point2f averageV(0,0);
        int count = 0;
        if (opCnt>0) {  //追跡点が存在する場合
            //"priorImage"と"presentImage"を用いて，追跡点"priorFeature"に対応する現フレーム点"presentFeature"を取得
            cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10,10), 4, criteria);
            //オプティカルフロー描画
            for(int i=0; i<opCnt; i++){
                cv::Point pt1 = cv::Point(priorFeature[i]);  //前フレーム追跡点
                cv::Point pt2 = cv::Point(presentFeature[i]);  //現フレーム対応点
                cv::Point2f nagareV=pt2-pt1;//(一本のベクトル)
                double len= sqrt(pow(pt1.x-pt2.x,2)+pow(pt1.y-pt2.y,2));//powは２乗
                double len2= sqrt(pow(pt1.x-shipPoint.x,2)+pow(pt1.y-shipPoint.y,2));
                if(len>2 && len<30 && len2<20){
                    //cv::line(frameImage, pt1, pt2, cv::Scalar(0,0,255), 2, 8);
                    averageV += nagareV;
                    count++;
                }
            }
        }
        if(count>0){
        averageV/= count;
        }
        cv::line(frameImage, shipPoint, shipPoint+averageV, cv::Scalar(255,0,0), 2, 8);
        
        shipPoint += averageV;
        translate[0] = cv::Point2f(shipPoint.x-shipImage.cols/2,shipPoint.y-shipImage.rows/2);
        translate[1] = cv::Point2f(shipPoint.x+shipImage.cols/2,shipPoint.y-shipImage.rows/2);
        translate[2] = cv::Point2f(shipPoint.x+shipImage.cols/2,shipPoint.y+shipImage.rows/2);
        translate[3] = cv::Point2f(shipPoint.x-shipImage.cols/2,shipPoint.y+shipImage.rows/2);
        presentImage.copyTo(priorImage);  //"priorImage"を"presentImage"で更新
        
        cv::Mat persMat = cv::getPerspectiveTransform(original, translate);
        
        
        cv::warpPerspective(shipImage,greenImage, persMat, frameImage.size(),
                            cv::INTER_LINEAR, cv::BORDER_CONSTANT,cv::Scalar(0,255,0));
        double theta=0.0;
        double len3=sqrt(pow(averageV.x,2)+pow(averageV.y,2));
        
        if(averageV.x>0 && len3>3){
            theta=-atan(averageV.y/averageV.x)*180.0/M_PI;
            if(theta>20){
                theta=20;
            }else if(theta<-20){
                theta=-20;
            }
            cv::Mat rotateMat = cv::getRotationMatrix2D(shipPoint, theta*0.5, 1.0); //行列生成
            cv::warpAffine(greenImage, greenImage, rotateMat, greenImage.size(),
                           cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(0,255,0));

        }
        presentImage.copyTo(priorImage);
        
        for(int j=0;j<height;j++){
            for(int i=0;i<width;i++){
                cv::Vec3b s=greenImage.at<cv::Vec3b>(j,i);
                if(s[0]<20 && s[1]>235 && s[2]<20){
                    continue;
                }
                frameImage.at<cv::Vec3b>(j,i) = s;
            }
        }
        
        //船の位置に円を表示
        //cv::circle(frameImage, shipPoint, 5, cv::Scalar(0,255,0), -1, 8);
        
        //ウィンドウに画像表示
        cv::imshow("Frame", frameImage);
        cv::imshow("Gerrn", greenImage);
        
        //キー入力待ち
        char key = cv::waitKey(1);  //20ミリ秒待機
        if(key == 'q') break;
        
        //動画ファイル書き出し
        rec << frameImage;  //ビデオライタに画像出力
    }
    
    //メッセージを出力して終了
    printf("Finished\n");q
    return 0;
}

