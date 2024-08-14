//(OpenCV4) g++ -std=c++11 dip07b.cpp `pkg-config --cflags --libs opencv4`
//(OpenCV3) g++ dip07b.cpp `pkg-config --cflags --libs opencv`
#include <iostream>  //入出力関連ヘッダ
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

int main (int argc, char * argv[])
{
    //①ビデオキャプチャの初期化
    cv::VideoCapture capture("car.mov");  //ビデオファイルをオープン
//    cv::VideoCapture capture(0);  //カメラをオープン
    if (capture.isOpened()==0) {
        printf("Camera not found\n");
        return -1;
    }
    
    //②画像格納用インスタンス準備
    cv::Size imageSize(720, 405);
    cv::Mat originalImage;
    cv::Mat frameImage(imageSize, CV_8UC3);
    cv::Mat optImage(imageSize, CV_8UC3);
    cv::Mat resultImage;

    
    //ビデオライター作成
    cv::VideoWriter rec("rec2.mpg", cv::VideoWriter::fourcc('P','I','M','1'), 30, imageSize);

    
    
    
    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    cv::namedWindow("OpticalFlow");
    cv::moveWindow("OpticalFlow", 50, 0);
    
    //④オプティカルフローに関する初期設定
    cv::Mat priorImage(imageSize, CV_8UC1);  //前フレーム画像
    cv::Mat presentImage(imageSize, CV_8UC1);  //現フレーム画像
    std::vector<cv::Point2f> priorFeature, presentFeature;  //前フレームおよび現フレーム特徴点
    //反復アルゴリズム停止基準
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 20, 0.05);
    std::vector<unsigned char> status;  //作業用
    std::vector<float> errors;  //作業用
    
    //⑤動画表示用無限ループ
    while (1) {
        //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
        capture >> originalImage;
        //ビデオが終了したら無限ループから脱出
        if (originalImage.data==NULL) break;
        //"originalImage"をリサイズして"frameImage"生成
        cv::resize(originalImage, frameImage, imageSize);
        
        //(b)"frameImage"をグレースケール変換して"presentImage"を生成(現フレーム)
        cv::cvtColor(frameImage, presentImage, cv::COLOR_BGR2GRAY);
        
        //(c)"priorImage"から特徴点を抽出して"priorFeature[]"に出力
        cv::goodFeaturesToTrack(priorImage, priorFeature, 300, 0.01, 10);

        

        //(d)オプティカルフロー検出・描画
        int opCnt = priorFeature.size();
        if (opCnt>0) {  //特徴点が存在する場合
            //前フレームの特徴点"priorFeature"から，対応する現フレームの特徴点"presentFeature"を検出
            cv::calcOpticalFlowPyrLK(priorImage, presentImage, priorFeature, presentFeature, status, errors, cv::Size(10,10), 4, criteria);
            
            int ex = 0;
            float distance;
            
            int exvelocity;

            for(int i=0; i<opCnt; i++){

                //オプティカルフロー描画
                cv::Point pt1 = cv::Point(priorFeature[i]);
                cv::Point pt2 = cv::Point(presentFeature[i]);
                
                ex = (pt1.x - pt2.x)*(pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y);
                
                exvelocity = (pt1.x - pt2.x);
                
                distance = sqrt(ex);
                
                if(distance > 15 && distance <50){
                    printf("distance = %lf\n", distance);
                    cv::line(optImage, pt1, pt2, cv::Scalar(0,0,255), 1, 8);
                    
                    if(exvelocity < 0){
                        cv::line(optImage, pt1, pt2, cv::Scalar(255,255,255), 1, 8);
                        cv::line(frameImage, pt1, pt2, cv::Scalar(255,255,255), 1, 8);
                    }
                    else if(distance > 27){
                        cv::line(optImage, pt1, pt2, cv::Scalar(0,0,255), 1, 8);
                        cv::line(frameImage, pt1, pt2, cv::Scalar(0,0,255), 1, 8);
                    }
                    else{
                        cv::line(optImage, pt1, pt2, cv::Scalar(0,255,0), 1, 8);
                        cv::line(frameImage, pt1, pt2, cv::Scalar(0,255,0), 1, 8);
                    }
                }
                
//                //フローベクトルの解析
//                cv::Point optflow = pt2 - pt1;
//                double norm = sqrt(optflow.x*optflow.x + optflow.y*optflow.y);
//
//
//                if(optflow.x > 0){
//                    cv::line(optImage, pt1, pt2, cv::Scalar(255,255,255), 1,8);
//                }
//                else if(optflow.x < 0){
//                    cv::line(optImage, pt1, pt2, cv::Scalar(0,0,255), 1,8);
//
//                }
            }
        }
        
        resultImage = cv::Scalar(0);
        frameImage.copyTo(resultImage, optImage);

        //動画ファイル生成
        rec << frameImage;

        
        //(e)"frameImage"と"resultImage"の表示
        cv::imshow("Frame", frameImage);
        cv::imshow("OpticalFlow", optImage);
//
        
        //(f)現フレームグレースケール画像"presentImage"を前フレームグレースケール画像"priorImage"にコピー
        presentImage.copyTo(priorImage);
        
        //(g)"optImage"をゼロセット
        optImage = cv::Scalar(0);
        
        //(h)キー入力待ち
        int key = cv::waitKey(20);
        //'q'が押されたら無限ループ脱出
        if (key=='q') {
            break;
        }
    }
    
    //⑥終了処理
    //カメラ終了
    capture.release();
    //メッセージを出力して終了
    printf("Finished\n");
    return 0;
}
