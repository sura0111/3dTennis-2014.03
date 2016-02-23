#include "DxLib.h"
#include <vector>
#include <stdio.h>
#include <math.h>
#include "Ball.h"
#include "Serial.h"
using namespace std;

#define SERIAL_PORT "COM3"
CSerial serial;

#define WINDOW_SIZE_X 800
#define WINDOW_SIZE_Y 600
//ボール
#define BALL_SIZE_X 100
#define BALL_SIZE_Y 100
#define BALL_SIZE_Z 100

int ModelBall, ModelBat, ModelFloor, ModelBigWall;
//オーディオ
int theme;
int ballbounce;
int bounceflag[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

const float PI = 4 * atan(1.0);

int serial_init(void){
	if (serial.Open(_T(SERIAL_PORT), 0, 0, false) != 0){
		return -1;
	}
	serial.Setup(CSerial::EBaud9600);
	serial.SetupReadTimeouts(CSerial::EReadTimeoutBlocking);
	return 0;
}
void init_resources()
{
	// モデルの読み込み
	ModelBall = MV1LoadModel("ball1.mqo");
	ModelBat = MV1LoadModel("Bat1.mqo");
	ModelFloor = MV1LoadModel("floor.mqo");
	theme = LoadSoundMem("smallcrowd.mp3");
	ModelBigWall = MV1LoadModel("wallbig.mqo");
	ballbounce = LoadSoundMem("ballbounce.mp3");
	// 音量の設定
	ChangeVolumeSoundMem(255 * 40 / 100, theme);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	//Serial.h iin huvisagchuud
	unsigned short sX, sY, sZ;
	unsigned char rcvBuffer[6];
	int Color;
	int dX = 0, dY = 0, dZ = 0;
	int s1X = 0, s1Y = 0, s1Z = 0, temp = 0;
	int tempi = 0;
	//Mouse Keyboardtai holbootoi huvisagchuud
	int MouseInput;
	int MouseX, MouseY;
	int Key;
	//Camernii Y tenhlegiin daguu hudulguun
	float camY=0;
	//Undsen togloomiin huvisagchuud
	double kf = 0.1;
	int t = 0;
	double ax = 0.0, ay = -1.0, az=0, dt = 1.0;
	double AB_x, AB_y, AB_z, tr;
	double distance;
	vector<Ball> balls;
	Ball bat(0, 0, 0); //地面も物体の一つ。
	Ball ground(0, -5000, 10000);
	int beforeMouseInput = 0;
	int ball_num = 0;
	int size;
	double d;
	double bat_accel = 0.0;
	double ground_accel = 0.0;
	int white;
	int i;
	int wallflag = 1;
	//untsug , bairshil
	float angleA = 0, angleB = 0;
	float batY = 0;
	//normal vector
	float nA, nB, nC, nQ;

	white = GetColor(255, 255, 255);
	ChangeWindowMode(TRUE);
	SetGraphMode(WINDOW_SIZE_X, WINDOW_SIZE_Y, 32);
	if (DxLib_Init() < 0){return -1;}	// ＤＸライブラリの初期化		// エラーが発生したら直ちに終了
	init_resources();
	if (serial_init() != 0){
		/*DrawFormatString(0, 0, GetColor(255, 0, 0), "can't open COM3");
		WaitTimer(1000);
		return -1;*/
	}
	// 描画先を裏画面にする
	SetDrawScreen(DX_SCREEN_BACK);
	SetMouseDispFlag(TRUE);
	//PlaySoundMem(theme, DX_PLAYTYPE_LOOP);
	
	
	// ＥＳＣキーが押されるかウインドウが閉じられるまでループ
	while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
	{
		// 画面をクリア
		ClearDrawScreen();

		//Serial.h
		serial.Read(rcvBuffer, 6);
		sX = (rcvBuffer[0] << 8 | rcvBuffer[1]);
		sY = (rcvBuffer[2] << 8 | rcvBuffer[3]);
		sZ = (rcvBuffer[4] << 8 | rcvBuffer[5]);
		//Keyboard
		Key = GetJoypadInputState(DX_INPUT_KEY_PAD1);
		if (Key&PAD_INPUT_RIGHT){ bat.x += 100;  }
		if (Key&PAD_INPUT_LEFT){ bat.x -= 100; }
		if (bat.x > 9000)bat.x = 9000;
		if (bat.x < -9000)bat.x = -9000;
		if (Key&PAD_INPUT_UP){ camY += 50; }
		if (Key&PAD_INPUT_DOWN){ camY -= 50; }
		if (camY > 15000)camY = 10000;
		if (camY < -5000)camY = -5000;
		//Game
		bat.accel(0, bat_accel, 0, dt);//地面の挙動のシミュレーション
		SetCameraPositionAndTarget_UpVecY(VGet(bat.x, 800, -2000), VGet(bat.x, 600+camY, 0));
		/*MV1SetPosition(ModelBackWall, VGet(0.0f, 0.0f, 500.0f));*/
		bat.y = -dZ;
		MV1SetRotationXYZ(ModelBat, VGet(DX_PI_F*dX / 1440.0f, 0.0f, DX_PI_F*dY / 720.0f));
		MV1SetPosition(ModelBat, VGet((float)bat.x, (float)bat.y, (float)bat.z));
		/*MV1SetPosition(ModelSideWall, VGet(-525.0f, 0.0f, 0.0));
		MV1DrawModel(ModelSideWall);
		MV1SetPosition(ModelSideWall, VGet(525.0f, 0.0f, 0.0));
		MV1DrawModel(ModelSideWall);*/
		MV1SetPosition(ModelBigWall, VGet(0.0f, 0.0f, 10000.0f));
		MV1SetPosition(ModelFloor, VGet(ground.x, ground.y, ground.z));
		MV1DrawModel(ModelBigWall);
		MV1DrawModel(ModelFloor);
		angleA = DX_PI_F*dY / 720.0f;
		angleB = -DX_PI_F*dX / 1440.0f;
		nA = -sin(angleA)*cos(angleB);
		nB = cos(angleA)*cos(angleB);
		nC = -cos(angleA)*sin(angleB);
		nQ = sqrt(nA*nA+nB*nB+nC*nC);
		// モデルの描画
		//MV1DrawModel(ModelBackWall);
		MV1DrawModel(ModelBat);
		if (tempi <= 200)tempi++;
		if (tempi == 200){
			s1X = sX;
			s1Y = sY;
			s1Z = sZ;
		}

		if (tempi == 201){
			if (s1X > 45000){
				dY = -(sY - s1Y) / 50;
				dX = -(sZ - s1Z) / 100;
				dZ = (sX - s1X) / 100;
			}
			else if (s1Y > 45000){
				dY = -(sZ - s1Z) / 50;
				dX = -(sX - s1X) / 100;
				dZ = (sY - s1Y) / 100;
			}
			else if (s1Z > 45000){
				dY = -(sX - s1X) / 50;
				dX = -(sY - s1Y) / 100;
				dZ = (sZ - s1Z) / 100;
			}
			DrawFormatString(0, 250, GetColor(255, 255, 255), "DAMN");

			MouseInput = GetMouseInput();
			
			GetMousePoint(&MouseX, &MouseY);

			if ((beforeMouseInput & MOUSE_INPUT_LEFT) != MOUSE_INPUT_LEFT && (GetMouseInput() & MOUSE_INPUT_LEFT)
				== MOUSE_INPUT_LEFT){
				// マウスの位置を取得
				GetMousePoint(&MouseX, &MouseY);
				balls.push_back(Ball(MouseX, MouseY, -500));
				balls[ball_num].x = bat.x;//MouseX-400;
				balls[ball_num].y = 250;// 1000 - MouseY / 2;
				balls[ball_num].vx = MouseX / 25 - 16;
				balls[ball_num].vy = (600 - MouseY) / 15;
				balls[ball_num].vz = sqrt(324 - (MouseX / 25 - 16)*(MouseX / 25 - 16));
				size = balls.size();
				ball_num++;
				vector<Ball>::iterator theIterator = balls.begin();
				if (size == 21){
					balls.erase(theIterator);
					size = balls.size();
					ball_num = size - 1;
				}
			}
			beforeMouseInput = GetMouseInput();
			// 物理シミュレーション。各ボールが自由落下運動する。衝突も考えて判定。位置、速度、加速度に関する構造体
			bat_accel = (double)(0 - bat.y) - bat.vy*1.0; //地面に、元の位置に戻る側の力が加わる。粘性項も。
			ground_accel = (double)(-5000 - ground.y) - ground.vy*1.0; //地面に、元の位置に戻る側の力が加わる。粘性項も。
			for (i = 0; i < ball_num; i++){
				t++;
				balls[i].ax = 0.0;
				balls[i].ay = -1.0;
				balls[i].az = 0.0;
				if (balls[i].y < -5000){
					ay += (-5000 - balls[i].y)*kf - balls[i].vy*0.1;
					ground_accel -= (-5000 - balls[i].y + 1 * sin(t))*0.08 - balls[i].vy*0.1; //反作用として地面に逆向きの力が加わる
				}
				if ((nA*(balls[i].x-bat.x) + nB*(balls[i].y-batY) + nC*(balls[i].z-bat.z)) / nQ<sqrt(5000) && balls[i].x<bat.x + 500 && balls[i].x>bat.x - 500 && balls[i].z<bat.z + 1000 && balls[i].z>bat.z-500){
					balls[i].ax += ((sqrt(5000) - (nA*(balls[i].x - bat.x) + nB*(balls[i].y - batY) + nC*(balls[i].z - bat.z)) / nQ)*kf - balls[i].vx*0.1)*nA / nQ;
					balls[i].ay += ((sqrt(5000) - (nA*(balls[i].x - bat.x) + nB*(balls[i].y - batY) + nC*(balls[i].z - bat.z)) / nQ)*kf - balls[i].vy*0.1)*nB / nQ;
					balls[i].az += ((sqrt(5000) - (nA*(balls[i].x - bat.x) + nB*(balls[i].y - batY) + nC*(balls[i].z - bat.z)) / nQ)*kf - balls[i].vz*0.1)*nC / nQ;
					bat_accel -= ((sqrt(5000) - (nA*(balls[i].x - bat.x) + nB*(balls[i].y - batY) + nC*(balls[i].z - bat.z)) / nQ)*kf + 1 * sin(t))*0.08 + balls[i].vy*0.1; //反作用として地面に逆向きの力が加わる
				}
				if (bounceflag[i] == 1){ PlaySoundMem(ballbounce, DX_PLAYTYPE_BACK); }
				bounceflag[i] = 0;

				for (int j = 0; j < ball_num; j++){
					if (j <= i)continue;
					AB_x = balls[j].x - balls[i].x;
					AB_y = balls[j].y - balls[i].y;
					AB_z = balls[j].z - balls[i].z;
					tr = 100;
					if (AB_x*AB_x + AB_y*AB_y + AB_z*AB_z < tr*tr){
						bounceflag[i] = 1;
						bounceflag[j] = 1;
						d = sqrt(AB_x*AB_x + AB_y*AB_y + AB_z*AB_z);
						distance = tr - d;
						if (d>0)d = 1 / d;
						AB_x *= d;
						AB_y *= d;
						AB_z *= d;
						balls[i].ax -= AB_x*0.1*distance + balls[i].vx*0.1;
						balls[i].ay -= AB_y*0.1*distance + balls[i].vy*0.1;
						balls[i].az -= AB_z*0.1*distance + balls[i].vz*0.1;
						balls[j].vx = -balls[i].vx;
						balls[j].ay = -balls[i].ay;
						balls[j].vz = -balls[i].vz;
					}
				}
				balls[i].accel(balls[i].ax, balls[i].ay, balls[i].az, dt);
				if (balls[i].x > 10000 || balls[i].x < -10000){
					if (wallflag == 1 && balls[i].x > 10000){ balls[i].x = 10000; wallflag = 0; }
					if (wallflag == 1 && balls[i].x <-10000){ balls[i].x = -10000; wallflag = 0; }
					balls[i].vx = -balls[i].vx*0.8;
					balls[i].vz = balls[i].vz*0.8;
				}
				if (bounceflag[i] == 1){ PlaySoundMem(ballbounce, DX_PLAYTYPE_BACK); }
				bounceflag[i] = 0;
				if (balls[i].z > 10000 || balls[i].z < -10000){
					if (wallflag == 1 && balls[i].z > 10000){ balls[i].z = 10000; wallflag = 0; }
					if (wallflag == 1 && balls[i].z <-10000){ balls[i].z = -10000; wallflag = 0; }
					balls[i].vz = -balls[i].vz*0.8;
					balls[i].vx = balls[i].vx*0.8;
					wallflag = 0;
				}
				if (balls[i].z < 10000 && balls[i].z > -10000 && balls[i].x < 10000 && balls[i].x > -10000)wallflag = 1;
				// モデルの座標を設定
				MV1SetPosition(ModelBall, VGet((float)balls[i].x - BALL_SIZE_X / 2, (float)balls[i].y + BALL_SIZE_Y / 2, (float)balls[i].z - BALL_SIZE_Z / 2));
				// モデルの描画
				MV1DrawModel(ModelBall);
			}
			//MV1SetPosition(ModelBall, VGet(0.0f, 180.0f, 100.0f));
			//MV1DrawModel(ModelBall);
		}
		// 画面左上に Near の値と Far の値を描画
		Color = GetColor(255, 255, 255);

		DrawFormatString(0, 0, Color, "dX:%d", sX);
		DrawFormatString(0, 50, Color, "dY:%d", sY);
		DrawFormatString(0, 100, Color, "dZ:%d", sZ);
		DrawFormatString(100, 0, Color, "dX:%d", dX);
		DrawFormatString(100, 50, Color, "dY:%d", dY);
		DrawFormatString(100, 100, Color, "dZ:%d", dZ);
		DrawFormatString(0, 150, Color, "firstX:%d, firstY:%d, firstZ:%d", s1X, s1Y, s1Z);
		DrawLine3D(VGet(bat.x, bat.y - dZ, bat.z), VGet(bat.x + 100 * nA, bat.y - dZ + 100 * nB, bat.z + 100*nC), GetColor(255, 255, 255));
		// 裏画面の内容を表画面に反映
		ScreenFlip();
	}

	// ＤＸライブラリの後始末
	DxLib_End();

	// ソフトの終了
	return 0;
}
/*#include "DxLib.h"

#define BLOCK_SIZE		1000.0f		// ブロックのサイズ

#define BLOCK_NUM_X		16		// Ｘ方向のブロック数
#define BLOCK_NUM_Z		16		// Ｚ方向のブロック数

#define CAMERA_Y		500.0f		// カメラの高さ

// マップ( 1:道  0:壁 )
char Map[BLOCK_NUM_Z][BLOCK_NUM_X] =
{
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
};

// WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
int KabeModel;		// 壁モデル
int x, z;		// 位置
int movx, movz;	// 移動先の座標
int Muki;		// 向き( 0:x軸プラス方向  1:z軸マイナス方向  2:x軸マイナス方向  3:z軸プラス方向 )
int NowInput;		// 現在のフレームの入力
int BackInput;		// 一つ前のフレームの入力
int EdgeInput;		// 入力のエッジ
int FrameNo;		// フレーム番号
int i, j;		// カウンタ
VECTOR CamPos;		// カメラの座標
VECTOR CamTarg;	// カメラの注視点

// ウインドウモードで起動
ChangeWindowMode(TRUE);

// ＤＸライブラリの初期化
if (DxLib_Init() < 0) return -1;

// 壁モデルの読みこみ
KabeModel = MV1LoadModel("Kabe.mqo");

// 位置と向きと入力状態の初期化
x = 1;
z = 1;
Muki = 0;
NowInput = 0;

// 描画先を裏画面にする
SetDrawScreen(DX_SCREEN_BACK);

// メインループ
// エスケープキーが押されるまでループ
while (ProcessMessage() == 0 && CheckHitKey(KEY_INPUT_ESCAPE) == 0)
{
// 画面をクリアする
ClearDrawScreen();

// 入力情報を一つ前のフレームの入力に代入する
BackInput = NowInput;

// 現在の入力を取得する
NowInput = GetJoypadInputState(DX_INPUT_KEY_PAD1);

// 現在のフレームで初めて押されたボタンを算出する
EdgeInput = NowInput & ~BackInput;

// 上が押されたら向いている方向に移動する
if (CheckHitKey(KEY_INPUT_UP) != 0)
{
// 向きによって移動方向が変わる
switch (Muki)
{
case 0: movx = 1; movz = 0; break;		// Ｘ軸プラス方向
case 1: movx = 0; movz = -1; break;		// Ｚ軸マイナス方向
case 2: movx = -1; movz = 0; break;		// Ｘ軸マイナス方向
case 3: movx = 0; movz = 1; break;		// Ｚ軸プラス方向
}

// 移動先のマスが道だったら移動する
if (Map[z + movz][x + movx] == 1)
{
x += movx;
z += movz;
}
}

// 下が押されたら向いている方向と逆方向に移動する
if ((EdgeInput & PAD_INPUT_DOWN) != 0)
{
// 向きによって移動方向が変わる
switch (Muki)
{
case 0: movx = -1; movz = 0; break;		// Ｘ軸プラス方向
case 1: movx = 0; movz = 1; break;		// Ｚ軸マイナス方向
case 2: movx = 1; movz = 0; break;		// Ｘ軸マイナス方向
case 3: movx = 0; movz = -1; break;		// Ｚ軸プラス方向
}

// 移動先のマスが道だったら移動する
if (Map[z + movz][x + movx] == 1)
{
x += movx;
z += movz;
}
}

// 左が押されていたら向いている方向を左に９０度変更する
if ((EdgeInput & PAD_INPUT_LEFT) != 0)
{
if (Muki == 0)
{
Muki = 3;
}
else
{
Muki--;
}
}

// 右が押されていたら向いている方向を右に９０度変更する
if ((EdgeInput & PAD_INPUT_RIGHT) != 0)
{
if (Muki == 3)
{
Muki = 0;
}
else
{
Muki++;
}
}

// カメラの座標をセット
CamPos = VGet(x * BLOCK_SIZE, CAMERA_Y, z * BLOCK_SIZE);

// カメラの注視点をセット
switch (Muki)
{
case 0: CamTarg = VGet(1.0f, 0.0f, 0.0f); break;	// Ｘ軸プラス方向
case 1: CamTarg = VGet(0.0f, 0.0f, -1.0f); break;	// Ｚ軸マイナス方向
case 2: CamTarg = VGet(-1.0f, 0.0f, 0.0f); break;	// Ｘ軸マイナス方向
case 3: CamTarg = VGet(0.0f, 0.0f, 1.0f); break;	// Ｚ軸プラス方向
}
CamTarg = VAdd(CamPos, CamTarg);

// カメラの位置と向きをセットする
SetCameraPositionAndTarget_UpVecY(CamPos, CamTarg);

// マップを描画する
for (i = 0; i < BLOCK_NUM_Z; i++)
{
for (j = 0; j < BLOCK_NUM_X; j++)
{
// 道ではないところは描画しない
if (Map[i][j] == 0) continue;

// 壁モデルの座標を変更する
MV1SetPosition(KabeModel, VGet(j * BLOCK_SIZE, 0.0f, i * BLOCK_SIZE));

// ４方の壁の状態で描画するフレーム番号を変更する
FrameNo = 0;
if (Map[i][j + 1] == 0) FrameNo += 1;
if (Map[i][j - 1] == 0) FrameNo += 2;
if (Map[i + 1][j] == 0) FrameNo += 4;
if (Map[i - 1][j] == 0) FrameNo += 8;

// 割り出した番号のフレームを描画する
MV1DrawFrame(KabeModel, FrameNo);
}
}

// 裏画面の内容を表画面に反映する
ScreenFlip();
}

// ＤＸライブラリの後始末
DxLib_End();

// ソフトの終了
return 0;
}*/
/*
#include "DxLib.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
// ＤＸライブラリの初期化
if (DxLib_Init() < 0)
{
// エラーが発生したら直ちに終了
return -1;
}
ChangeWindowMode(TRUE);
SetGraphMode(800, 600, 32);
SetDrawScreen(DX_SCREEN_BACK);
SetMouseDispFlag(TRUE);
// Ｚバッファを有効にする
SetUseZBuffer3D(TRUE);

// Ｚバッファへの書き込みを有効にする
SetWriteZBuffer3D(TRUE);

// ３Ｄ空間上に球を描画する
DrawSphere3D(VGet(320.0f, 200.0f, 0.0f), 80.0f, 32, GetColor(255, 0, 0), GetColor(255, 255, 255), TRUE);

// キー入力待ちをする
WaitKey();

// ＤＸライブラリの後始末
DxLib_End();

// ソフトの終了
return 0;
}
*/