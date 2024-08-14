//(OpenCV4) g++ -std=c++11 kadai1.cpp -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv4` -Wno-deprecated
//(OpenCV3) g++ dip15b.cpp -framework OpenGL -framework GLUT `pkg-config --cflags --libs opencv` -Wno-deprecated

/*
 （ヒント）
 ０．各パーツを大雑把にモデリング
        各パーツを描画する関数を作ると良い（細かいモデリングをした時にdisplay関数を見やすくするため）．
 １．画像座標系からCG座標系への座標変換を立式する
 　　そして，画像↔︎CG座標変換を行う関数image2cg()を作る
        cv::Rect image2cg(cv::Rect p)
            入力は画像座標p，戻り値はCG座標．
            cv::Rect pは矩形の左上座標(p.x,p.y)と矩形のサイズ（p.with, p.height)をメンバに持つ
            x, yが座標値なのでこれを変換する．width, heightはそのままで良い
 ２．今はCGの顔の位置は固定だが，CGの顔の位置が認識した顔の位置になるように（連動して動くように）してみよう．
 ３．連動できたら，他のパーツも同じようにする
 ４．顔を傾けたらCGの顔も傾くようにする
    左右の眼の位置関係に着目すれば，何度回転しているか分かる．タンジェントが使えそう．
    ただし，左右の眼はそれぞれ1個のみ認識されるように工夫すること
 ５．各パーツを丁寧にモデリングし直し
 */

#include <iostream>           //入出力関連ヘッダ
#include <GLUT/glut.h>        //OpenGL関連ヘッダ
#include <opencv2/opencv.hpp> //OpenCV関連ヘッダ

// 関数名の宣言
void initCV(void);     // OpenCVの初期化
void initGL(void);     // OpenGLの初期化
void display(void);    // 描画関数
void timer(int value); // タイマー関数
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);

// グローバル変数
double eDist, eDegX, eDegY;  // 視点極座標
int mX, mY, mState, mButton; // マウス座標
int winW, winH;              // ウィンドウサイズ
double fr = 30.0;            // フレームレート
cv::VideoCapture capture;    // 映像キャプチャ用変数
cv::Mat originalImage, frameImage, hsvImage, tempImage;
cv::Size imageSize(640, 360);

cv::CascadeClassifier faceClassifier;      // 顔認識用分類器
cv::CascadeClassifier left_eyeClassifier;  // 左目認識用分類器
cv::CascadeClassifier right_eyeClassifier; // 右目認識用分類器
cv::CascadeClassifier mouthClassifier;     // 口認識用分類器
cv::CascadeClassifier noseClassifier;      // 鼻認識用分類

int halfWidth = 200, halfHeight = 100;

cv::Point facePoint;

class Face
{
public:
    cv::Rect faceRect;
    cv::Rect leftEyeRect;
    cv::Rect rightEyeRect;
    cv::Rect mouthRect;
    cv::Rect noseRect;
    double rotate;
    double scale;
};

Face faceStatus;

void drawFace()
{
    GLfloat col[4];
    // 顔の色設定
    col[0] = 1.0;
    col[1] = 0.8;
    col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col);
    glMaterialfv(GL_FRONT, GL_AMBIENT, col);
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);

    // 顔の描画
    glPushMatrix();
    glScaled(250.0, 250.0, 60.0);
    glutSolidCube(1.0);
    glPopMatrix();
}

void drawEyes()
{
    // 目の描画（簡単な球体としてモデリング）
    glPushMatrix();
    glTranslated(-50.0, 50.0, 125.0); // 左目の位置
    glutSolidSphere(30.0, 20, 20);
    glTranslated(100.0, 0.0, 0.0); // 右目の位置
    glutSolidSphere(30.0, 20, 20);
    glPopMatrix();
}

void drawNose()
{
    // 鼻の描画（簡単な円錐としてモデリング）
    glPushMatrix();
    glTranslated(0.0, 0.0, 130.0);
    glRotated(-90, 1.0, 0.0, 0.0);
    glutSolidCone(20.0, 50.0, 20, 20);
    glPopMatrix();
}

void drawMouth()
{
    // 口の描画（簡単な円としてモデリング）
    glPushMatrix();
    glTranslated(0.0, -50.0, 120.0);
    glScaled(50.0, 20.0, 1.0);
    glutSolidSphere(1.0, 20, 20);
    glPopMatrix();
}

cv::Rect image2cg(cv::Rect p)
{
    // 画像座標をCG座標に変換
    cv::Rect cgRect;
    cgRect.x = (p.x - imageSize.width / 2) * (imageSize.width);
    cgRect.y = (imageSize.height / 2 - p.y) * (imageSize.height);
    cgRect.width = p.width;
    cgRect.height = p.height;
    return cgRect;
}

// main関数
int main(int argc, char *argv[])
{
    // OpenGL初期化
    glutInit(&argc, argv);

    // OpenCV初期設定処理
    initCV();

    // OpenGL初期設定処理
    initGL();

    // イベント待ち無限ループ
    glutMainLoop();

    return 0;
}

// OpenCV初期設定処理
void initCV()
{
    // カメラキャプチャの初期化
    capture = cv::VideoCapture(0);
    if (capture.isOpened() == 0)
    {
        // カメラが見つからないときはメッセージを表示して終了
        printf("Camera not found\n");
        exit(1);
    }
    capture >> originalImage;
    cv::resize(originalImage, frameImage, imageSize);

    // OpenCVウィンドウ生成
    cv::namedWindow("Frame");

    // 顔検出器の読み込み
    faceClassifier.load("haarcascade_frontalface_default.xml");
    left_eyeClassifier.load("haarcascade_lefteye_2splits.xml");
    right_eyeClassifier.load("haarcascade_righteye_2splits.xml");
    mouthClassifier.load("haarcascade_mcs_mouth.xml");
    noseClassifier.load("haarcascade_mcs_nose.xml");
}

// OpenGL初期設定処理
void initGL(void)
{
    // 初期設定
    glutInitWindowSize(imageSize.width, imageSize.height);     // ウィンドウサイズ指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE); // ディスプレイモード設定

    // OpenGLウィンドウ生成
    glutCreateWindow("CG");

    // ウィンドウ消去色設定
    glClearColor(0.0, 0.0, 0.2, 1.0);

    // 機能有効化
    glEnable(GL_DEPTH_TEST); // デプスバッファ
    glEnable(GL_NORMALIZE);  // 法線ベクトル正規化
    glEnable(GL_LIGHTING);   // 陰影付け

    // 光原設定
    GLfloat col[4];      // パラメータ(RGBA)
    glEnable(GL_LIGHT0); // 光源0
    col[0] = 0.9;
    col[1] = 0.9;
    col[2] = 0.9;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  // 光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col); // 光源0の鏡面反射の強度
    col[0] = 0.05;
    col[1] = 0.05;
    col[2] = 0.05;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col); // 光源0の環境光の強度

    // コールバック関数
    glutDisplayFunc(display);           // ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);           // リシェイプコールバック関数の指定
    glutMouseFunc(mouse);               // マウスクリックコールバック関数の指定
    glutMotionFunc(motion);             // マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);         // キーボードコールバック関数の指定
    glutTimerFunc(1000 / fr, timer, 0); // タイマーコールバック関数の指定

    // 視点極座標初期値
    eDist = 1200;
    eDegX = 0.0;
    eDegY = 0.0;
}

// ディスプレイコールバック関数
void display(void)
{
    //------------------------------CV------------------------------
    // ビデオキャプチャから1フレーム画像取得
    capture >> originalImage;
    cv::resize(originalImage, frameImage, imageSize);

    // 検出情報を受け取るための配列を用意する
    std::vector<cv::Rect> faces, left_eye, right_eye, mouth, nose;

    // 画像中から検出対象の情報を取得する
    faceClassifier.detectMultiScale(frameImage, faces, 1.1, 3, 0, cv::Size(20, 20)); // 顔

    // 顔の画像を切り出し
    if (faces.size() > 0)
    {
        cv::Rect faceRect = faces[0];
        // 画像範囲内に収まるように調整
        faceRect = faceRect & cv::Rect(0, 0, frameImage.cols, frameImage.rows);

        cv::Mat faceImage = frameImage(faceRect);
        facePoint = cv::Point(faceRect.x, faceRect.y);

        // 顔画像の右上の4分の1を取得
        cv::Rect faceRightRect = cv::Rect(0, 0, faceRect.width / 2, faceRect.height / 2);
        faceRightRect = faceRightRect & cv::Rect(0, 0, faceImage.cols, faceImage.rows);

        // 顔画像の左上の4分の1を取得
        cv::Rect faceLeftRect = cv::Rect(faceRect.width / 2, 0, faceRect.width / 2, faceRect.height / 2);
        faceLeftRect = faceLeftRect & cv::Rect(0, 0, faceImage.cols, faceImage.rows);

        cv::Mat faceRightImage = faceImage(faceRightRect);
        cv::Mat faceLeftImage = faceImage(faceLeftRect);

        left_eyeClassifier.detectMultiScale(faceLeftImage, left_eye, 1.1, 3, 0, cv::Size(10, 10));    // 左目
        right_eyeClassifier.detectMultiScale(faceRightImage, right_eye, 1.1, 3, 0, cv::Size(10, 10)); // 右目
        mouthClassifier.detectMultiScale(faceImage, mouth, 1.1, 3, 0, cv::Size(10, 10));              // 口
        noseClassifier.detectMultiScale(faceImage, nose, 1.1, 3, 0, cv::Size(10, 10));                // 鼻

        // 顔
        for (int i = 0; i < faces.size(); i++)
        {
            // 検出情報から位置情報を取得
            cv::Rect face = faces[i];
            // 大きさによるチェック。
            if (face.width * face.height < 100 * 100)
            {
                continue; // 小さい矩形は採用しない
            }
            // 取得した位置情報に基づき矩形描画
            cv::rectangle(frameImage, cv::Point(face.x, face.y), cv::Point(face.x + face.width, face.y + face.height), CV_RGB(255, 0, 0), 2, 8);
            faceStatus.faceRect = cv::Rect(
                face.x - halfWidth,
                face.y * -1 + halfHeight,
                face.width,
                face.height);
        }

        // 左目
        for (int i = 0; i < left_eye.size(); i++)
        {
            // 検出情報から位置情報を取得
            cv::Rect leftEye = left_eye[i];
            // 大きさによるチェック。
            if (leftEye.width * leftEye.height < 10 * 10)
            {
                continue; // 小さい矩形は採用しない
            }
            // 取得した位置情報に基づき矩形描画
            cv::Point leftUpPoint = cv::Point(
                leftEye.x + faceLeftRect.x + faceRect.x,
                leftEye.y + faceLeftRect.y + faceRect.y);
            cv::Point rightDownPoint = cv::Point(
                leftEye.x + leftEye.width + faceLeftRect.x + faceRect.x,
                leftEye.y + leftEye.height + faceLeftRect.y + faceRect.y);

            cv::rectangle(frameImage, leftUpPoint, rightDownPoint, CV_RGB(0, 0, 255), 2, 8);
            faceStatus.leftEyeRect = cv::Rect(
                leftUpPoint.x - halfWidth - 50,
                leftUpPoint.y * -1 + halfHeight + 60,
                rightDownPoint.x - leftUpPoint.x,
                rightDownPoint.y - leftUpPoint.y);
        }

        // 右目
        for (int i = 0; i < right_eye.size(); i++)
        {
            // 検出情報から位置情報を取得
            cv::Rect rightEye = right_eye[i];
            // 大きさによるチェック。
            if (rightEye.width * rightEye.height < 10 * 10)
            {
                continue; // 小さい矩形は採用しない
            }
            // 取得した位置情報に基づき矩形描画
            cv::Point leftUpPoint = cv::Point(
                rightEye.x + faceRightRect.x + faceRect.x,
                rightEye.y + faceRightRect.y + faceRect.y);
            cv::Point rightDownPoint = cv::Point(
                rightEye.x + rightEye.width + faceRightRect.x + faceRect.x,
                rightEye.y + rightEye.height + faceRightRect.y + faceRect.y);

            cv::rectangle(frameImage, leftUpPoint, rightDownPoint, CV_RGB(0, 255, 255), 2, 8);

            faceStatus.rightEyeRect = cv::Rect(
                leftUpPoint.x - halfWidth - 90,
                leftUpPoint.y * -1 + halfHeight + 60,
                rightDownPoint.x - leftUpPoint.x,
                rightDownPoint.y - leftUpPoint.y);
        }

        // 顔の角度
        if (left_eye.size() > 0 && right_eye.size() > 0)
        {
            cv::Point leftEyeCenter = cv::Point(
                faceStatus.leftEyeRect.x + faceStatus.leftEyeRect.width / 2,
                faceStatus.leftEyeRect.y + faceStatus.leftEyeRect.height / 2);
            cv::Point rightEyeCenter = cv::Point(
                faceStatus.rightEyeRect.x + faceStatus.rightEyeRect.width / 2,
                faceStatus.rightEyeRect.y + faceStatus.rightEyeRect.height / 2);

            double dx = rightEyeCenter.x - leftEyeCenter.x;
            double dy = rightEyeCenter.y - leftEyeCenter.y;
            double rad = atan2(dy, dx);
            double deg = rad * 180 / M_PI;
            faceStatus.rotate = deg;
        }

        if (faceRect.width > 200)
        {
            faceStatus.scale = faceRect.width - 200;
        }
    }

    // フレーム画像表示
    cv::imshow("Frame", frameImage);

    //------------------------------CG------------------------------
    GLfloat col[4]; // 色設定用

    // ウィンドウ内消去
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 行列初期化
    glLoadIdentity();

    // 視点座標の計算
    double ex = eDist * cos(eDegX * M_PI / 180.0) * sin(eDegY * M_PI / 180.0);
    double ey = eDist * sin(eDegX * M_PI / 180.0);
    double ez = eDist * cos(eDegX * M_PI / 180.0) * cos(eDegY * M_PI / 180.0) + faceStatus.scale * -20;

    // 視点視線の設定
    gluLookAt(ex, ey, ez, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0); // 変換行列に視野変換行列を乗算

    // 光源0の位置指定
    GLfloat pos0[] = {200.0, 700.0, 200.0, 1.0}; //(x, y, z, 0(平行光源)/1(点光源))
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);

    //--------------------顔--------------------
    // 色設定
    col[0] = 1.0;
    col[1] = 0.8;
    col[2] = 0.5;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col); // 拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col); // 環境光反射係数
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);                         // ハイライト係数
    glPushMatrix();                                                  // 行列一時保存
    glTranslated(faceStatus.faceRect.x, faceStatus.faceRect.y, 0.0); // 平行移動
    glScaled(250.0, 250.0, 60.0);                                    // 拡大縮小
    glRotated(faceStatus.rotate, 0.0, 0.0, 1.0);
    glutSolidCube(1.0); // 立方体の配置
    glPopMatrix();
    // 行列復帰
    //--------------------左目--------------------
    // 色設定
    col[0] = 0.0;
    col[1] = 0.0;
    col[2] = 0.0;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col); // 拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col); // 環境光反射係数
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);                                 // ハイライト係数
    glPushMatrix();                                                          // 行列一時保存
    glTranslated(faceStatus.leftEyeRect.x, faceStatus.leftEyeRect.y, 100.0); // 平行移動
    glScaled(15.0, 15.0, 15.0);                                              // 拡大縮小
    glutSolidSphere(1.0, 20, 20);                                            // 球の配置
    glPopMatrix();                                                           // 行列復帰

    // 色設定
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col); // 拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col); // 環境光反射係数
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);                               // ハイライト係数
    glPushMatrix();                                                        // 行列一時保存
    glTranslated(faceStatus.leftEyeRect.x, faceStatus.leftEyeRect.y, 0.0); // 平行移動
    glRotated(faceStatus.rotate, 0.0, 0.0, 1.0);
    glScaled(50.0, 25.0, 50.0);   // 拡大縮小
    glutSolidSphere(1.0, 20, 20); // 球の配置
    glPopMatrix();

    //--------------------右目--------------------
    // 色設定
    col[0] = 0.0;
    col[1] = 0.0;
    col[2] = 0.0;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col); // 拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col); // 環境光反射係数
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);                                   // ハイライト係数
    glPushMatrix();                                                            // 行列一時保存
    glTranslated(faceStatus.rightEyeRect.x, faceStatus.rightEyeRect.y, 100.0); // 平行移動
    glScaled(15.0, 15.0, 15.0);                                                // 拡大縮小
    glutSolidSphere(1.0, 20, 20);                                              // 球の配置
    glPopMatrix();                                                             // 行列復帰

    // 色設定
    col[0] = 1.0;
    col[1] = 1.0;
    col[2] = 1.0;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, col); // 拡散反射係数
    glMaterialfv(GL_FRONT, GL_AMBIENT, col); // 環境光反射係数
    col[0] = 0.5;
    col[1] = 0.5;
    col[2] = 0.5;
    col[3] = 1.0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, col);
    glMaterialf(GL_FRONT, GL_SHININESS, 64);                                 // ハイライト係数
    glPushMatrix();                                                          // 行列一時保存
    glTranslated(faceStatus.rightEyeRect.x, faceStatus.rightEyeRect.y, 0.0); // 平行移動
    glRotated(faceStatus.rotate, 0.0, 0.0, 1.0);
    glScaled(50.0, 25.0, 50.0);   // 拡大縮小
    glutSolidSphere(1.0, 20, 20); // 球の配置
    glPopMatrix();                // 行列復帰

    // 描画実行
    glutSwapBuffers();
}

// タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();                // ディスプレイイベント強制発生
    glutTimerFunc(1000 / fr, timer, 0); // タイマー再設定
}

// リサイズコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);                                    // ウィンドウ全体が描画対象
    glMatrixMode(GL_PROJECTION);                               // 投影変換行列を計算対象に設定
    glLoadIdentity();                                          // 行列初期化
    gluPerspective(30.0, (double)w / (double)h, 1.0, 10000.0); // 変換行列に透視投影を乗算
    glMatrixMode(GL_MODELVIEW);                                // モデルビュー変換行列を計算対象に設定

    winW = w;
    winH = h;
}

// マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        // マウス情報格納
        mX = x;
        mY = y;
        mState = state;
        mButton = button;
    }
}

// マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton == GLUT_RIGHT_BUTTON)
    {
        // マウスの移動量を角度変化量に変換
        eDegY = eDegY + (mX - x) * 0.5; // マウス横方向→水平角
        eDegX = eDegX + (y - mY) * 0.5; // マウス縦方向→垂直角
    }

    // マウス座標格納
    mX = x;
    mY = y;
}

// キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'q':
    case 'Q':
    case 27:
        exit(0);
    }
}
