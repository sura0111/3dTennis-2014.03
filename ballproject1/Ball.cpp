#include "Ball.h"


Ball::Ball(int X, int Y, int Z){
	x = X;
	y = Y;
	z = Z;
	vx = 0;
	vy = 0;
	vz = 0;
	ax = 0.0;
	ay = -1.0;
	az = 0.0;
}
//加速度が加わった際の挙動をシミュレート。
void Ball::accel(double Ax, double Ay, double Az, double dt)
{
	ax = Ax;
	ay = Ay;
	az = Az;
	vx += ax * dt;
	vy += ay * dt;
	vz += az * dt;
	x += vx * dt;
	y += vy * dt;
	z += vz * dt;
}