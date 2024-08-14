//g++ dip14.cpp -std=c++11 -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
#include <iostream>  //入出力関連ヘッダ
#include <GLUT/glut.h>  //OpenGL
#include <math.h>  //数学関数
#include <opencv2/opencv.hpp>  //OpenCV関連ヘッダ

//関数名の宣言
void initGL(void);
void display(void);
void reshape(int w, int h);
void timer(int value);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void initCV(void);
void mouseCallback(int event, int x, int y, int flags, void *userdata);
void onMouseClick(int event, int x, int y);
void transferToCGSpace(std::vector<cv::Point> point);

//グローバル変数
double eDist, eDegX, eDegY;  //視点極座標
int mX, mY, mState, mButton;  //マウス座標
int winW, winH;  //ウィンドウサイズ
double fr = 30.0;  //フレームレート
cv::VideoCapture capture;  //ビデオキャプチャ
cv::Size imageSize;  //画像サイズ
cv::Mat originalImage, frameImage;  //画像格納用
double theta = 0.0;
double delta = 1.0;  // 回転の速度
int rotFlag = 1;  // 回転フラグ

// マウスの座標を配列に保存
std::vector<cv::Point> points;

// 転送後の座標を複数保存
std::vector<std::vector<cv::Point3f>> point3dArray;

// 前のフレームを保存
cv::Mat prevFrame , nowFrame;
std::vector<cv::Point2f> prevFeature, nowFeature;  //前フレームおよび現フレーム特徴点
std::vector<unsigned char> status;  //作業用
std::vector<float> errors;  //作業用
cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::MAX_ITER|cv::TermCriteria::EPS, 20, 0.05);  //反復アルゴリズム停止基準
cv::Mat optImage(imageSize, CV_8UC3);

//main関数
int main(int argc, char* argv[])
{
    //OpenGL初期化
    glutInit(&argc, argv);

    //OpenCV初期設定処理
    initCV();

    //OpenGL初期設定処理
    initGL();
    
    //イベント待ち無限ループ
    glutMainLoop();
    
    return 0;
}

void init(){
    // 前のフレームを初期化
    prevFrame = cv::Mat(imageSize, CV_8UC1);
    nowFrame = cv::Mat(imageSize, CV_8UC1);
}

//OpenCV初期設定処理
void initCV(void)
{
    //①ビデオキャプチャの初期化
    capture = cv::VideoCapture(1);  //カメラ0番をオープン
    if (capture.isOpened()==0) {  //オープンに失敗した場合
        printf("Capture not found\n");
        return;
    }
    
    //②画像格納用インスタンス準備
    int imageWidth=720, imageHeight=405;
    imageSize = cv::Size(imageWidth, imageHeight);  //画像サイズ
    frameImage = cv::Mat(imageSize, CV_8UC3);  //3チャンネル

    //③画像表示用ウィンドウの生成
    cv::namedWindow("Frame");
    cv::moveWindow("Frame", 0, 0);
    
    //マウスコールバック関数のウィンドウへの登録
    cv::setMouseCallback("Frame", mouseCallback);
}

//OpenGL初期設定処理
void initGL(void)
{
    //初期設定
    glutInitWindowSize(600, 400);  //ウィンドウサイズ指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);  //ディスプレイモード設定
    
    //OpenGLウィンドウ生成
    glutInitWindowPosition(imageSize.width, 0);
    glutCreateWindow("GL");
    
    //ウィンドウ消去色設定
    glClearColor(0.9, 0.95, 1.0, 1.0);
    
    //機能有効化
    glEnable(GL_DEPTH_TEST);  //デプスバッファ
    glEnable(GL_NORMALIZE);  //法線ベクトル正規化
    glEnable(GL_LIGHTING);  //陰影付け
    glEnable(GL_LIGHT0);  //光源０

    //光原０の設定
    GLfloat col[4];  //パラメータ(RGBA)
    glEnable(GL_LIGHT0);  //光源0
    col[0] = 0.9; col[1] = 0.9; col[2] = 0.9; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //光源0の鏡面反射の強度
    col[0] = 0.05; col[1] = 0.05; col[2] = 0.05; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);  //光源0の環境光の強度

    //コールバック関数
    glutDisplayFunc(display);  //ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);  //リシェイプコールバック関数の指定
    glutMouseFunc(mouse);  //マウスクリックコールバック関数の指定
    glutMotionFunc(motion);  //マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数の指定
    glutTimerFunc(1000/fr, timer, 0);  //タイマーコールバック関数の指定
    
    //視点極座標初期値
    eDist = 1500; eDegX = 10.0; eDegY = 0.0;
    
    glLineWidth(3.0);
}

//ディスプレイコールバック関数
void display()
{
    // ------------------------------- OpenCV --------------------------------
    //(a)ビデオキャプチャから1フレーム"originalImage"を取り込んで，"frameImage"を生成
    capture >> originalImage;
    //ビデオが終了したら無限ループから脱出
    if (originalImage.data==NULL) {
        exit(0);
    }
    //"originalImage"をリサイズして"frameImage"生成
    cv::resize(originalImage, frameImage, imageSize);

    // 描いた図形を描画（cv::line等で）
    for (int i = 0; i < points.size(); i++) {
        cv::circle(frameImage, points[i], 5, cv::Scalar(0, 0, 255), -1);
        if (i > 0) {
            cv::line(frameImage, points[i], points[i-1], cv::Scalar(0, 0, 255), 2);
        }
    }
    // pointsの中身を出力
    for (int i = 0; i < points.size(); i++) {
        std::cout << "x=" << points[i].x << ", y=" << points[i].y << std::endl;
    }

    // b グレースケール変換
    cv::cvtColor(frameImage, nowFrame, cv::COLOR_BGR2GRAY);

    //(c)"priorImage"から特徴点を抽出して"prevFeature[]"に出力
    cv::goodFeaturesToTrack(prevFrame, prevFeature, 300, 0.01, 10);
    
    // オプティカルフローの計算
    int leftCount = 0;
    int rightCount = 0;
    //(d)オプティカルフロー検出・描画
    int opCnt = prevFeature.size(); //特徴点数
    if (opCnt>0) { //特徴点が存在する場合
        //前フレームの特徴点"prevFeature"から，対応する現フレームの特徴点"presentFeature"を検出
        cv::calcOpticalFlowPyrLK(prevFrame, nowFrame, prevFeature, nowFeature, status, errors, cv::Size(10,10), 4, criteria);
        
        //オプティカルフロー描画
        for(int i=0; i<opCnt; i++){
            cv::Point pt1 = cv::Point(prevFeature[i]); //前フレーム特徴点
            cv::Point pt2 = cv::Point(nowFeature[i]); //現フレーム特徴点

            // xとyの差が大きい場合・小さい場合は無視 sqrt((x1-x2)^2 + (y1-y2)^2)
            double value = std::sqrt(std::pow(pt1.x - pt2.x, 2) + std::pow(pt1.y - pt2.y, 2));
            if (value > 100) {
                continue;
            }
            if (value < 5) {
                continue;
            }

            // y方向の移動距離が大きい場合は無視
            if (std::abs(pt1.y - pt2.y) > 20) {
                continue;
            }

            // 右から左へのオプティカルフローを取得
            if (pt1.x > pt2.x) {
                rightCount++;
            }
            // 左から右へのオプティカルフローを取得
            if (pt1.x < pt2.x) {
                leftCount++;
            }

            // 描画
            cv::line(frameImage, pt1, pt2, cv::Scalar(0, 0, 255), 2);
        }
    }

    // // 右に移動したオプティカルフローの数を出力
    // std::cout << "右に移動したオプティカルフローの数: " << rightCount << std::endl;
    // // 左に移動したオプティカルフローの数を出力
    // std::cout << "左に移動したオプティカルフローの数: " << leftCount << std::endl;

    // どちらに動いているかを出力
    if (rightCount > leftCount && abs(rightCount - leftCount) > 10) {
        std::cout << "右にスワイプしました" << std::endl;

        // クリックした座標を保存
        transferToCGSpace(points);
        // クリックした座標をリセット
        points.clear();

    } else if (rightCount < leftCount && abs(rightCount - leftCount) > 10) {
        std::cout << "左にスワイプしました" << std::endl;
    }



    //(b)"frameImage"の表示
    cv::imshow("Frame", frameImage);

    // フレームを保存
    prevFrame = nowFrame.clone();
    
    // ------------------------------- OpenGL --------------------------------
    GLfloat col[4];  //色設定用
    
    //ウィンドウ内消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    //行列初期化
    glLoadIdentity();
    
    //視点座標の計算
    double ex = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    double ey = eDist*sin(eDegX*M_PI/180.0);
    double ez = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    
    //視点視線の設定
    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //変換行列に視野変換行列を乗算
    
    //光源0の位置指定
    GLfloat pos0[] = {200.0, 700.0, 200.0, 0.0};  //(x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    
    //--------------------  地面  --------------------
    //色設定
    col[0] = 0.5; col[1] = 1.0; col[2] = 0.5;  // (0.5, 1.0, 0.5) : RGB
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);  //拡散反射係数
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0; col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);  //ハイライト係数
    glPushMatrix();  //行列一時保存
    glScaled(1000, 1, 1000);  //拡大縮小
    glutSolidCube(1.0);  //立方体の配置
    glPopMatrix();  //行列復帰
    
    // 図形描画(OpenCVで描画した絵（取得したマウス座標（cv::Point型の軌跡）をcv::Point3f型にする)
    // point3dArrayを線で表示
    for (int i = 0; i < point3dArray.size(); i++) {
        for (int j = 0; j < point3dArray[i].size(); j++) {
            if (j > 0) {
                cv::Point3f p1 = point3dArray[i][j-1];
                cv::Point3f p2 = point3dArray[i][j];
                // 線を描画
                glBegin(GL_LINES);
                glVertex3f(p1.x, p1.y, p1.z);
                glVertex3f(p2.x, p2.y, p2.z);
                glEnd();
            }
        }
    }
    
    //描画実行
    glutSwapBuffers();
}

//タイマーコールバック関数
void timer(int value)
{
    if(rotFlag){
        eDegY += delta;
    }
    
    theta += delta;
    
    glutPostRedisplay();  //ディスプレイイベント強制発生
    glutTimerFunc(1000/fr, timer, 0);  //タイマー再設定
}

//リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);  //ウィンドウ全体が描画対象
    glMatrixMode(GL_PROJECTION);  //投影変換行列を計算対象に設定
    glLoadIdentity();  //行列初期化
    gluPerspective(30.0, (double)w/(double)h, 1.0, 10000.0);  //変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);  //モデルビュー変換行列を計算対象に設定
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    if (state==GLUT_DOWN) {
        //マウス情報格納
        mX = x; mY = y;
        mState = state; mButton = button;
    }
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton==GLUT_RIGHT_BUTTON) {
        //マウスの移動量を角度変化量に変換
        eDegY = eDegY+(mX-x)*0.5;  //マウス横方向→水平角
        eDegX = eDegX+(y-mY)*0.5;  //マウス縦方向→垂直角
    }
    
    //マウス座標格納
    mX = x; mY = y;
}

//キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':
        case 'Q':
        case 27:
            exit(0);
    }
}

// マウスコールバック関数 in a window made by OpenCV
void mouseCallback(int event, int x, int y, int flags, void *userdata)
{
    // マウスの座標を出力
    std::cout << "x=" << x << ", y=" << y << " ";
    
    // イベントの種類を出力
    switch (event) {
        case cv::EVENT_MOUSEMOVE:
            std::cout << "マウスが動いた";
            break;
        case cv::EVENT_LBUTTONDOWN:
            std::cout << "左ボタンを押した";
            onMouseClick(event, x, y);
            break;
        case cv::EVENT_RBUTTONDOWN:
            std::cout << "右ボタンを押した";
            break;
        case cv::EVENT_LBUTTONUP:
            std::cout << "左ボタンを離した";
            break;
        case cv::EVENT_RBUTTONUP:
            std::cout << "右ボタンを離した";
            break;
        case cv::EVENT_RBUTTONDBLCLK:
            std::cout << "右ボタンをダブルクリック";
            break;
        case cv::EVENT_LBUTTONDBLCLK:
            std::cout << "左ボタンをダブルクリック";
            break;
    }
    
    // マウスボタンと特殊キーの押下状態を出力
    std::string str;
    if (flags & cv::EVENT_FLAG_ALTKEY) {
        str += "Alt ";        // ALTキーが押されている
    }
    if (flags & cv::EVENT_FLAG_CTRLKEY) {
        str += "Ctrl ";        // Ctrlキーが押されている
    }
    if (flags & cv::EVENT_FLAG_SHIFTKEY) {
        str += "Shift ";    // Shiftキーが押されている
    }
    if (flags & cv::EVENT_FLAG_LBUTTON) {
        str += "左ボタン ";    // マウスの左ボタンが押されている
    }
    if (flags & cv::EVENT_FLAG_RBUTTON) {
        str += "右ボタン";    // マウスの右ボタンが押されている
    }
    if (!str.empty()) {
        std::cout << "  押下: " << str;
    }
    std::cout << std::endl;
    
}


// クリックした時の座標を取得して保存する
void onMouseClick(int event, int x, int y)
{
    if (event == cv::EVENT_LBUTTONDOWN) {
        // クリックした座標を保存
        points.push_back(cv::Point(x, y));

        // クリックされた座標を出力
        std::cout << "x=" << x << ", y=" << y << std::endl;
    }
}

// CG空間に図形を転送する
void transferToCGSpace(std::vector<cv::Point> point)
{
    std::vector<cv::Point3f> point3d;
    // 変換を行う
    for (int i = 0; i < point.size(); i++) {
        cv::Point3f p = cv::Point3f(point[i].x, point[i].y, 0);
        point3d.push_back(p);
    }

    // 画像サイズを保存
    point3dArray.push_back(point3d);
}


