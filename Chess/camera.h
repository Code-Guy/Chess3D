#ifndef _CAMERA_H
#define  _CAMERA_H

#include "ork/math/mat4.h"
#include "light.h"

#define X_AXIS ork::vec3f(1.0, 0, 0)
#define Y_AXIS ork::vec3f(0, 1.0, 0)
#define Z_AXIS ork::vec3f(0, 0, 1.0)

#define PI 3.1416
#define ANGLE_TO_RADIAN PI / 180.0f

const ork::mat4f BiasMatrix(
	0.5, 0.0, 0.0, 0.0, 
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0,
	0.5, 0.5, 0.5, 1.0
	);

class Camera
{
public:
	Camera(ork::vec3f pos = ork::vec3f(0, 1, 1.5), ork::vec3f view = ork::vec3f(0, -1, -1), ork::vec3f up = ork::vec3f(0, 1, 0),
		float fov = 60.0f, float aspectRatio = 9.0/16.0, float nearDist = 0.1f, float farDist = 1000.0f,
		float mouseSensitivity = 0.1f, float moveSpeed = 0.1f);

	~Camera();

	void Yaw(float angle);
	void Pitch(float angle);
	void Roll(float angle);

	void Slide(float du, float dv, float dn);

	void RotateX(float angle);
	void RotateY(float angle);

	//更新相关参数（矩阵）
	void Update(int w, int h);

	float CalcDisToOrigin();

	ork::mat4f GetLightVP(const Light &light);
	
	void Reset();

	ork::vec3f pos;//摄像机的位置
	ork::vec3f view;//摄像机的观看目标点
	ork::vec3f up;//摄像机的向上的方向

	ork::vec3f u, v, n;

	float fov;//视角
	float aspectRatio;//宽高比
	float nearDist;//视域棱台的最近距离
	float farDist;//视域棱台的最远距离

	float mouseSensitivity;//鼠标的灵敏度
	float moveSpeed;//按下按键摄像机的平移速度

	ork::mat4f V;//视图矩阵
	ork::mat4f axesV;//坐标轴专用视图矩阵
	ork::mat4f P;//投影矩阵
	ork::mat4f O;//正交投影矩阵
	ork::mat4f VP;//视图投影矩阵
	ork::mat4f N;//normalmatrix

	static Camera *instance;//单例
	static Camera *GetCamera();

private:
	ork::vec3f originPos;//摄像机的位置
	ork::vec3f originView;//摄像机的观看目标点
	ork::vec3f originUp;//摄像机的向上的方向
};

#endif //_CAMERA_H