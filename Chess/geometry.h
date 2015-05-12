#ifndef _GEOMETRY_H
#define  _GEOMETRY_H

#include <vector>
#include "global.h"
#include "ork\math\vec2.h"

#define  MAXNUM 1e8
#define INFINITY_AABB AABB(-MAXNUM, MAXNUM, -MAXNUM, MAXNUM, -MAXNUM, MAXNUM)

struct V2_UV
{
	float vx, vy, u, v;

	V2_UV(float vx = 0, float vy = 0,
		float u = 0, float v = 0) :
	vx(vx), vy(vy), u(u), v(v)
	{
	}
};

struct V_UV_N {

	float vx, vy, vz, u, v, nx, ny, nz;

	V_UV_N()
	{

	}

	V_UV_N(float vx, float vy, float vz,
		float u, float v,
		float nx, float ny, float nz) :
	vx(vx), vy(vy), vz(vz), u(u), v(v), nx(nx), ny(ny), nz(nz)
	{
	}

};

struct V_UV_N_T_B {

	float vx, vy, vz, u, v, nx, ny, nz, tx, ty, tz, bx, by, bz;

	V_UV_N_T_B()
	{

	}

	V_UV_N_T_B(float vx, float vy, float vz,
		float u, float v,
		float nx, float ny, float nz,
		float tx, float ty, float tz, 
		float bx, float by, float bz) :
	vx(vx), vy(vy), vz(vz), u(u), v(v), nx(nx), ny(ny), nz(nz),
		tx(tx), ty(ty), tz(tz), bx(bx), by(by), bz(bz)
	{
	}

};

struct VBO
{
	static std::vector<ork::vec3f> vertices;
	static std::vector<ork::vec2f> uvs;
	static std::vector<ork::vec3f> normals;

	static void Clear()
	{
		vertices.clear();
		uvs.clear();
		normals.clear();
	}
};

struct Refer//顶点、纹理坐标、法向三个索引
{
	Refer(int v, int uv, int n) : v(v), uv(uv), n(n)
	{

	}

	int v;//点的索引
	int uv;//纹理坐标的索引
	int n;//法向的索引
};

struct Face
{
	std::vector<Refer> refers;//一个索引的链表
};

class AABB
{
public:
	AABB(float xMin = -MAXNUM, float xMax = MAXNUM,
		float yMin = -MAXNUM, float yMax = MAXNUM,
		float zMin = -MAXNUM, float zMax = MAXNUM);
	~AABB();

	void Inverse();
	void TranslateSelf(ork::vec3f offset);
	AABB Translate(ork::vec3f offset) const;

	bool IsVertexIn(const ork::vec3f &v) const;

	float xMin, xMax;
	float yMin, yMax;
	float zMin, zMax;
};

bool IsFloatEqual(float a, float b);

template<typename T>
T GetVectorMax(std::vector<T> n)
{
	T max = -MAXNUM;
	for (int i = 0; i < n.size(); i++)
	{
		if (n[i] > max)
		{
			max = n[i];
		}
	}
	return max;
}

#endif //_GEOMETRY_H