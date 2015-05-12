#include "camera.h"

using namespace ork;

Camera * Camera::instance = NULL;//单例

Camera::Camera(vec3f pos, vec3f view, vec3f up,
	float fov, float aspectRatio, float nearDist, float farDist,
	float mouseSensitivity, float moveSpeed)
	: pos(pos), view(view.normalize()), up(up.normalize()),
	fov(fov), aspectRatio(aspectRatio), nearDist(nearDist), farDist(farDist),
	mouseSensitivity(mouseSensitivity), moveSpeed(moveSpeed)
{
	originPos = pos;
	originUp = up;
	originView = view;

	vec3f upVector = up - view;

	n = pos - view;
	u = upVector.crossProduct(n);
	v = n.crossProduct(u);

	n = n.normalize();
	u = u.normalize();
	v = v.normalize();
}

Camera::~Camera()
{
}

void Camera::Yaw(float angle)
{
	float radian = angle * ANGLE_TO_RADIAN;
	float cosVal = cos(radian);
	float sinVal = sin(radian);

	vec3f t = n;
	vec3f s = u;

	n = t * cosVal - s * sinVal;
	u = t * sinVal + s * cosVal;
}

void Camera::Pitch(float angle)
{
	float radian = angle * ANGLE_TO_RADIAN;
	float cosVal = cos(radian);
	float sinVal = sin(radian);

	vec3f t = v;
	vec3f s = n;

	v = t * cosVal - s * sinVal;
	n = t * sinVal + s * cosVal;
}

void Camera::Roll(float angle)
{
	float radian = angle * ANGLE_TO_RADIAN;
	float cosVal = cos(radian);
	float sinVal = sin(radian);

	vec3f t = u;
	vec3f s = v;

	u = t * cosVal - s * sinVal;
	v = t * sinVal + s * cosVal;
}

void Camera::Slide(float du, float dv, float dn)
{
	pos.x += du * u.x + dv * v.x + dn * n.x;
	pos.y += du * u.y + dv * v.y + dn * n.y;
	pos.z += du * u.z + dv * v.z + dn * n.z;

	view.x += du * u.x + dv * v.x + dn * n.x;
	view.y += du * u.y + dv * v.y + dn * n.y;
	view.z += du * u.z + dv * v.z + dn * n.z;
}

void Camera::RotateX(float angle)
{
	float d = CalcDisToOrigin();
	int cnt = 100;
	float theta = angle / cnt;
	float slide_d = (float)(-2 * d * sin(theta * PI / 360.0));
	Yaw(theta / 2);
	for (; cnt != 0; --cnt)
	{
		Slide(slide_d, 0, 0);
		Yaw(theta);
	}
	Yaw(-theta / 2);
}

void Camera::RotateY(float angle)
{
	float d = CalcDisToOrigin();
	int cnt = 100;
	float theta = angle / cnt;
	float slide_d = (float)(2 * d * sin(theta * PI / 360.0));
	Pitch(theta / 2);
	for (; cnt != 0; --cnt)
	{
		Slide(0, slide_d, 0);
		Pitch(theta);
	}
	Pitch(-theta / 2);
}

//更新相关参数（矩阵）
void Camera::Update(int w, int h)
{
	float m[4][4];

	m[0][0] = u.x; m[0][1] = u.y; m[0][2] = u.z; m[0][3] = -pos.dotproduct(u);
	m[1][0] = v.x; m[1][1] = v.y; m[1][2] = v.z; m[1][3] = -pos.dotproduct(v);
	m[2][0] = n.x; m[2][1] = n.y; m[2][2] = n.z; m[2][3] = -pos.dotproduct(n);
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1.0f;

	V = mat4f(m);

	m[0][0] = V[0][0]; m[0][1] = V[0][1]; m[0][2] = V[0][2]; m[0][3] = -0.9f;
	m[1][0] = V[1][0]; m[1][1] = V[1][1]; m[1][2] = V[1][2]; m[1][3] = 0.85f;
	m[2][0] = V[2][0]; m[2][1] = V[2][1]; m[2][2] = V[2][2]; m[2][3] = -5;
	m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1.0f;

	axesV = mat4f(m);

	aspectRatio = (float)w / (float)h;
	P = mat4f::perspectiveProjection(fov, aspectRatio, nearDist, farDist);
	O = mat4f::orthoProjection(1, -1, 1, -1, nearDist, farDist);

	VP = P * V;

	N = V.inverse().transpose();
}

Camera * Camera::GetCamera()
{
	if (instance == NULL)
	{
		instance = new Camera();
	}
	return instance;
}

float Camera::CalcDisToOrigin()
{
	return sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
}

mat4f Camera::GetLightVP(const Light &light)
{
	mat4f V, P;
	V = mat4f::lookAt(light.pos, vec3f::ZERO, vec3f(0, 1, 0));
	if (light.lightType == SpotLight)
	{
		P = mat4f::perspectiveProjection(fov, aspectRatio, LIGHT_ZNEAR, LIGHT_ZFAR);
	}
	else if(light.lightType == DirectionLight)
	{
		P = mat4f::orthoProjection(-10, 10, -10, 10, -10, 20);
	}

	return P * V;
}

void Camera::Reset()
{
	pos = originPos;
	up = originUp;
	view = originView;

	vec3f upVector = up - view;

	n = pos - view;
	u = upVector.crossProduct(n);
	v = n.crossProduct(u);

	n = n.normalize();
	u = u.normalize();
	v = v.normalize();
}