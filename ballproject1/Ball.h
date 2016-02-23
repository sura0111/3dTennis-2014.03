#pragma once
#include "DxLib.h"
#include <vector>
#include <stdio.h>
#include <math.h>
#define _USE_MATH_DEFINES
#include <cmath>

using namespace std;
class Ball
{
public:
	double x, y, z;
	double vx, vy, vz;
	double ax, ay, az;
	void accel(double Ax, double Ay, double Az, double dt); //加速度ax,ayが加わった時のdt時間後の状態を決定
	Ball(int X, int Y, int Z);

};
