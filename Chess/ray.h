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

	ork::vec3f p;//Դ��
	ork::vec3f dir;//�����ѹ�һ����

	ork::vec2i RayCastWithBoard(const Board *board);//�����̰�Χ���ཻ�������ཻ����һ������
	bool RayCastWithPiece(const Piece *piece, DebugFrameMode mode);//�����Ӱ�Χ���ཻ�������Ƿ��ཻ

	friend Ray CalcPickRay(int x, int y);//��������

private:
	//������XYƽ�� XZƽ�� YZƽ��
	ork::vec2f XYPlaneJoint(float z) const;
	ork::vec2f XZPlaneJoint(float y) const;
	ork::vec2f YZPlaneJoint(float x) const;

	bool IsJointOnSide(ork::vec2f joint, float min1, float max1, float min2, float max2, 
		float sideOffsetRatio1 = 0, float sideOffsetRatio2 = 0) const;//�жϽ����Ƿ�������

	ork::ptr<ork::Program> lineProgram;
	ork::ptr< ork::Mesh<ork::vec3f, unsigned int> > mesh;

	bool IntersectWithAABB(AABB aabb, ork::vec3f translate) const;
	bool IntersectWithOctree(OctreeNode *root, ork::vec3f translate) const;
};

#endif //_RAY_H