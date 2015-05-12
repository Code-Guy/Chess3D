#ifndef _RAY_H
#define  _RAY_H

#include "board.h"
#include "piece.h"

#define RAY_COLOR ork::vec3f(1, 0, 0)
#define RENDER_LEN 10

class Ray
{
public:
	Ray(const ork::vec3f &p = ork::vec3f::ZERO, const ork::vec3f &dir = ork::vec3f(0, 0, 1));
	~Ray();

	void Render(float dt);

	ork::vec3f p;//源点
	ork::vec3f dir;//方向（已归一化）

	ork::vec2i RayCastWithBoard(const Board *board);//和棋盘包围盒相交，返回相交到那一个格子
	bool RayCastWithPiece(const Piece *piece, DebugFrameMode mode);//和棋子包围盒相交，返回是否相交

	friend Ray CalcPickRay(int x, int y);//计算射线

private:
	//光线与XY平面 XZ平面 YZ平面
	ork::vec2f XYPlaneJoint(float z) const;
	ork::vec2f XZPlaneJoint(float y) const;
	ork::vec2f YZPlaneJoint(float x) const;

	bool IsJointOnSide(ork::vec2f joint, float min1, float max1, float min2, float max2, 
		float sideOffsetRatio1 = 0, float sideOffsetRatio2 = 0) const;//判断交点是否在面上

	ork::ptr<ork::Program> lineProgram;
	ork::ptr< ork::Mesh<ork::vec3f, unsigned int> > mesh;

	bool IntersectWithAABB(AABB aabb, ork::vec3f translate) const;
	bool IntersectWithOctree(OctreeNode *root, ork::vec3f translate) const;
};

#endif //_RAY_H